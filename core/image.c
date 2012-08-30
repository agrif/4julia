#include "4julia.h"

#include "png.h"

#include <pthread.h>
#include <sys/ioctl.h>

unsigned int _j_image_size;
unsigned char* _j_image_data;
FILE* _j_image_file;
png_structp _j_image_png;
png_infop _j_image_info;

#define _J_NUM_RENDER_THREADS 2
#define _J_RENDER_BUFFER_SIZE _J_NUM_RENDER_THREADS
pthread_t _j_render_threads[_J_NUM_RENDER_THREADS];
pthread_mutex_t _j_render_lock = PTHREAD_MUTEX_INITIALIZER;
/* signal this when there is a row in the buffer to write to disk */
pthread_cond_t _j_write_rows = PTHREAD_COND_INITIALIZER;
/* signal this when there is room in the buffer for more data */
pthread_cond_t _j_buffer_rows = PTHREAD_COND_INITIALIZER;
j_vector* _j_render_buffer[_J_RENDER_BUFFER_SIZE];
unsigned int _j_render_next_generate, _j_render_next_buffer, _j_render_size;

int j_image_open(const char* name)
{
	_j_image_size = j_raytracer_get_image_size();
	_j_image_file = fopen(name, "wb");
	
	if (!_j_image_file)
	{
		printf("error: could not open file: %s\n", name);
		return 1;
	}
	
	_j_image_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!_j_image_png)
	{
		printf("error: could not create png writer\n");
		return 2;
	}
	
	_j_image_info = png_create_info_struct(_j_image_png);
	
	if (!_j_image_info)
	{
		printf("error: could not create png info container\n");
		return 3;
	}
	
	if (setjmp(png_jmpbuf(_j_image_png)))
	{
		printf("error: could not initialize png writer\n");
		return 4;
	}
	
	png_init_io(_j_image_png, _j_image_file);
	
	png_set_IHDR(_j_image_png, _j_image_info, _j_image_size, _j_image_size, 8,
		PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	
	png_write_info(_j_image_png, _j_image_info);
	
	_j_image_data = malloc(_j_image_size * 4);
	
	return 0;
}

int j_image_write_row(j_vector* row)
{
	if (setjmp(png_jmpbuf(_j_image_png)))
	{
		printf("error: could not write png data\n");
		return 1;
	}
	
	unsigned int n;
	for (n = 0; n < _j_image_size; ++n)
	{
		_j_image_data[(n * 4)] = J_CLAMP(row[n].x, 0, 1) * 255;
		_j_image_data[(n * 4) + 1] = J_CLAMP(row[n].y, 0, 1) * 255;
		_j_image_data[(n * 4) + 2] = J_CLAMP(row[n].z, 0, 1) * 255;
		_j_image_data[(n * 4) + 3] = J_CLAMP(row[n].w, 0, 1) * 255;
	}
	
	png_write_row(_j_image_png, _j_image_data);
	return 0;
}

int j_image_close()
{
	if (setjmp(png_jmpbuf(_j_image_png)))
	{
		printf("error: could not deinitialize png writer\n");
		return 1;
	}
	
	free(_j_image_data);
	
	png_write_end(_j_image_png, _j_image_info);
	fclose(_j_image_file);
	png_destroy_write_struct(&_j_image_png, &_j_image_info);
	return 0;
}

void _j_render_intro(const char* name, unsigned int size)
{
	printf("%s %s", J_NAME, J_VERSION);
	printf(" - %s (%i x %i)\n", name, size, size);
}

void _j_render_bar(unsigned int row, unsigned int size)
{
	unsigned int i;
	
	struct winsize w;
	char* widget = "-\\|/";
	
	ioctl(fileno(stdout), TIOCGWINSZ, &w);
	
	printf("\x1b[s  %c [", widget[row % 4]);
	for (i = 0; i < w.ws_col - 14; ++i)
		printf("%c", i * size < row * (w.ws_col - 14) ? '=' : ' ');
	printf("] %5.1f%%\x1b[u", (double)(row * 100) / size);
	fflush(stdout);
}

void* _j_render_thread(void* data)
{
	unsigned int y, x;
	j_vector* row;
	while (_j_render_next_generate < _j_render_size)
	{
		pthread_mutex_lock(&_j_render_lock);
		y = _j_render_next_generate;
		++_j_render_next_generate;
		pthread_mutex_unlock(&_j_render_lock);
		
		/*printf("work thread: starting %i\n", y);*/
		
		row = malloc(sizeof(j_vector) * _j_render_size);
		
		for (x = 0; x < _j_render_size; ++x)
		{
			row[x] = j_raytracer_render(x, y);
		}
		
		/*printf("work thread: buffering %i\n", y);*/
		
		pthread_mutex_lock(&_j_render_lock);
		while ((y >= _j_render_next_buffer + _J_RENDER_BUFFER_SIZE) ||
			(_j_render_buffer[y % _J_RENDER_BUFFER_SIZE] != NULL))
			pthread_cond_wait(&_j_buffer_rows, &_j_render_lock);
		_j_render_buffer[y % _J_RENDER_BUFFER_SIZE] = row;
		/*printf("work thread: buffered %i\n", y);*/
		pthread_mutex_unlock(&_j_render_lock);
		
		if (y == _j_render_next_buffer)
			pthread_cond_broadcast(&_j_write_rows);
	}
	
	return NULL;
}

void _j_image_render_threaded(const char* name)
{
	if (j_image_open(name))
		return;
	
	unsigned int i;
	for (i = 0; i < _J_RENDER_BUFFER_SIZE; ++i)
		_j_render_buffer[i] = NULL;
	
	_j_render_next_generate = 0;
	_j_render_next_buffer = 0;
	_j_render_size = j_raytracer_get_image_size();
	
	for (i = 0; i < _J_NUM_RENDER_THREADS; ++i)
		pthread_create(&_j_render_threads[i], NULL, _j_render_thread, NULL);
	
	_j_render_intro(name, _j_render_size);
	
	unsigned int y = 0;
	while (y < _j_render_size)
	{
		pthread_mutex_lock(&_j_render_lock);
		while (_j_render_buffer[y % _J_RENDER_BUFFER_SIZE] == NULL)
			pthread_cond_wait(&_j_write_rows, &_j_render_lock);
		while (_j_render_buffer[y % _J_RENDER_BUFFER_SIZE] != NULL)
		{
			/*printf("main thread: writing %i\n", y);*/
			j_image_write_row(_j_render_buffer[y % _J_RENDER_BUFFER_SIZE]);
			free(_j_render_buffer[y % _J_RENDER_BUFFER_SIZE]);
			_j_render_buffer[y % _J_RENDER_BUFFER_SIZE] = NULL;
			++y;
		}
		_j_render_next_buffer = y;
		pthread_mutex_unlock(&_j_render_lock);
		pthread_cond_broadcast(&_j_buffer_rows);
		_j_render_bar(y, _j_render_size);
	}
	
	_j_render_bar(y, _j_render_size);
	printf("\n");
	j_image_close();
}

void _j_image_render_nonthreaded(const char* name)
{
	unsigned int x, y, i;
	if (j_image_open(name))
		return;
	j_vector* row = malloc(sizeof(j_vector) * j_raytracer_get_image_size());
	if (!row)
	{
		printf("error: Could not allocate enough memory for one row.\n");
		return;
	}
	
	unsigned int size = j_raytracer_get_image_size();
	_j_render_intro(name, size);
	for (y = 0; y < size; ++y)
	{
		for (x = 0; x < size; ++x)
		{
			row[x] = j_raytracer_render(x, y);
			//j_vector_print(row[x]); printf("\n");
		}
		if (j_image_write_row(row))
			break;
	        
		_j_render_bar(y, size);
	}
	_j_render_bar(y, size);
	printf("\n");
	free(row);
	j_image_close();
}

void j_image_render(const char* name)
{
	_j_image_render_threaded(name);
}


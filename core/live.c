#include "4julia.h"

#include "SDL/SDL.h"

#include <pthread.h>
#include <time.h>

#define _J_STARTSIZE 64
#define _J_BPP 32
#define _J_FRAMERATE 60
#define _J_SURFACETYPE SDL_SWSURFACE

volatile unsigned int _j_live_running;
volatile unsigned int _j_live_rendering;
volatile unsigned int _j_live_reset;

volatile unsigned int _j_live_size = 0;
unsigned int _j_live_current_size;
unsigned int _j_live_x;
unsigned int _j_live_y;

pthread_t _j_live_thread;

SDL_Surface* _j_live_screen;

void _j_live_thread_work()
{	
	if (_j_live_reset)
	{
		if (_j_live_size != j_raytracer_get_image_size())
			_j_live_screen = SDL_SetVideoMode(j_raytracer_get_image_size(), j_raytracer_get_image_size(), _J_BPP, _J_SURFACETYPE);
		_j_live_size = j_raytracer_get_image_size();
		_j_live_current_size = _J_STARTSIZE;
		_j_live_x = 0;
		_j_live_y = 0;
		_j_live_reset = 0;
		j_raytracer_set_image_size(_j_live_current_size);
		return;
	}
	
	if (_j_live_rendering)
	{
		j_vector color = j_raytracer_render(_j_live_x, _j_live_y);
		color = j_vector_multiply(color, color.w); /* account for alpha */
		
		unsigned int pixelsize = _j_live_size / _j_live_current_size;
		SDL_Rect dest = {_j_live_x * pixelsize, _j_live_y * pixelsize, pixelsize, pixelsize};
		
		SDL_FillRect(_j_live_screen, &dest, SDL_MapRGB(_j_live_screen->format, J_CLAMP(color.x, 0, 1) * 255, J_CLAMP(color.y, 0, 1) * 255, J_CLAMP(color.z, 0, 1) * 255));
		
		++_j_live_x;
		if (_j_live_x >= _j_live_current_size)
		{
			_j_live_x = 0;
			++_j_live_y;
		}
		if (_j_live_y >= _j_live_current_size)
		{
			_j_live_x = 0;
			_j_live_y = 0;
			_j_live_current_size *= 2;
			if (_j_live_current_size <= _j_live_size)
			{
				j_raytracer_set_image_size(_j_live_current_size);
			}
		}
		if (_j_live_current_size > _j_live_size)
		{
			_j_live_rendering = 0;
		}
		
		return;
	}
}

void* _j_live_thread_entry(void* data)
{
	SDL_Event event;
	SDL_Init(SDL_INIT_VIDEO);
	_j_live_screen = SDL_SetVideoMode(j_raytracer_get_image_size(), j_raytracer_get_image_size(), _J_BPP, _J_SURFACETYPE);
	_j_live_size = j_raytracer_get_image_size();
	SDL_WM_SetCaption(J_NAME " " J_VERSION, NULL);
	unsigned long lasttime = clock();
	while (_j_live_running && _j_live_screen)
	{
		_j_live_thread_work();
		if (clock() > lasttime + ((1.0 / _J_FRAMERATE) * CLOCKS_PER_SEC))
		{
			lasttime = clock();
			SDL_Flip(_j_live_screen);
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					_j_live_running = 0;
				}
			}
		}
	}
	SDL_Quit();
	j_raytracer_set_image_size(_j_live_size);
	_j_live_running = 0;
	_j_live_rendering = 0;
	_j_live_reset = 0;
	_j_live_screen = NULL;
}

void j_live_open()
{
	if (_j_live_running == 0)
	{
		_j_live_running = 1;
		pthread_create(&_j_live_thread, NULL, _j_live_thread_entry, NULL);
	}
}

void j_live_start()
{
	j_live_open();
	if (_j_live_running)
	{
		_j_live_rendering = 1;
		_j_live_reset = 1;
	}
}

void j_live_stop()
{
	if (_j_live_running)
	{
		if (_j_live_rendering)
		{
			if (_j_live_size)
				j_raytracer_set_image_size(_j_live_size);
			_j_live_rendering = 0;
		}
	}
}

void j_live_close()
{
	j_live_stop();
	if (_j_live_running)
	{
		_j_live_running = 0;
		_j_live_rendering = 0;
		_j_live_reset = 0;
		pthread_join(_j_live_thread, NULL);
	}
}

void j_live_wait()
{
	if (_j_live_running)
	{
		pthread_join(_j_live_thread, NULL);
	}
}

void j_live_initialize()
{
	_j_live_running = 0;
	_j_live_rendering = 0;
	_j_live_reset = 0;
	_j_live_screen = NULL;
}

void j_live_exit()
{
	j_live_close();
}


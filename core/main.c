#include "4julia.h"

#include <readline/readline.h>
#include <readline/history.h>

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	j_initialize();
	atexit(j_exit);
	
	if (argc == 1)
	{
		/* interactive mode */
		unsigned int running = 1;
		char* line = NULL;
		unsigned int error;
		
		printf(J_NAME " " J_VERSION "\nType EOF to exit. (CTRL-D on most systems)\n\n");
		
		while (running)
		{
			line = readline("4julia > ");
			if (line == NULL)
			{
				printf("\n");
				return 0;
			}
			
			if (strlen(line) != 0)
			{
				error = j_execute(line);
			
				if (error)
				{
					printf("error: (0x%04x) ", error);
					printf(j_get_error(error), j_get_error_argument(error));
					printf("\n");
				}
				add_history(line);
			}
			free(line);
		}
		return 0;
	}
	
	/*unsigned int n, x, y;
	
	unsigned short seed[3];
	seed[0] = rand() & 0xFF;
	seed[2] = rand() & 0xFF;
	seed[3] = rand() & 0xFF;
	seed48(seed);
	
	j_light a = j_raytracer_add_light(j_vector_create(5, -5, 5, 0), j_vector_create(.7, .5, .2, 0));
	j_light b = j_raytracer_add_light(j_vector_create(-5, -5, 5, 0), j_vector_create(.4, .4, .2, 0));
	j_light c = j_raytracer_add_light(j_vector_create(0, -5, -5, 0), j_vector_create(.3, .2, .2, 0));
	
	j_julia_set_slice(j_vector_create(0, 0, 0, 0), j_vector_create(0, 0, 0, 1));
	j_julia_set_constant(j_vector_create(-0.125,-0.256,0.847,0.0895));
	j_julia_set_iterations(100);
	j_raytracer_set_epsilon(.01, 0.00000001);
	j_raytracer_set_supersampling(1);
	
	j_raytracer_set_camera(3.3, 0 * (M_PI / 180), 45 * (M_PI / 180), 50 * (M_PI / 180));
	j_raytracer_set_image_size(4096);
	
	j_image_render("out.png");
	
	j_live_open();
	j_live_start();
	
	j_live_wait();*/
	
	return 0;
}


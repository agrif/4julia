#include "4julia.h"

#include <string.h>
#include <time.h>
#include <stdlib.h>

#define _J_BUFSIZE 128

void j_initialize()
{
	j_raytracer_initialize();
	j_julia_initialize();
	j_live_initialize();
}

void j_exit()
{
	j_live_exit();
	j_raytracer_exit();
}

/* commands */

struct _j_command_s
{
	const char* command;
	const char* argument_types;
	const char* description;
};

union _j_value_u
{
	char* string;
	long int integer;
	double floating;
};

void _j_value_free(union _j_value_u* values, unsigned int i, unsigned int c)
{
	unsigned int j;
	for (j = 0; j < i; ++j)
	{
		switch (j_get_command_argument_type(c, j))
		{
			case J_STRING:
				free(values[j].string);
				break;
			default:
				break;
		};
	}
	
	free(values);
}

struct _j_command_s _j_commands[] = {
{"help", "", "display a list of commands"},

{"sleep", "i", "sleep [1] seconds"},

{"set-constant", "ffff", "set the julia constant to the quaternion [1]+[2]i+[3]j+[4]k"},
{"get-constant", "", "get the julia constant quaternion"},
{"set-slice", "fffff", "set the render volume to one with normal <[1] [2] [3] [4]>, [5] from the origin"},
{"get-slice-vector", "", "get the render volume normal"},
{"get-slice-distance", "", "get the render volume's distance from the origin"},
{"set-iterations", "i", "set the maximum iterations"},
{"get-iterations", "", "get the maximum iterations"},

{"set-camera", "ffff", "put the camera at radius [1], theta [2], phi [3] with FOV [4]"},
{"get-camera-radius", "", "get the radius of the camera"},
{"get-camera-theta", "", "get the theta of the camera"},
{"get-camera-phi", "", "get the phi of the camera"},
{"get-camera-fov", "", "get the FOV of the camera"},

{"add-light", "fffff", "add a light at theta [1], phi [2], color r[3], g[4], b[5]"},
{"clear-lights", "", "remove all lights"},

{"set-size", "i", "set the image size to [1] x [1] pixels"},
{"get-size", "", "get the current image size"},
{"set-supersampling", "i", "sample each pixel [1] times, then average"},
{"get-supersampling", "", "get the number of samples per pixel"},
{"set-occlusion", "ff", "set occlusion maximum to [1], and strength to [2]"},
{"get-occlusion-maximum", "", "get the occlusion maximum"},
{"get-occlusion-strength", "", "get the occlusion strength"},
{"set-epsilon", "ff", "set epsilon to [1] * pixel size, or [2], whichever is greater"},
{"get-epsilon-ratio", "", "get the epsilon ratio"},
{"get-epsilon-maximum", "", "get the epsilon maximum"},

{"render", "s", "render a PNG image to [1]"},

{"open-view", "", "open a window for rendering to"},
{"start-view", "", "start rendering to the open window"},
{"stop-view", "", "stop rendering to the open window"},
{"close-view", "", "close an opened render window"},

{"render-cloud", "s", "render a PLY point cloud to [1]"},

{NULL, NULL, "Man, this is a sucky easter egg."} };

unsigned int j_get_commands()
{
	unsigned int r = 0;
	while (_j_commands[r].command != NULL)
		++r;
	return r;
}

const char* j_get_command_name(unsigned int i)
{
	return _j_commands[i].command;
}

unsigned int j_get_command_arguments(unsigned int i)
{
	return strlen(_j_commands[i].argument_types);
}

unsigned int j_get_command_argument_type(unsigned int i, unsigned int c)
{
	return _j_commands[i].argument_types[c];
}

const char* j_get_command_description(unsigned int i)
{
	return _j_commands[i].description;
}

void _j_help()
{
	printf("Available commands:\n\n");
	unsigned int i, j;
	for (i = 0; i < j_get_commands(); ++i)
	{
		printf("%s", j_get_command_name(i));
		for (j = 0; j < j_get_command_arguments(i); ++j)
		{
			switch (j_get_command_argument_type(i, j))
			{
				case J_INTEGER:
					printf(" [%i : integer]", j + 1);
					break;
				case J_FLOAT:
					printf(" [%i : float]", j + 1);
					break;
				case J_STRING:
					printf(" [%i : string]", j + 1);
					break;
			};
		}
		printf("\n - %s\n", j_get_command_description(i));
	}
	printf("\n");
}

#define _J_EXECUTE(cname, run) if (strcmp(command_name, cname) == 0) { run; } else

unsigned int j_execute(const char* command)
{
	unsigned int i, bi, j;
	unsigned int arg = 0;
	char buffer[_J_BUFSIZE]; /* hopefully no commands greater than _J_BUFSIZE */
	unsigned int command_index;
	union _j_value_u* values = NULL;
	
	if (strlen(command) == 0)
		return 0 << 8;
	
	for (i = 0, bi = 0; i < strlen(command) + 1; ++i, ++bi)
	{
		if (bi >= _J_BUFSIZE)
			return 2 << 8;
		if (arg == 0)
		{
			if (command[i] == ' ' || command[i] == 0)
			{
				buffer[bi] = 0;
				j = 0;
				while (_j_commands[j].command != NULL)
				{
					/* FIXME: should be case-insensitive */
					if (strcmp(_j_commands[j].command, buffer) == 0)
					{
						command_index = j;
						break;
					}
					++j;
				}
				if (_j_commands[j].command == NULL)
					return 1 << 8;
				/*printf("Found command: %i %s\n", command_index, buffer);*/
				unsigned int memneeded = sizeof(union _j_value_u) * j_get_command_arguments(command_index);
				if (memneeded)
				{
					values = malloc(memneeded);
					if (values == NULL)
						return 3 << 8;
				}
				++arg;
				bi = -1;
				continue;
			}
			buffer[bi] = command[i];
		} else {
			if (values == NULL || arg > j_get_command_arguments(command_index))
			{
				if (values)
					_j_value_free(values, arg - 1, command_index);
				return 5 << 8;
			}
			if (command[i] == ' ' || command[i] == 0)
			{
				buffer[bi] = 0;
				
				char* endptr;
				
				switch (j_get_command_argument_type(command_index, arg - 1))
				{
					case J_FLOAT:
						values[arg - 1].floating = strtod(buffer, &endptr);
						if (endptr == buffer || endptr != buffer + strlen(buffer))
						{
							_j_value_free(values, arg - 1, command_index);
							return (7 << 8) | arg;
						}
						/*printf("found arg: %f\n", values[arg - 1].floating);*/
						break;
					case J_INTEGER:
						values[arg - 1].integer = strtol(buffer, &endptr, 10);
						if (endptr == buffer || endptr != buffer + strlen(buffer))
						{
							_j_value_free(values, arg - 1, command_index);
							return (8 << 8) | arg;
						}
						/*printf("found arg: %i\n", values[arg - 1].integer);*/
						break;
					case J_STRING:
						values[arg - 1].string = malloc(sizeof(char) * (strlen(buffer) + 1));
						strcpy(values[arg - 1].string, buffer);
						/*printf("found arg: [%s]\n", values[arg - 1].string);*/
						break;
					default:
						_j_value_free(values, arg - 1, command_index);
						return (4 << 8) | 0;
				};
				
				++arg;
				bi = -1;
				continue;
			}
			buffer[bi] = command[i];
		}
	}
	
	if (arg != j_get_command_arguments(command_index) + 1)
	{
		if (values)
			_j_value_free(values, arg - 1, command_index);
		return 6 << 8;
	}
	
	j_live_stop();
	
	const char* command_name = j_get_command_name(command_index);
	/* actually execute here */
	_J_EXECUTE("help", _j_help())
	
	_J_EXECUTE("sleep", sleep(values[0].integer))
	
	_J_EXECUTE("set-constant", j_julia_set_constant(j_vector_create(values[0].floating, values[1].floating, values[2].floating, values[3].floating)))
	_J_EXECUTE("get-constant", j_quaternion_print(j_julia_get_constant()); printf("\n"))
	_J_EXECUTE("set-slice", j_julia_set_slice(j_vector_create(values[0].floating, values[1].floating, values[2].floating, values[3].floating), values[4].floating))
	_J_EXECUTE("get-slice-vector", j_vector_print(j_julia_get_slice_vector()); printf("\n"))
	_J_EXECUTE("get-slice-distance", printf("%f\n", j_julia_get_slice_distance()))
	_J_EXECUTE("set-iterations", j_julia_set_iterations(values[0].integer))
	_J_EXECUTE("get-iterations", printf("%i\n", j_julia_get_iterations()))
	
	_J_EXECUTE("set-camera", j_raytracer_set_camera(values[0].floating, J_DEG2RAD(values[1].floating), J_DEG2RAD(values[2].floating), J_DEG2RAD(values[3].floating)))
	_J_EXECUTE("get-camera-radius", printf("%f\n", j_raytracer_get_camera_radius()))
	_J_EXECUTE("get-camera-theta", printf("%f\n", J_RAD2DEG(j_raytracer_get_camera_theta())))
	_J_EXECUTE("get-camera-phi", printf("%f\n", J_RAD2DEG(j_raytracer_get_camera_phi())))
	_J_EXECUTE("get-camera-fov", printf("%f\n", J_RAD2DEG(j_raytracer_get_camera_fov())))
	
	_J_EXECUTE("add-light", j_raytracer_add_light(J_DEG2RAD(values[0].floating), J_DEG2RAD(values[1].floating), j_vector_create(values[2].floating, values[3].floating, values[4].floating, 0)))
	_J_EXECUTE("clear-lights", j_raytracer_clear_lights())
	
	_J_EXECUTE("set-size", j_raytracer_set_image_size(values[0].integer))
	_J_EXECUTE("get-size", printf("%i\n", j_raytracer_get_image_size()))
	_J_EXECUTE("set-supersampling", j_raytracer_set_supersampling(values[0].integer))
	_J_EXECUTE("get-supersampling", printf("%i\n", j_raytracer_get_supersampling()))
	_J_EXECUTE("set-occlusion", j_raytracer_set_occlusion(values[0].floating, values[1].floating))
	_J_EXECUTE("get-occlusion-maximum", printf("%f\n", j_raytracer_get_occlusion_maximum()))
	_J_EXECUTE("get-occlusion-strength", printf("%f\n", j_raytracer_get_occlusion_strength()))
	_J_EXECUTE("set-epsilon", j_raytracer_set_epsilon(values[0].floating, values[1].floating))
	_J_EXECUTE("get-epsilon-ratio", printf("%f\n", j_raytracer_get_epsilon_ratio()))
	_J_EXECUTE("get-epsilon-maximum", printf("%f\n", j_raytracer_get_epsilon_minimum()))
	
	_J_EXECUTE("render", j_image_render(values[0].string))
	
	_J_EXECUTE("open-view", j_live_open())
	_J_EXECUTE("start-view", j_live_start())
	_J_EXECUTE("stop-view", j_live_stop())
	_J_EXECUTE("close-view", j_live_close())
	
	_J_EXECUTE("render-cloud", j_julia_render_cloud(values[0].string))
	
	/* if none of the above matched, run this */ {
		if (values)
			_j_value_free(values, arg - 1, command_index);
		return (9 << 8) | command_index;
	}
	
	if (values)
		_j_value_free(values, arg - 1, command_index);
	
	return 0 << 8;
}


const char* j_get_error(unsigned int i)
{
	switch ((i & 0xFF00) >> 8)
	{
		case 0:
			return "no error";
		case 1:
			return "invalid command";
		case 2:
			return "buffer overflow";
		case 3:
			return "insufficient memory";
		case 4:
			return "impossible internal error breed %i";
		case 5:
			return "too many arguments";
		case 6:
			return "too few arguments";
		case 7:
			return "argument %i was not a valid float";
		case 8:
			return "argument %i was not a valid integer";
		case 9:
			return "command %i is not yet implemented";
	};
}

unsigned int j_get_error_argument(unsigned int i)
{
	return i & 0xFF;
}


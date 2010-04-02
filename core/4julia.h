/* 4julia.h */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __4JULIA_H__
#define __4JULIA_H__

#define J_NAME "4julia renderer"
#define J_VERSION "0.3"

/* core.c */

void j_initialize();
void j_exit();

#define J_STRING 's'
#define J_INTEGER 'i'
#define J_FLOAT 'f'

unsigned int j_get_commands();
const char* j_get_command_name(unsigned int i);
unsigned int j_get_command_arguments(unsigned int i);
unsigned int j_get_command_argument_type(unsigned int i, unsigned int c);
const char* j_get_command_description(unsigned int i);
unsigned int j_execute(const char* command);
const char* j_get_error(unsigned int i);
unsigned int j_get_error_argument(unsigned int i);

/* math.c */

#define J_MIN(a, b) (a < b ? a : b)
#define J_MAX(a, b) (a > b ? a : b)
#define J_CLAMP(x, a, b) (J_MAX(J_MIN(x, b), a))

#define J_RAD2DEG(a) (a * (180 / M_PI))
#define J_DEG2RAD(a) (a * (M_PI / 180))

struct j_vector_s
{
	double x;
	double y;
	double z;
	double w;
};

typedef struct j_vector_s j_vector;

struct j_matrix_s
{
	double data[4][4]; // data[h][w]
};

typedef struct j_matrix_s j_matrix;

j_vector j_vector_create(double x, double y, double z, double w);
j_vector j_vector_create_axis(unsigned int axis);
unsigned int j_vector_equals(j_vector a, j_vector b);
j_vector j_vector_add(j_vector a, j_vector b);
j_vector j_vector_negate(j_vector a);
j_vector j_vector_subtract(j_vector a, j_vector b);
j_vector j_vector_multiply(j_vector a, double b);
j_vector j_vector_multiply_components(j_vector a, j_vector b);
j_vector j_vector_divide_components(j_vector a, j_vector b);
double j_vector_dot(j_vector a, j_vector b);
j_vector j_vector_cross(j_vector a, j_vector b);
double j_vector_magnitude_squared(j_vector a);
double j_vector_magnitude(j_vector a);
j_vector j_vector_normalize(j_vector a);
j_vector j_vector_reflect(j_vector a, j_vector norm);
j_vector j_vector_rotate(j_vector in, j_vector axis, double theta);
void j_vector_print(j_vector a);

j_vector j_quaternion_multiply(j_vector a, j_vector b);
void j_quaternion_print(j_vector a);

j_matrix j_matrix_create(double* data); /* data in row, row, row, row format */
j_matrix j_matrix_create_rotation(j_vector axis, double theta);
j_vector j_matrix_transform_vector(j_matrix m, j_vector v);
void j_matrix_print(j_matrix m);

/* raytracer.c */

/* theta is elevation */
void j_raytracer_set_camera(double radius, double theta, double phi, double fov);
double j_raytracer_get_camera_radius();
double j_raytracer_get_camera_theta();
double j_raytracer_get_camera_phi();
double j_raytracer_get_camera_fov();
j_vector j_raytracer_get_camera_vector();
j_vector j_raytracer_get_camera_up_vector();
j_vector j_raytracer_get_camera_side_vector();
j_vector j_raytracer_get_camera_position();
/* x and y are [-1, 1] */
j_vector j_raytracer_get_camera_ray(double x, double y);

struct j_light_s
{
	double theta;
	double phi;
	j_vector position;
	j_vector color;
};

typedef struct j_light_s j_light;

j_light j_raytracer_add_light(double theta, double phi, j_vector color);
void j_raytracer_remove_light(j_light l);
unsigned int j_raytracer_get_lights();
j_light j_raytracer_get_light(unsigned int i);
void j_raytracer_clear_lights();

void j_raytracer_set_image_size(unsigned int size);
unsigned int j_raytracer_get_image_size();
j_vector j_raytracer_get_image_ray(double x, double y);

void j_raytracer_set_supersampling(unsigned int samples);
unsigned int j_raytracer_get_supersampling();

void j_raytracer_set_epsilon(double e_ratio, double e_min);
double j_raytracer_get_epsilon_ratio();
double j_raytracer_get_epsilon_minimum(); /* wow actually a max, huh */

struct j_raycast_s
{
	unsigned int hit; /* 1 when we hit something */
	unsigned int itercount; /* how many iters we went through */
	j_vector point; /* the point we hit */
	j_vector normal; /* the normal of the surface we hit */
};

typedef struct j_raycast_s j_raycast;

j_raycast j_raytracer_cast(j_vector origin, j_vector ray, unsigned int gen_normal);
j_vector j_raytracer_shader(j_raycast cast);
j_vector j_raytracer_render(unsigned int x, unsigned int y);

void j_raytracer_initialize();
void j_raytracer_exit();

/* image.c */

int j_image_open(const char* name);
int j_image_write_row(j_vector* row);
int j_image_close();

void j_image_render(const char* name);

/* julia.c */

void j_julia_set_slice(j_vector v, double d);
j_vector j_julia_get_slice_vector();
double j_julia_get_slice_distance();
j_vector j_julia_get_origin();
j_vector j_julia_get_x_basis();
j_vector j_julia_get_y_basis();
j_vector j_julia_get_z_basis();
j_vector j_julia_transform(j_vector a);

void j_julia_set_constant(j_vector v);
j_vector j_julia_get_constant();

void j_julia_set_iterations(unsigned int i);
unsigned int j_julia_get_iterations();

double j_julia_estimate(j_vector p);
unsigned int j_julia_in_set(j_vector p);

void j_julia_render_cloud(const char* fname);

void j_julia_initialize();

/* live.c */

void j_live_open();
void j_live_start();
void j_live_stop();
void j_live_close();
void j_live_wait();

void j_live_initialize();
void j_live_exit();

#endif /* __4JULIA_H__ */


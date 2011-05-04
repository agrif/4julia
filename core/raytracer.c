#include "4julia.h"

/* let's define axes...
   + X is to the right
   - Y is towards the screen (theta 0 phi 0)
   + Z is up
   
   it's like a normal graph on the table, that you are looking
   parallel to the table surface */

/* camera */

double _j_raytracer_camera_radius, _j_raytracer_camera_theta;
double _j_raytracer_camera_phi, _j_raytracer_camera_fov;
double _j_raytracer_camera_fov_over_2;
j_vector _j_raytracer_camera_vector, _j_raytracer_camera_up_vector;
j_vector _j_raytracer_camera_side_vector, _j_raytracer_camera_position;

void _j_raytracer_update_lights();

void j_raytracer_set_camera(double radius, double theta, double phi, double fov)
{	
	_j_raytracer_camera_radius = radius;
	_j_raytracer_camera_theta = theta;
	_j_raytracer_camera_phi = phi;
	_j_raytracer_camera_fov = fov;
	_j_raytracer_camera_fov_over_2 = fov / 2;
	
	/* because these rotations are ccw, negate theta so + means up */
	theta = -theta;
	/* phi doesn't need it. + means ccw, which means right, which makes more sense */
	
	j_vector position = j_vector_negate(j_vector_create_axis(1));
	position = j_vector_multiply(position, radius);
	position = j_vector_rotate(position, j_vector_create_axis(0), theta);
	position = j_vector_rotate(position, j_vector_create_axis(2), phi);
	
	j_vector vector = j_vector_normalize(j_vector_negate(position));
	j_vector up, side;
	
	if (theta <= - M_PI / 2) // remember the negate, -pi / 2 is actually up
	{
		up = j_vector_create_axis(1);
		up = j_vector_rotate(up, j_vector_create_axis(2), phi);
		side = j_vector_cross(vector, up);
	} else if (theta >= M_PI / 2) {
		up = j_vector_negate(j_vector_create_axis(1));
		up = j_vector_rotate(up, j_vector_create_axis(2), phi);
		side = j_vector_cross(vector, up);
	} else {
		up = j_vector_create_axis(2);
		side = j_vector_cross(vector, up);
		up = j_vector_rotate(vector, side, (M_PI / 2));
	}
	
	_j_raytracer_camera_vector = vector;
	_j_raytracer_camera_up_vector = j_vector_normalize(up);
	_j_raytracer_camera_side_vector = j_vector_normalize(side);
	_j_raytracer_camera_position = position;
	
	_j_raytracer_update_lights();
}

double j_raytracer_get_camera_radius()
{
	return _j_raytracer_camera_radius;
}

double j_raytracer_get_camera_theta()
{
	return _j_raytracer_camera_theta;
}

double j_raytracer_get_camera_phi()
{
	return _j_raytracer_camera_phi;
}

double j_raytracer_get_camera_fov()
{
	return _j_raytracer_camera_fov;
}

j_vector j_raytracer_get_camera_vector()
{
	return _j_raytracer_camera_vector;
}

j_vector j_raytracer_get_camera_up_vector()
{
	return _j_raytracer_camera_up_vector;
}

j_vector j_raytracer_get_camera_side_vector()
{
	return _j_raytracer_camera_side_vector;
}

j_vector j_raytracer_get_camera_position()
{
	return _j_raytracer_camera_position;
}

j_vector j_raytracer_get_camera_ray(double x, double y)
{
	j_vector r = j_vector_rotate(_j_raytracer_camera_vector, _j_raytracer_camera_side_vector,
		y * _j_raytracer_camera_fov_over_2);
	r = j_vector_rotate(r, _j_raytracer_camera_up_vector, -x * _j_raytracer_camera_fov_over_2);
	
	return r;
}

/* light */

struct _j_raytracer_light_list_s
{
	j_light light;
	struct _j_raytracer_light_list_s* next;
};

struct _j_raytracer_light_list_s* _j_raytracer_lights;

void _j_raytracer_update_lights()
{
	struct _j_raytracer_light_list_s* this = _j_raytracer_lights;
	while (this != NULL)
	{
		j_vector position = j_vector_negate(j_vector_create_axis(1));
		position = j_vector_multiply(position, _j_raytracer_camera_radius);
		position = j_vector_rotate(position, j_vector_create_axis(0), -(_j_raytracer_camera_theta + this->light.theta));
		position = j_vector_rotate(position, j_vector_create_axis(2), _j_raytracer_camera_phi + this->light.phi);
		this->light.position = position;
		this = this->next;
	}
}

j_light j_raytracer_add_light(double theta, double phi, j_vector color)
{
	struct _j_raytracer_light_list_s** nextptr = &_j_raytracer_lights;
	unsigned int index = 0;
	while (*nextptr != NULL)
	{
		++index;
		nextptr = &((*nextptr)->next);
	}
	
	struct _j_raytracer_light_list_s* new = malloc(sizeof(struct _j_raytracer_light_list_s));
	new->light.theta = theta;
	new->light.phi = phi;
	new->light.color = color;
	new->next = NULL;
	(*nextptr) = new;
	_j_raytracer_update_lights();
	return new->light;
}

void j_raytracer_remove_light(j_light l)
{
	struct _j_raytracer_light_list_s** lastptr = &_j_raytracer_lights;
	struct _j_raytracer_light_list_s* this = *lastptr;
	
	while (this != NULL)
	{
		if (j_vector_equals(this->light.position, l.position) && j_vector_equals(this->light.color, l.color))
		{
			*lastptr = this->next;
			free(this);
			return;
		} else {
			lastptr = &(this->next);
			this = *lastptr;
		}
	}
}

unsigned int j_raytracer_get_lights()
{
	struct _j_raytracer_light_list_s* this = _j_raytracer_lights;
	unsigned int len = 0;
	while (this != NULL)
	{
		len++;
		this = this->next;
	}
	return len;
}

j_light j_raytracer_get_light(unsigned int i)
{
	struct _j_raytracer_light_list_s* this = _j_raytracer_lights;
	unsigned int len = 0;
	while (this != NULL)
	{
		if (len == i)
		{
			return this->light;
		}
		this = this->next;
		len++;
	}
	j_light l;
	return l;
}

void j_raytracer_clear_lights()
{
	struct _j_raytracer_light_list_s* this = _j_raytracer_lights;
	struct _j_raytracer_light_list_s* next;
	while (this != NULL)
	{
		next = this->next;
		free(this);
		this = next;
	}
	_j_raytracer_lights = NULL;
}

/* image */

unsigned int _j_raytracer_image_size;

void j_raytracer_set_image_size(unsigned int size)
{
	_j_raytracer_image_size = size;
}

unsigned int j_raytracer_get_image_size()
{
	return _j_raytracer_image_size;
}

j_vector j_raytracer_get_image_ray(double x, double y)
{
	double xd = (((x) / _j_raytracer_image_size) * 2) - 1;
	double yd = (((y) / _j_raytracer_image_size) * 2) - 1;
	
	return j_raytracer_get_camera_ray(xd, -yd); /* images are top to bottom, math is bottom to top */
}

/* supersampling */

#include "jitter.h"
/* includes _j_jitter[30][32][2], with [aalevel - 2][sample num][x or y] in [-0.5, 0.5] */

unsigned int _j_raytracer_supersampling;

void j_raytracer_set_supersampling(unsigned int samples)
{
	_j_raytracer_supersampling = J_MIN(samples, _J_MAX_SUPERSAMPLING);
}

unsigned int j_raytracer_get_supersampling()
{
	return _j_raytracer_supersampling;
}

/* occlusion */

double _j_raytracer_occlusion_max, _j_raytracer_occlusion_strength;

void j_raytracer_set_occlusion(double max, double strength)
{
	_j_raytracer_occlusion_max = max;
	_j_raytracer_occlusion_strength = strength;
}

double j_raytracer_get_occlusion_maximum()
{
	return _j_raytracer_occlusion_max;
}

double j_raytracer_get_occlusion_strength()
{
	return _j_raytracer_occlusion_strength;
}

/* epsilon */

double _j_raytracer_epsilon_minimum, _j_raytracer_epsilon_ratio;

void j_raytracer_set_epsilon(double e_ratio, double e_min)
{
	_j_raytracer_epsilon_minimum = e_min;
	_j_raytracer_epsilon_ratio = e_ratio;
}

double j_raytracer_get_epsilon_ratio()
{
	return _j_raytracer_epsilon_ratio;
}

double j_raytracer_get_epsilon_minimum()
{
	return _j_raytracer_epsilon_minimum;
}


/* raycast */

j_raycast j_raytracer_cast(j_vector origin, j_vector ray, unsigned int gen_normal)
{
	/* let's assume everything we want to see is in a cameraradius bounding sphere
	   so let's stop looking after 2*cameraradius. */
	
	double stoplength = 2 * _j_raytracer_camera_radius;
	
	ray = j_vector_normalize(ray);
	double curlength = 0;
	j_vector curpos = origin;
	
	j_raycast r;
	r.itercount = 0;
	
	double epsilon;
	double d;
	while (curlength < stoplength)
	{
		d = j_julia_estimate(curpos);
		
		epsilon = 2 * j_vector_magnitude_squared(j_vector_subtract(curpos, origin)) * (1 - cos(_j_raytracer_camera_fov));
		epsilon = sqrt(epsilon) / _j_raytracer_image_size;
		epsilon = J_MAX(epsilon * _j_raytracer_epsilon_ratio, _j_raytracer_epsilon_minimum);
		
		if (d < epsilon)
		{
			break;
		} else if (d > 1) {
			d = 1;
		}
		
		r.itercount++;
		curlength += d;
		curpos = j_vector_add(curpos, j_vector_multiply(ray, d));
	}
		
	if (curlength >= stoplength)
	{
		r.hit = 0;
		return r;
	}
	
	r.hit = 1;
	r.point = curpos;
	
	if (gen_normal)
	{
		r.normal.x = j_julia_estimate(j_vector_create(curpos.x + _j_raytracer_epsilon_minimum, curpos.y, curpos.z, 0));
		r.normal.x -= j_julia_estimate(j_vector_create(curpos.x - _j_raytracer_epsilon_minimum, curpos.y, curpos.z, 0));
		r.normal.y = j_julia_estimate(j_vector_create(curpos.x, curpos.y + _j_raytracer_epsilon_minimum, curpos.z, 0));
		r.normal.y -= j_julia_estimate(j_vector_create(curpos.x, curpos.y - _j_raytracer_epsilon_minimum, curpos.z, 0));
		r.normal.z = j_julia_estimate(j_vector_create(curpos.x, curpos.y, curpos.z + _j_raytracer_epsilon_minimum, 0));
		r.normal.z -= j_julia_estimate(j_vector_create(curpos.x, curpos.y, curpos.z - _j_raytracer_epsilon_minimum, 0));
	
		r.normal = j_vector_normalize(r.normal);
	}
	
	return r;
}

j_vector j_raytracer_shader(j_raycast cast)
{
	j_vector r;
	if (cast.hit == 0)
	{
		r.x = 0;
		r.y = 0;
		r.z = 0;
		r.w = 0;
		return r;
	}

	r.x = 0;
	r.y = 0;
	r.z = 0;

	unsigned int lights = j_raytracer_get_lights();
	unsigned int i;
	for (i = 0; i < lights; ++i)
	{
		j_light l = j_raytracer_get_light(i);
		j_vector light_vector = j_vector_subtract(l.position, cast.point);
		double lambert = j_vector_dot(cast.normal, j_vector_normalize(light_vector));
		if (lambert > 0)
		{
			r = j_vector_add(r, j_vector_multiply(l.color, lambert));
		}
    }
	double occlusion = 1.0 - (cast.itercount * _j_raytracer_occlusion_strength);
	occlusion = 1.0 - _j_raytracer_occlusion_max + (occlusion * _j_raytracer_occlusion_max);
	r = j_vector_multiply(r, occlusion);
	r.w = 1;
	return r;
}

j_vector j_raytracer_render(unsigned int x, unsigned int y)
{
	if (_j_raytracer_supersampling < 2)
	{
		return j_raytracer_shader(j_raytracer_cast(_j_raytracer_camera_position, j_raytracer_get_image_ray(x, y), 1));
	}
	
	j_vector r = j_vector_create(0, 0, 0, 0);
	unsigned int i, solidhits;
	for (i = 0, solidhits = 0; i < _j_raytracer_supersampling; ++i)
	{
		j_vector tmp = j_raytracer_get_image_ray(x + _j_jitter[_j_raytracer_supersampling - 2][i][0], y + _j_jitter[_j_raytracer_supersampling - 2][i][1]);
		tmp = j_raytracer_shader(j_raytracer_cast(_j_raytracer_camera_position, tmp, 1));
		if (tmp.w == 1)
		{
			solidhits++;
			r = j_vector_add(r, tmp);
		}
	}
	if (solidhits)
		r = j_vector_multiply(r, 1.0 / solidhits);
	r.w = ((double)(solidhits)) / _j_raytracer_supersampling;
	return r;
}

/* init */

void j_raytracer_initialize()
{
	j_raytracer_set_camera(4, 0, 0, 50 * (M_PI / 180));
	_j_raytracer_lights = NULL;
	j_raytracer_add_light(0, 0, j_vector_create(0.3, 0.3, 0.3, 0));
	j_raytracer_set_image_size(256);
	j_raytracer_set_occlusion(0.8, 1.0/30);
	j_raytracer_set_epsilon(.1, .00000001);
	j_raytracer_set_supersampling(1);
}

void j_raytracer_exit()
{
	j_raytracer_clear_lights();
}


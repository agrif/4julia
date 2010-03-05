#include "4julia.h"

/* slice */

double _j_julia_slice_distance;
j_vector _j_julia_slice, _j_julia_x_basis, _j_julia_y_basis;
j_vector _j_julia_z_basis, _j_julia_origin;

void j_julia_set_slice(j_vector v, double d)
{
	_j_julia_slice = v;
	_j_julia_slice_distance = d;
	v = j_vector_normalize(v);
	_j_julia_origin = j_vector_multiply(v, d);
	
	j_vector x, y, z;
	unsigned int axis = 0;
	unsigned int axismask = 0;
	
	for (axis = 0; axis < 4; ++axis)
	{
		if (j_vector_equals(v, j_vector_create_axis(axis)))
		{
			axismask |= 1 << axis;
			break;
		}
	}
	
	for (axis = 0; axis < 4; ++axis)
	{
		if (!(axismask & (1 << axis)))
		{
			axismask |= 1 << axis;
			x = j_vector_create_axis(axis);
			break;
		}
	}
	
	x = j_vector_subtract(x, j_vector_multiply(v, j_vector_dot(x, v)));
	x = j_vector_normalize(x);
	
	for (axis = 0; axis < 4; ++axis)
	{
		if (!(axismask & (1 << axis)))
		{
			axismask |= 1 << axis;
			y = j_vector_create_axis(axis);
			break;
		}
	}
	
	y = j_vector_subtract(y, j_vector_multiply(v, j_vector_dot(y, v)));
	y = j_vector_subtract(y, j_vector_multiply(x, j_vector_dot(y, x)));
	y = j_vector_normalize(y);
	
	for (axis = 0; axis < 4; ++axis)
	{
		if (!(axismask & (1 << axis)))
		{
			axismask |= 1 << axis;
			z = j_vector_create_axis(axis);
			break;
		}
	}
	
	z = j_vector_subtract(z, j_vector_multiply(v, j_vector_dot(z, v)));
	z = j_vector_subtract(z, j_vector_multiply(x, j_vector_dot(z, x)));
	z = j_vector_subtract(z, j_vector_multiply(y, j_vector_dot(z, y)));
	z = j_vector_normalize(z);
	
	_j_julia_x_basis = x;
	_j_julia_y_basis = y;
	_j_julia_z_basis = z;
}

j_vector j_julia_get_slice_vector()
{
	return _j_julia_slice;
}

double j_julia_get_slice_distance()
{
	return _j_julia_slice_distance;
}

j_vector j_julia_get_origin()
{
	return _j_julia_origin;
}

j_vector j_julia_get_x_basis()
{
	return _j_julia_x_basis;
}

j_vector j_julia_get_y_basis()
{
	return _j_julia_y_basis;
}

j_vector j_julia_get_z_basis()
{
	return _j_julia_z_basis;
}

j_vector j_julia_transform(j_vector a)
{
	j_vector r = j_vector_multiply(_j_julia_x_basis, a.x);
	r = j_vector_add(r, j_vector_multiply(_j_julia_y_basis, a.y));
	r = j_vector_add(r, j_vector_multiply(_j_julia_z_basis, a.z));
	r = j_vector_add(r, _j_julia_origin);
	return r;
}

/* constant */

j_vector _j_julia_constant;

void j_julia_set_constant(j_vector v)
{
	_j_julia_constant = v;
}

j_vector j_julia_get_constant()
{
	return _j_julia_constant;
}

/* iterations */

unsigned int _j_julia_iterations;

void j_julia_set_iterations(unsigned int i)
{
	_j_julia_iterations = i;
}

unsigned int j_julia_get_iterations()
{
	return _j_julia_iterations;
}

/* estimate */

#define _J_JULIA_BOUND_SPHERE 2
#define _J_JULIA_BOUND_SPHERE_SQUARED (_J_JULIA_BOUND_SPHERE * _J_JULIA_BOUND_SPHERE)

double j_julia_estimate(j_vector p)
{
	j_vector f = j_julia_transform(p);
	j_vector fp = j_vector_create(1, 0, 0, 0);
	unsigned int n;
	
	for (n = 0; n < _j_julia_iterations; ++n)
	{
		if (j_vector_magnitude_squared(f) > _J_JULIA_BOUND_SPHERE_SQUARED)
			break;
		fp = j_vector_multiply(j_quaternion_multiply(f, fp), 2);
		f = j_vector_add(j_quaternion_multiply(f, f), _j_julia_constant);
	}
	
	double fm = j_vector_magnitude(f);
	
	//j_vector_print(f);
	//j_vector_print(fp);
	//printf(" %f\n", fm);
	
	return (fm * log(fm)) / (2 * j_vector_magnitude(fp));
}

unsigned int j_julia_in_set(j_vector p)
{
	j_vector f = j_julia_transform(p);
	unsigned int n;
	
	for (n = 0; n < _j_julia_iterations; ++n)
	{
		if (j_vector_magnitude_squared(f) > _J_JULIA_BOUND_SPHERE_SQUARED)
			return 0;
		f = j_vector_add(j_quaternion_multiply(f, f), _j_julia_constant);
	}
	
	return 1;
}

/* render point cloud */

void _j_render_intro(const char* name, unsigned int size);
void _j_render_bar(unsigned int row, unsigned int size);

void j_julia_render_cloud(const char* fname)
{
	FILE* f = fopen(fname, "w");
	if (!f)
		return;
	
	unsigned int size = j_raytracer_get_image_size();
	double step = (2.0 * _J_JULIA_BOUND_SPHERE) / (size - 1);
	unsigned int count = 0;
	
	double x, y, z;
	unsigned int zint;
	j_vector p;
	
	_j_render_intro(fname, size);
	
	zint = 0;
	for (z = -_J_JULIA_BOUND_SPHERE; z <= _J_JULIA_BOUND_SPHERE; z += step)
	{
		for (y = -_J_JULIA_BOUND_SPHERE; y <= _J_JULIA_BOUND_SPHERE; y += step)
		{
			for (x = -_J_JULIA_BOUND_SPHERE; x <= _J_JULIA_BOUND_SPHERE; x += step)
			{
				p = j_vector_create(x, y, z, 0.0);
				if (j_julia_in_set(p) == 0)
					continue;
				++count;
			}
		}
		_j_render_bar(zint, size * 2);
		++zint;
	}
	
	fprintf(f, "ply\nformat ascii 1.0\n");
	fprintf(f, "comment Created by " J_NAME " " J_VERSION "\n");
	fprintf(f, "element vertex %i\n", count);
	fprintf(f, "property float x\nproperty float y\nproperty float z\n");
	fprintf(f, "end_header\n");
	
	zint = 0;
	for (z = -_J_JULIA_BOUND_SPHERE; z <= _J_JULIA_BOUND_SPHERE; z += step)
	{
		for (y = -_J_JULIA_BOUND_SPHERE; y <= _J_JULIA_BOUND_SPHERE; y += step)
		{
			for (x = -_J_JULIA_BOUND_SPHERE; x <= _J_JULIA_BOUND_SPHERE; x += step)
			{
				p = j_vector_create(x, y, z, 0.0);
				if (j_julia_in_set(p) == 0)
					continue;
				fprintf(f, "%f %f %f\n", x, y, z);
			}
		}
		_j_render_bar(zint + size, size * 2);
		++zint;
	}
	
	_j_render_bar(size * 2, size * 2);
	printf("\n");
	
	fclose(f);
}

/* init */

void j_julia_initialize()
{
	j_julia_set_slice(j_vector_create(0, 0, 0, 1), 0);
	j_julia_set_constant(j_vector_create(0, 0, 0, 0));
	j_julia_set_iterations(50);
}


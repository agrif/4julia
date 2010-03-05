#include "4julia.h"

j_vector j_vector_create(double x, double y, double z, double w)
{
	j_vector r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

j_vector j_vector_create_axis(unsigned int axis)
{
	j_vector r;
	r.x = (axis == 0 ? 1 : 0);
	r.y = (axis == 1 ? 1 : 0);
	r.z = (axis == 2 ? 1 : 0);
	r.w = (axis == 3 ? 1 : 0);
	return r;
}


unsigned int j_vector_equals(j_vector a, j_vector b)
{
	return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}

j_vector j_vector_add(j_vector a, j_vector b)
{
	j_vector r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	r.w = a.w + b.w;
	return r;
}

j_vector j_vector_negate(j_vector a)
{
	j_vector r;
	r.x = -a.x;
	r.y = -a.y;
	r.z = -a.z;
	r.w = -a.w;
	return r;
}

j_vector j_vector_subtract(j_vector a, j_vector b)
{
	j_vector r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	r.w = a.w - b.w;
	return r;
}

j_vector j_vector_multiply(j_vector a, double b)
{
	j_vector r;
	r.x = a.x * b;
	r.y = a.y * b;
	r.z = a.z * b;
	r.w = a.w * b;
	return r;
}

j_vector j_vector_multiply_components(j_vector a, j_vector b)
{
	j_vector r;
	r.x = a.x * b.x;
	r.y = a.y * b.y;
	r.z = a.z * b.z;
	r.w = a.w * b.w;
	return r;
}

j_vector j_vector_divide_components(j_vector a, j_vector b)
{
	j_vector r;
	r.x = a.x / b.x;
	r.y = a.y / b.y;
	r.z = a.z / b.z;
	r.w = a.w / b.w;
	return r;
}

double j_vector_dot(j_vector a, j_vector b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

j_vector j_vector_cross(j_vector a, j_vector b)
{
	j_vector r;
	r.x = (a.y * b.z) - (a.z * b.y);
	r.y = (a.z * b.x) - (a.x * b.z);
	r.z = (a.x * b.y) - (a.y * b.x);
	r.w = 0;
	return r;
}

double j_vector_magnitude_squared(j_vector a)
{
	return (a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w);
}

double j_vector_magnitude(j_vector a)
{
	return sqrt(j_vector_magnitude_squared(a));
}

j_vector j_vector_normalize(j_vector a)
{
	double mag = j_vector_magnitude_squared(a);
	if (mag == 1)
		return a;
	
	mag = 1 / sqrt(mag);
	return j_vector_multiply(a, mag);
}

j_vector j_vector_reflect(j_vector a, j_vector norm)
{
	a = j_vector_normalize(a);
	norm = j_vector_normalize(norm);
	
	return j_vector_subtract(a, j_vector_multiply(norm, 2 * j_vector_dot(a, norm)));
}

j_vector j_vector_rotate(j_vector in, j_vector axis, double theta)
{
	return j_matrix_transform_vector(j_matrix_create_rotation(axis, theta), in);
}

void j_vector_print(j_vector a)
{
	printf("<%f, %f, %f, %f>", a.x, a.y, a.z, a.w);
}

j_vector j_quaternion_multiply(j_vector a, j_vector b)
{
	j_vector r;
	r.x = (a.x * b.x) - (a.y * b.y) - (a.z * b.z) - (a.w * b.w);
	r.y = (a.x * b.y) + (a.y * b.x) + (a.z * b.w) - (a.w * b.z);
	r.z = (a.x * b.z) - (a.y * b.w) + (a.z * b.x) + (a.w * b.y);
	r.w = (a.x * b.w) + (a.y * b.z) - (a.z * b.y) + (a.w * b.x);
	return r;
}

void j_quaternion_print(j_vector a)
{
	printf("%f + %fi + %fj + %fk", a.x, a.y, a.z, a.w);
}

j_matrix j_matrix_create(double* data)
{
	j_matrix r;
	unsigned int n, i = 0, j = 0;
	for (n = 0; n < 16; ++n)
	{
		r.data[j][i] = data[n];
		++i;
		if (i >= 4)
		{
			i = 0;
			++j;
		}
	}
	return r;
}

j_matrix j_matrix_create_rotation(j_vector axis, double theta)
{
	double c = cos(theta);
	double s = sin(theta);
	axis = j_vector_normalize(axis);
	j_matrix r;
	
	r.data[0][0] = (axis.x * axis.x) + ((1 - (axis.x * axis.x)) * c);
	r.data[0][1] = ((axis.x * axis.y) * (1 - c)) - (axis.z * s);
	r.data[0][2] = ((axis.x * axis.z) * (1 - c)) + (axis.y * s);
	r.data[0][3] = 0;
	
	r.data[1][0] = ((axis.x * axis.y) * (1 - c)) + (axis.z * s);
	r.data[1][1] = (axis.y * axis.y) + ((1 - (axis.y * axis.y)) * c);
	r.data[1][2] = ((axis.y * axis.z) * (1 - c)) - (axis.x * s);
	r.data[1][3] = 0;
	
	r.data[2][0] = ((axis.x * axis.z) * (1 - c)) - (axis.y * s);
	r.data[2][1] = ((axis.y * axis.z) * (1 - c)) + (axis.x * s);
	r.data[2][2] = (axis.z * axis.z) + ((1 - (axis.z * axis.z)) * c);
	r.data[2][3] = 0;
	
	r.data[3][0] = 0;
	r.data[3][1] = 0;
	r.data[3][2] = 0;
	r.data[3][3] = 1;
	
	return r;
}

j_vector j_matrix_transform_vector(j_matrix m, j_vector v)
{
	/* for reference, to multiply matrices:
	   (I'm using standard w,h notation, not the crazy matrix standard h,w)
	   a M x N times a P x M will give a P X N matrix
	   the first width must == the second height
	   to get data (x, y) in the result, multiply row y of the first by column x of the second
	   there, I'm done */
	
	j_vector r;
	r.x = (m.data[0][0] * v.x) + (m.data[0][1] * v.y) + (m.data[0][2] * v.z) + (m.data[0][3] * v.w);
	r.y = (m.data[1][0] * v.x) + (m.data[1][1] * v.y) + (m.data[1][2] * v.z) + (m.data[1][3] * v.w);
	r.z = (m.data[2][0] * v.x) + (m.data[2][1] * v.y) + (m.data[2][2] * v.z) + (m.data[2][3] * v.w);
	r.w = (m.data[3][0] * v.x) + (m.data[3][1] * v.y) + (m.data[3][2] * v.z) + (m.data[3][3] * v.w);
	return r;
}

void j_matrix_print(j_matrix m)
{
	unsigned int n;
	printf("[");
	for (n = 0; n < 4; ++n)
	{
		printf("[ %f %f %f %f ]", m.data[n][0], m.data[n][1], m.data[n][2], m.data[n][3]);
		if (n < 3)
			printf("\n ");
	}
	printf("]\n");
}


#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	float x;
	float y;
} vec2_t;

typedef struct {
	float x;
	float y;
	float z;
} vec3_t;


// TODO: add functions to manipulate vectors 2D and 3D
// ***
vec3_t vec3_rotate_x(vec3_t v, float angle); 
vec3_t vec3_rotate_y(vec3_t v, float angle); 
vec3_t vec3_rotate_z(vec3_t v, float angle); 


#endif // !VECTOR_H

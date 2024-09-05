#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

#include "display.h"
#include "vector.h"

#include "mesh.h"

#include "array.h"



//pure c arrays not dynamic ->>> N_MESH_FACES
//triangle_t triangles_to_render[N_MESH_FACES];
// --
// first pos of array triangle_t*
triangle_t* triangles_to_render = NULL;


vec3_t camera_position = { .x = 0, .y = 0,  .z = -5 };




float fov_factor = 640;

bool is_running = false;

int previous_frame_time = 0;


void setup(void) {

	//Allocate the required memory in bytes to hold the color buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	//Eðer tahsis baþarýlý olursa, malloc tahsis edilen bloðun ilk pozisyonuna bir iþaretçi döndürecektir.
	//malloc'un bellekte o kadar bayt ayýrmayý baþaramamasý (belki de makinede yeterli boþ bellek yoktur). Eðer bu olursa, malloc bir NULL iþaretçisi döndürecektir.
	//*******************************************************************************************************
	// ***** Ancak þunu her zaman unutmayýn ki, profesyonel üretim kodunda fonksiyonlarýn 
	// baþarýyla yürütüldüðünü varsayarak kurtulamayýz ve her zaman hatalarý kontrol etmeli 
	// ve bir þeyler ters gittiðinde hata mesajlarýný görüntülememiz gerekir. asagidaki gibi kontrol yapmaliyiz
	//if (!color_buffer) {
	//	// if the return of malloc is a NULL pointer, the allocation was not succesfull
	//}
	//else {
	//	///
	//}
	//*******************************************************************************************************

	// Creating a SDL texture that used to display the color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	// Loads the cube values in the mesh data structure
	load_cube_mesh_data();



}

void process_input(void) {

	SDL_Event event;

	SDL_PollEvent(&event);

	switch (event.type) {
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			is_running = false;
		break;

	}


}

// Function that receives a 3D vector and returns a projected 2dpoint
vec2_t project(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov_factor * point.x)  / point.z, //perspective divided formula pX / pZ
		.y = (fov_factor * point.y)  / point.z  //perspective divided formula pY / pZ
	};
	return projected_point;
}

void update(void) {

	//Bu bekleme döngüsünün amacý, her bir kareyi eþit zaman aralýklarýyla güncellemektir. Bu sayede kare hýzýný kontrol eder ve programýn
	// sürekli ayný hýzda çalýþmasýný saðlar. Eðer kareler çok hýzlý güncellenirse (örneðin, hedef süreden daha kýsa sürede çalýþýrsa),
	// döngü aktif hale gelir ve bir sonraki kareyi çizmeye baþlamadan önce yeterli süre bekler. Bu yaklaþým, animasyonlarýn ve hareketlerin 
	// tutarlý görünmesini saðlar
	// ->> while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME)); 
	// while loops are processor instructions like %100 of cpu  bunun yerine SDL_Delay() kullandik asagida

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	// prev time 
	previous_frame_time = SDL_GetTicks();

	//Initialize the array of triangles to render
	triangles_to_render = NULL;

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.01;

	// for (int i = 0; i < N_POINTS; i++) {
	// 	vec3_t point = cube_points[i];
	// 	vec3_t  transformed_point = vec3_rotate_x(point, mesh.rotation.x);
	// 			transformed_point = vec3_rotate_y(transformed_point, mesh.rotation.y);
	// 			transformed_point = vec3_rotate_z(transformed_point, mesh.rotation.z);
	// 	// Translate the points away from the camera
	// 	transformed_point.z -= camera_position.z;
	// 	//project the current point
	// 	vec2_t projected_point = project(transformed_point);
	// 	// Save the projected 2D vector in the array of projected points
	// 	projected_points[i] = projected_point;
	// }

	

	int num_faces = array_length(mesh.faces);
	// Loop all triangle faces of our mesh
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		triangle_t projected_triangle;

		//loop all three vertices of this current face and aply transformations
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			//translate the vertex away from the camera in Z IMPORTANT
			transformed_vertex.z -= camera_position.z;

			// Project the current
			vec2_t projected_point = project(transformed_vertex);

			// Scale and translate the projected points to middle of the screen
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;
		}

		// Save the projected triangle in the array of triangles to render
		//triangles_to_render[i] = projected_triangle;
		// ----
		// new version
		array_push(triangles_to_render, projected_triangle);
	}

}


void render(void) {

	/*SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);*/
	// ....

	draw_grid();

	int num_triangles =	array_length(triangles_to_render);


	// Loop all projected triangles and render then
	for (int i = 0; i < num_triangles; i++) {
		triangle_t triangle = triangles_to_render[i];


		// Draw vertex points
		draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
		draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
		draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

		//Draw unfilled triangle
		draw_triangle(
			triangle.points[0].x,
			triangle.points[0].y,
			triangle.points[1].x,
			triangle.points[1].y,
			triangle.points[2].x,
			triangle.points[2].y,
			0xFF00FF00
		);

	}

	// draw_line(100, 200, 300, 50, 0xFF0000FF);
	//draw_pixel(20, 20, 0xFFFFFF00);
	////draw_rect();
	//draw_rect(300, 200, 300, 150, 0xFFFF0000);


	// !!!!!! Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);

	

	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);


}

/// <summary>
/// Free the memory that was dynamically allocated by the program
/// </summary>
void free_resources(void) {
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}


int main(int argc, char* args[]) {

	/* TODO: Create STL Window*/


	is_running = initialize_window();

	setup();



	while (is_running) {
		process_input();
		update();
		render();
	}


	


	destroy_window();
	
	//create the func
	free_resources();

	return 0;
}



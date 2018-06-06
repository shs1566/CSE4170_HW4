#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
#define SIMPLE_SHADER 0
#define PHONG_SHADER 1
#define GOURAUD_SHADER 2

int shader_selected = PHONG_SHADER;
GLuint h_ShaderProgram_simple, h_ShaderProgram_PS, h_ShaderProgram_GS;		// handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables
GLint loc_ModelViewProjectionMatrix_S, loc_ModelViewMatrix_S, loc_ModelViewMatrixInvTrans_S; // uniform variable for shader

float blind_ratio = 1.0f;
GLfloat loc_blind_ratio;
// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.

glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ModelViewMatrix, ViewMatrix, ProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

#include "Object_Definitions.h"

float WIN_HEIGHT;
float WIN_WIDTH;

void select_shader(int selected) {

	switch (selected) {
	case SIMPLE_SHADER:
		glUseProgram(h_ShaderProgram_simple);
		break;
	case PHONG_SHADER:
		glUseProgram(h_ShaderProgram_PS);
		break;
	case GOURAUD_SHADER:
		glUseProgram(h_ShaderProgram_GS);
		break;
	}
}
void set_up_scene_light(void) {

	glm::vec4 position_EC;
	glm::vec3 direction_EC;

	select_shader(shader_selected);

	glUniform4f(loc_global_ambient_color, 0.01f, 0.01f, 0.01f, 1.0f);

	glUniform1f(loc_blind_ratio, blind_ratio);

	light[0].position[0] = 0.0f; light[0].position[1] = 0.0f; light[0].position[2] = -10.0f; light[0].position[3] = 1.0f;	// EC
	light[0].ambient_color[0] = 0.0f; light[0].ambient_color[1] = 0.0f; light[0].ambient_color[2] = 0.0f; light[0].ambient_color[3] = 1.0f;
	light[0].diffuse_color[0] = 0.3f; light[0].diffuse_color[1] = 0.3f; light[0].diffuse_color[2] = 0.3f; light[0].diffuse_color[3] = 1.0f;
	light[0].specular_color[0] = 0.3f; light[0].specular_color[1] = 0.3f; light[0].specular_color[2] = 0.3f; light[0].specular_color[3] = 1.0f;
	light[0].spot_direction[0] = 0.0f; light[0].spot_direction[1] = 0.0f; light[0].spot_direction[2] = -1.0f;
	light[0].spot_exponent = 10.0f;
	light[0].spot_cutoff_angle = 180.0f;
	light[0].light_attenuation_factors[0] = 1.0f; light[0].light_attenuation_factors[1] = 0.0f; light[0].light_attenuation_factors[2] = 0.0f; light[0].light_attenuation_factors[3] = 1.0f;

	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);
	glUniform3fv(loc_light[0].spot_direction, 1, light[0].spot_direction);
	glUniform1f(loc_light[0].spot_cutoff_angle, light[0].spot_cutoff_angle);
	glUniform1f(loc_light[0].spot_exponent, light[0].spot_exponent);
	glUniform4fv(loc_light[0].light_attenuation_factors, 1, light[0].light_attenuation_factors);

	for (int i = 1; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		position_EC = ViewMatrix * glm::vec4(light[i].position[0], light[i].position[1], light[i].position[2], light[i].position[3]);
		direction_EC = glm::mat3(ViewMatrix) * glm::vec3(light[i].spot_direction[0], light[i].spot_direction[1], light[i].spot_direction[2]);

		glUniform1i(loc_light[i].light_on, light[i].light_on);
		glUniform4fv(loc_light[i].position, 1, &position_EC[0]);
		glUniform4fv(loc_light[i].ambient_color, 1, light[i].ambient_color);
		glUniform4fv(loc_light[i].diffuse_color, 1, light[i].diffuse_color);
		glUniform4fv(loc_light[i].specular_color, 1, light[i].specular_color);
		glUniform3fv(loc_light[i].spot_direction, 1, &direction_EC[0]);
		glUniform1f(loc_light[i].spot_cutoff_angle, light[i].spot_cutoff_angle);
		glUniform1f(loc_light[i].spot_exponent, light[i].spot_exponent);
		glUniform4fv(loc_light[i].light_attenuation_factors, 1, light[i].light_attenuation_factors);
	}

	glUseProgram(0);
}

void print_mat4(const char *string, glm::mat4 M) {
	fprintf(stdout, "\n***** %s ******\n", string);
	for (int i = 0; i < 4; i++)
		fprintf(stdout, "*** COL[%d] (%f, %f, %f, %f)\n", i, M[i].x, M[i].y, M[i].z, M[i].w);
	fprintf(stdout, "**************\n\n");
}

void set_ViewMatrix_from_camera_frame() {
	ViewMatrix[0] = glm::vec4(selected->uaxis.x, selected->vaxis.x, selected->naxis.x, 0.0f);
	ViewMatrix[1] = glm::vec4(selected->uaxis.y, selected->vaxis.y, selected->naxis.y, 0.0f);
	ViewMatrix[2] = glm::vec4(selected->uaxis.z, selected->vaxis.z, selected->naxis.z, 0.0f);
	ViewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, -(selected->pos));
}

bool draw_volume_flag = true;

void draw_camera(CAMERA camera) {

	float angle = camera.fov_y / 2.0f;

	glm::mat4 RotateMatrix;

	ModelViewMatrix = glm::translate(ViewMatrix, camera.pos);
	RotateMatrix[0] = glm::vec4(camera.uaxis.x, camera.uaxis.y, camera.uaxis.z, 0.0f);
	RotateMatrix[1] = glm::vec4(camera.vaxis.x, camera.vaxis.y, camera.vaxis.z, 0.0f);
	RotateMatrix[2] = glm::vec4(camera.naxis.x, camera.naxis.y, camera.naxis.z, 0.0f);
	ModelViewMatrix = ModelViewMatrix * RotateMatrix;

	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(CAM_AXIS_LENGTH, CAM_AXIS_LENGTH, CAM_AXIS_LENGTH));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	
	draw_axes();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CAMERA *temp = selected;

	for (int i = 0; i < 4; i++) {
		switch (i) {
		case 0:
			selected = &camera_down;
			set_ViewMatrix_from_camera_frame();
			ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
			glViewport(3.0f / 4.0f*WIN_WIDTH, i*WIN_HEIGHT / 3, WIN_WIDTH / 3, WIN_HEIGHT / 3);
			break;
		case 1:
			selected = &camera_front;
			set_ViewMatrix_from_camera_frame();
			ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
			glViewport(3.0f / 4.0f*WIN_WIDTH, i*WIN_HEIGHT / 3, WIN_WIDTH / 3, WIN_HEIGHT / 3);
			break;
		case 2:
			selected = &camera_side;
			set_ViewMatrix_from_camera_frame();
			ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
			glViewport(3.0f / 4.0f*WIN_WIDTH, i*WIN_HEIGHT / 3, WIN_WIDTH / 3, WIN_HEIGHT / 3);
			break;
		case 3:
			selected = temp;
			set_ViewMatrix_from_camera_frame();
			ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
			glViewport(0.0f, 0.0f, 3.0f / 4.0f * WIN_WIDTH, WIN_HEIGHT);
			break;
		}

		set_up_scene_light();

		/* Draw world coordinate */
		glUseProgram(h_ShaderProgram_simple);
		ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(2.0f);
		draw_axes();
		glLineWidth(1.0f);

		draw_camera(camera_world);

		if (draw_volume_flag) {
			define_volume();
			draw_volume();
		}

		draw_camera(camera_cctv1);
		draw_camera(camera_cctv2);
		draw_camera(camera_cctv3);

		draw_camera(camera_front);
		draw_camera(camera_down);
		draw_camera(camera_side);
		glUseProgram(0);

		select_shader(shader_selected);
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(car_pos[car_frame].x, car_pos[car_frame].y, car_pos[car_frame].z));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, car_pos[car_frame].w * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_S, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_car_dummy();

		draw_static_object(&(static_objects[OBJ_BUILDING]), 0);

		draw_static_object(&(static_objects[OBJ_TABLE]), 0);
		draw_static_object(&(static_objects[OBJ_TABLE]), 1);
		draw_static_object(&(static_objects[OBJ_TABLE]), 2);

		draw_static_object(&(static_objects[OBJ_LIGHT]), 0);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 1);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 2);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 3);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 4);

		draw_static_object(&(static_objects[OBJ_TEAPOT]), 0);
		draw_static_object(&(static_objects[OBJ_TEAPOT]), 1);
		draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0);
		draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 1);
		draw_static_object(&(static_objects[OBJ_FRAME]), 0);
		draw_static_object(&(static_objects[OBJ_FRAME]), 1);
		draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0);
		draw_static_object(&(static_objects[OBJ_COW]), 0);
		draw_static_object(&(static_objects[OBJ_COW]), 1);

		draw_animated_tiger();
		draw_camera(camera_tiger);
		glUseProgram(0);
	}
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0, polygon_fill_on = 0, depth_test_on = 0;
	glm::mat3 RotationMatrix;


	if (key >= '0' && key <= '0' + NUMBER_OF_LIGHT_SUPPORTED - 1) {
		int light_ID = (int)(key - '0');
		light[light_ID].light_on = 1 - light[light_ID].light_on;
		glUniform1i(loc_light[light_ID].light_on, light[light_ID].light_on);
		fprintf(stdout, "%d light %s\n", light_ID, (light[light_ID].light_on) ? "On" : "Off");
		glutPostRedisplay();
	}

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;
		}
		break;
	case 'f':
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;
	case 'd':
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;

		/* Camera Rotation */
	case 'q': /* camera rotate +vaxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED*TO_RADIAN, selected->vaxis));
		selected->uaxis = RotationMatrix * selected->uaxis;
		selected->naxis = RotationMatrix * selected->naxis;
		glutPostRedisplay();
		break;
	case 'w': /* camera rotate -vaxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), -CAM_ROTATION_SPEED*TO_RADIAN, selected->vaxis));
		selected->uaxis = RotationMatrix * selected->uaxis;
		selected->naxis = RotationMatrix * selected->naxis;
		glutPostRedisplay();
		break;
	case 'a': /* camera rotate +uaxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED*TO_RADIAN, selected->uaxis));
		selected->vaxis = RotationMatrix * selected->vaxis;
		selected->naxis = RotationMatrix * selected->naxis;
		glutPostRedisplay();
		break;
	case 's': /* camera rotate -uaxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), -CAM_ROTATION_SPEED*TO_RADIAN, selected->uaxis));
		selected->vaxis = RotationMatrix * selected->vaxis;
		selected->naxis = RotationMatrix * selected->naxis;
		glutPostRedisplay();
		break;
	case 'z': /* camera rotate +naxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED*TO_RADIAN, selected->naxis));
		selected->vaxis = RotationMatrix * selected->vaxis;
		selected->uaxis = RotationMatrix * selected->uaxis;
		glutPostRedisplay();
		break;
	case 'x': /* camera rotate -naxis */
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), -CAM_ROTATION_SPEED*TO_RADIAN, selected->naxis));
		selected->vaxis = RotationMatrix * selected->vaxis;
		selected->uaxis = RotationMatrix * selected->uaxis;
		glutPostRedisplay();
		break;

		/* Camera zoom-in, zoom-out */
	case 'v': /* camera zoom-out */
		selected->fov_y += CAM_ZOOM_SPEED;
		glutPostRedisplay();
		break;
	case 'b': /* camera zoom-in */
		selected->fov_y -= CAM_ZOOM_SPEED;
		glutPostRedisplay();
		break;

		/* Camera change */
	case 'p': /* change to cctv1 */
		selected = &camera_cctv1;
		glutPostRedisplay();
		break;
	case 'o': /* change to cctv2 */
		selected = &camera_cctv2;
		glutPostRedisplay();
		break;
	case 'i': /* change to cctv3 */
		selected = &camera_cctv3;
		glutPostRedisplay();
		break;
	case 'u': /* change to world */
		selected = &camera_world;
		glutPostRedisplay();
		break;
	case 'l': /* change to down view */
		selected = &camera_down;
		glutPostRedisplay();
		break;
	case 'k': /* change to front view */
		selected = &camera_front;
		glutPostRedisplay();
		break;
	case 'j': /* change to side view */
		selected = &camera_side;
		glutPostRedisplay();
		break;
	case 't':	/* change to tiger view */
		selected = &camera_tiger;
		glutPostRedisplay();
		break;

		/* toogle view volume */
	case 'y':
		draw_volume_flag = (draw_volume_flag) ? false : true;
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	WIN_HEIGHT = height;
	WIN_WIDTH = width;
	glViewport(0, 0, width, height);
	
	camera_world.aspect_ratio = (float)width / height;
	
	set_ViewMatrix_from_camera_frame();
	ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
	ModelViewMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	tiger_data.rotation_angle = (timestamp_scene % 360)*TO_RADIAN;
	if (tiger_data.direction) {
		tiger_data.pos++;
		if (tiger_data.pos>N_TIGER_POS_FRAME)
			tiger_data.direction = false;
	}
	else {
		tiger_data.pos--;
		if (tiger_data.pos < 0)
			tiger_data.direction = true;
	}

	car_frame++;
	if (car_frame>=N_CAR_FRAME)
		car_frame = 0;
	wheel_rotate -= 5.0f;

	glutPostRedisplay();
	glutTimerFunc(50, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void special(int key, int x, int y) {

	switch (key) {
	
	case GLUT_KEY_LEFT:	/* camera move -uaxis */
		camera_world.pos -= CAM_TRANSLATION_SPEED * camera_world.uaxis;
		glutPostRedisplay();
		break;
		
	case GLUT_KEY_RIGHT: /* camera move +uaxis */
		camera_world.pos += CAM_TRANSLATION_SPEED * camera_world.uaxis;
		glutPostRedisplay();
		break;
		
	case GLUT_KEY_UP: /* camera move +vaxis */
		camera_world.pos += CAM_TRANSLATION_SPEED * camera_world.vaxis;
		glutPostRedisplay();
		break;

	case GLUT_KEY_DOWN: /* camera move -vaxis */
		camera_world.pos -= CAM_TRANSLATION_SPEED * camera_world.vaxis;
		glutPostRedisplay();
		break;

	case GLUT_KEY_PAGE_DOWN: /* camera move +naxis */
		camera_world.pos += CAM_TRANSLATION_SPEED * camera_world.naxis;
		glutPostRedisplay();
		break;

	case GLUT_KEY_PAGE_UP: /* camera move -naxis */
		camera_world.pos -= CAM_TRANSLATION_SPEED * camera_world.naxis;
		glutPostRedisplay();
		break;

	case GLUT_KEY_F1:	/* toogle blind effect */
		blind_ratio = (blind_ratio > 0.0f) ? 0.0f : 1.0f;
		glUniform1f(loc_blind_ratio, blind_ratio);
		fprintf(stdout, "blind effect : %s\n", (blind_ratio > 0.0f) ? "On" : "Off");
		glutPostRedisplay();
		break;

	case GLUT_KEY_F2:	/* increase blind effect */
		blind_ratio += 0.1f;
		glUniform1f(loc_blind_ratio, blind_ratio);
		glutPostRedisplay();
		break;

	case GLUT_KEY_F3:	/* decrease blind effect */
		blind_ratio -= 0.1f;
		glUniform1f(loc_blind_ratio, blind_ratio);
		glutPostRedisplay();
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON) {
		int mod = glutGetModifiers();
		if (state == GLUT_DOWN && mod == GLUT_ACTIVE_SHIFT) {
				shader_selected = GOURAUD_SHADER;
		}
		else
			shader_selected = PHONG_SHADER;
	}
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup_OpenGL_stuffs);

	glutSpecialFunc(special);
	glutMouseFunc(mouse);
}

void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong.frag" },
		{ GL_NONE, NULL }
	};
	ShaderInfo shader_info_GS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};
	h_ShaderProgram_simple = LoadShaders(shader_info);
	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_S = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_S = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_S = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrixInvTrans");
	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
	
	h_ShaderProgram_GS = LoadShaders(shader_info_GS);
	loc_ModelViewProjectionMatrix_S = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_S = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_S = glGetUniformLocation(h_ShaderProgram_GS, "u_ModelViewMatrixInvTrans");
	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");

	loc_blind_ratio = glGetUniformLocation(h_ShaderProgram_PS, "u_ratio");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.ambient_color");

	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.diffuse_color");

	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_color");

	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_GS, "u_material.emissive_color");

	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_GS, "u_material.specular_exponent");
}

void initialize_camera(void) {

	/* World camera */
	glm::mat4 Matrix = glm::lookAt(glm::vec3(120.0f, 90.0f, 120.0f), glm::vec3(120.0f, 90.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	camera_world.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_world.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_world.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_world.pos = -(Matrix[3].x*camera_world.uaxis + Matrix[3].y*camera_world.vaxis + Matrix[3].z*camera_world.naxis);

	camera_world.move_status = 0;
	camera_world.fov_y = 100.0f;
	camera_world.aspect_ratio = 1.5f;
	camera_world.near_clip = 3.0f;
	camera_world.far_clip = 2000.0f;

	/* cctv1 */
	Matrix = glm::lookAt(glm::vec3(190.0f, 120.0f, 20.0f), glm::vec3(200.0f, 120.0f, 19.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera_cctv1.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_cctv1.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_cctv1.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_cctv1.pos = -(Matrix[3].x*camera_cctv1.uaxis + Matrix[3].y*camera_cctv1.vaxis + Matrix[3].z*camera_cctv1.naxis);

	camera_cctv1.move_status = 0;
	camera_cctv1.fov_y = 90.0f;
	camera_cctv1.aspect_ratio = 1.5f;
	camera_cctv1.near_clip = 0.1f;
	camera_cctv1.far_clip = 100.0f;

	/* cctv2 */
	Matrix = glm::lookAt(glm::vec3(155.0f, 75.0f, 10.0f), glm::vec3(0.0f, 75.0f, 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera_cctv2.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_cctv2.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_cctv2.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_cctv2.pos = -(Matrix[3].x*camera_cctv2.uaxis + Matrix[3].y*camera_cctv2.vaxis + Matrix[3].z*camera_cctv2.naxis);

	camera_cctv2.move_status = 0;
	camera_cctv2.fov_y = 90.0f;
	camera_cctv2.aspect_ratio = 1.5f;
	camera_cctv2.near_clip = 0.1f;
	camera_cctv2.far_clip = 100.0f;

	/* cctv3 */
	Matrix = glm::lookAt(glm::vec3(200.0f, 100.0f, 50.0f), glm::vec3(200.0f, 100.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f));
	camera_cctv3.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_cctv3.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_cctv3.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_cctv3.pos = -(Matrix[3].x*camera_cctv3.uaxis + Matrix[3].y*camera_cctv3.vaxis + Matrix[3].z*camera_cctv3.naxis);

	camera_cctv3.move_status = 0;
	camera_cctv3.fov_y = 90.0f;
	camera_cctv3.aspect_ratio = 1.5f;
	camera_cctv3.near_clip = 10.0f;
	camera_cctv3.far_clip = 1000.0f;

	/* Down camera */
	Matrix = glm::lookAt(glm::vec3(180.0f, 120.0f, 1200.0f), glm::vec3(180.0f, 120.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f));
	camera_down.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_down.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_down.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_down.pos = -(Matrix[3].x*camera_down.uaxis + Matrix[3].y*camera_down.vaxis + Matrix[3].z*camera_down.naxis);
				   
	camera_down.move_status = 0;
	camera_down.fov_y = 15.0f;
	camera_down.aspect_ratio = 1.5f;
	camera_down.near_clip = 1.0f;
	camera_down.far_clip = 100000.0f;

	/* Front camera */
	Matrix = glm::lookAt(glm::vec3(1200.0f, 120.0f, 25.0f), glm::vec3(0.0f, 120.0f, 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera_front.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_front.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_front.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_front.pos = -(Matrix[3].x*camera_front.uaxis + Matrix[3].y*camera_front.vaxis + Matrix[3].z*camera_front.naxis);
				   
	camera_front.move_status = 0;
	camera_front.fov_y = 15.0f;
	camera_front.aspect_ratio = 1.5f;
	camera_front.near_clip = 1.0f;
	camera_front.far_clip = 100000.0f;

	/* Side camera */
	Matrix = glm::lookAt(glm::vec3(100.0f, 1200.0f, 25.0f), glm::vec3(100.0f, 0.0f, 25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera_side.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_side.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_side.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_side.pos = -(Matrix[3].x*camera_side.uaxis + Matrix[3].y*camera_side.vaxis + Matrix[3].z*camera_side.naxis);

	camera_side.move_status = 0;
	camera_side.fov_y = 15.0f;
	camera_side.aspect_ratio = 1.5f;
	camera_side.near_clip = 1.0f;
	camera_side.far_clip = 100000.0f;

	/* Tiger camera */
	Matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(-1.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 1.0f));
	camera_tiger.uaxis = glm::vec3(Matrix[0].x, Matrix[1].x, Matrix[2].x);
	camera_tiger.vaxis = glm::vec3(Matrix[0].y, Matrix[1].y, Matrix[2].y);
	camera_tiger.naxis = glm::vec3(Matrix[0].z, Matrix[1].z, Matrix[2].z);
	camera_tiger.pos = -(Matrix[3].x*camera_tiger.uaxis + Matrix[3].y*camera_tiger.vaxis + Matrix[3].z*camera_tiger.naxis);
				  
	camera_tiger.move_status = 0;
	camera_tiger.fov_y = 80.0f;
	camera_tiger.aspect_ratio = 1.5f;
	camera_tiger.near_clip = 5.0f;
	camera_tiger.far_clip = 100000.0f;

	set_ViewMatrix_from_camera_frame();
}
 
void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST); // Default state
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	selected = &camera_world;
	initialize_camera();

	select_shader(shader_selected);
	ProjectionMatrix = glm::perspective(selected->fov_y*TO_RADIAN, selected->aspect_ratio, selected->near_clip, selected->far_clip);
	ModelViewMatrix = ProjectionMatrix * ViewMatrix;
}

void prepare_scene(void) {
	define_axes();
	define_static_objects();
	define_animated_tiger();
	define_volume();
	prepare_geom_obj2(GEOM_OBJ_ID_CAR_BODY, "Data/car_body_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj2(GEOM_OBJ_ID_CAR_WHEEL, "Data/car_wheel_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj2(GEOM_OBJ_ID_CAR_NUT, "Data/car_nut_triangles_v.txt", GEOM_OBJ_TYPE_V);
	initialize_car_pos();

	set_up_scene_light();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}



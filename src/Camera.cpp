#include "Camera.h"

#include "MatrixStack.h"
#include <glad/glad.h>
#include "GLSL.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

#include <vector>

	
Camera::Camera() {
	w = 0;
	a = 0;
	s = 0;
	d = 0;
	up = 0;
	down = 0;
	bird = 1;
	pos = glm::vec3(0, 0, 0);
	rot = glm::vec3(0, 0, 0);
	bird_pos = glm::vec3(0, -30, -25);
	bird_rot = glm::vec3(3.1515926535f / 4.f, 0, 0);
}


glm::mat4 Camera::process(double ftime, glm::vec3 position) {

	float xangle = 0;
	float yangle = 0;

	if (a == 1)
		yangle = -3 * ftime;
	else if (d == 1)
		yangle = 3 * ftime;

	if (w == 1)
		xangle = -.5 * ftime;
	else if (s == 1)
		xangle = .5 * ftime;


	if (bird == 1) {
		bird_rot.x += xangle;
		bird_rot.y += yangle;

		glm::mat4 R = glm::rotate(glm::mat4(1), bird_rot.x, glm::vec3(1, 0, 0)); // tilt up and down
		glm::mat4 T = glm::translate(glm::mat4(1), bird_pos);
		T *= glm::rotate(glm::mat4(1), bird_rot.y, glm::vec3(0, 1, 0)); // pan around

		return R * T;
	}
	else {
		rot.y += yangle;
		rot.x += xangle * 2;

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		R *= glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));

		glm::mat4 T = glm::translate(glm::mat4(1), position);
		return R * T;
	};
	
}

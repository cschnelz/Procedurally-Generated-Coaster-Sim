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
	bird = 0;
	pos = glm::vec3(0, 0, 0);
	rot = glm::vec3(0, 0, 0);
}


glm::mat4 Camera::process(double ftime, std::vector<glm::vec3> positions, std::vector<float> rotations, std::vector<int> directions,
	std::vector<double> heightmap, std::vector<float> slopes, std::vector<glm::vec3> dirs) {
	float speed = 0;
	float flight = 0;
	if (up == 1) {
		flight = 10 * ftime;
	}
	else if (down == 1) {
		flight = -10 * ftime;
	}
	if (w == 1) {
		speed = 10 * ftime;
	}
	else if (s == 1) {
		speed = -10 * ftime;
	}
	float yangle = 0;
	if (a == 1)
		yangle = -3 * ftime;
	else if (d == 1)
		yangle = 3 * ftime;

	rot.y += yangle;
	glm::mat4 R;

	if (bird == 1) {
		R = glm::rotate(glm::mat4(1), 3.1515926535f / 4.f, glm::vec3(1, 0, 0));
	}
	else {
		R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		R *= glm::rotate(glm::mat4(1), 0.f, glm::vec3(1, 0, 0));
	}
	glm::vec4 dir = glm::vec4(0, -flight, speed, 1);
	dir = dir * R;
	pos += glm::vec3(dir.x, dir.y, dir.z);
	glm::mat4 T = glm::translate(glm::mat4(1), pos);
	return R * T;
}

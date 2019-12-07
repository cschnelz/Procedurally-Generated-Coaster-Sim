#pragma once

#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include "GLSL.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {

public:
	glm::vec3 pos = glm::vec3(0,0,0);
	glm::vec3 bird_pos = glm::vec3(0,0,0);
	glm::vec3 rot = glm::vec3(0,0,0);
	glm::vec3 bird_rot = glm::vec3(0,0,0);

	int w = 0;
	int a = 0;
	int s = 0;
	int d = 0;
	int up = 0;
	int down = 0;
	int bird = 0;
	
	Camera();
		
	glm::mat4 process(double ftime, glm::vec3 position);
};

#endif
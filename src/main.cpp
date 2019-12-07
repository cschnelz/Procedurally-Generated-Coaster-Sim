#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"

#include "time.h"
#include <algorithm>

#include "PerlinNoise.h"
#include "Camera.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;


double get_last_elapsed_time() {
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

Camera mycam;

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, heightshader, trackProg;

	// Contains vertex information for OpenGL

	// cube gluints
	GLuint VertexArrayID;
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDCube;

	// cubes
	GLuint CubeArrayID;
	GLuint CubeBufferID, CubeNorID, CubeIndexID;

	// Data necessary to give our box to OpenGL
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2,HeightTex;

	// cylinder dimensions
	int CIRC_SAMP_RATE = 80;
	int cylinderLength = 4;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GL_TRUE); }
		if (key == GLFW_KEY_W && action == GLFW_PRESS) { mycam.w = 1; }
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) { mycam.w = 0; }
		if (key == GLFW_KEY_S && action == GLFW_PRESS) { mycam.s = 1; }
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) { mycam.s = 0; }
		if (key == GLFW_KEY_A && action == GLFW_PRESS) { mycam.a = 1; }
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) { mycam.a = 0; }
		if (key == GLFW_KEY_D && action == GLFW_PRESS) { mycam.d = 1; }
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) { mycam.d = 0; }
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { mycam.up = 1; }
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { mycam.up = 0; }
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) { mycam.down = 1; }
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) { mycam.down = 0; }
		if (key == GLFW_KEY_G && action == GLFW_PRESS) { mycam.bird = 1; }
		if (key == GLFW_KEY_H && action == GLFW_PRESS) { mycam.bird = 0; }
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
		void;
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) {
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	#define MESHSIZE 100
	void init_mesh() {
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &MeshPosID);
		glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
		vec3 vertices[MESHSIZE * MESHSIZE * 4];
		for(int x=0;x<MESHSIZE;x++)
			for (int z = 0; z < MESHSIZE; z++) {
				vertices[x * 4 + z*MESHSIZE * 4 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 3] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);
				}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 4, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//tex coords
		float t = 1. / 100;
		vec2 tex[MESHSIZE * MESHSIZE * 4];
		for (int x = 0; x<MESHSIZE; x++)
			for (int y = 0; y < MESHSIZE; y++) {
				tex[x * 4 + y*MESHSIZE * 4 + 0] = vec2(0.0, 0.0)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 1] = vec2(t, 0.0)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 2] = vec2(t, t)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 3] = vec2(0.0, t)+ vec2(x, y)*t;
			}
		glGenBuffers(1, &MeshTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort elements[MESHSIZE * MESHSIZE * 6];
		int ind = 0;
		for (int i = 0; i<MESHSIZE * MESHSIZE * 6; i+=6, ind+=4) {
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 2;
			elements[i + 3] = ind + 0;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 3;
		}			
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*MESHSIZE * MESHSIZE * 6, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}


	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom() {
		//generate the VAO
		glGenVertexArrays(1, &CubeArrayID);
		glBindVertexArray(CubeArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &CubeBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, CubeBufferID);

		GLfloat cube_vertices2[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
			//tube 8 - 11
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			//12 - 15
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0


		};

		// scale down
		for (int i = 0; i < 48; i++) {
			cube_vertices2[i] = cube_vertices2[i] * .8;
		}
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices2), cube_vertices2, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &CubeIndexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeIndexID);
		GLushort cube_elements2[] = {

			// front
			0, 1, 2,
			2, 3, 0,
			// back
			7, 6, 5,
			5, 4, 7,
			//tube 8-11, 12-15
			8,12,13,
			8,13,9,
			9,13,14,
			9,14,10,
			10,14,15,
			10,15,11,
			11,15,12,
			11,12,8

		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements2), cube_elements2, GL_STATIC_DRAW);

		glGenBuffers(1, &CubeNorID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, CubeNorID);

		//normals
		GLfloat cube_nor[] = {
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,-1,
			0,0,-1,
			0,0,-1,
			0,0,-1,
			0,1,0,
			0,1,0,
			1,0,0,
			1,0,0,
			0,-1,0,
			0,-1,0,
			-1,0,0,
			-1,0,0
		};
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_nor), cube_nor, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);

		//initialize the net mesh
		init_mesh();

		string resourceDirectory = "../resources" ;			   		 	  	  
		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 3
		str = resourceDirectory + "/height.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &HeightTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		//GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		//GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		//// Then bind the uniform samplers to texture units:
		//glUseProgram(prog->pid);
		//glUniform1i(Tex1Location, 0);
		//glUniform1i(Tex2Location, 1);

		GLuint Tex1Location = glGetUniformLocation(heightshader->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(heightshader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(heightshader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		//prog = std::make_shared<Program>();
		//prog->setVerbose(true);
		//prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		//if (!prog->init()) {
		//	std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
		//	exit(1);
		//}
		//prog->addUniform("P");
		//prog->addUniform("V");
		//prog->addUniform("M");
		//prog->addUniform("campos");
		//prog->addUniform("color_change");
		//prog->addAttribute("vertPos");
		//prog->addAttribute("vertNor");
		//prog->addAttribute("vertTex");

		trackProg = std::make_shared<Program>();
		trackProg->setVerbose(true);
		trackProg->setShaderNames(resourceDirectory + "/track_vertex.glsl", resourceDirectory + "/track_fragment.glsl");
		if (!trackProg->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		trackProg->addUniform("P");
		trackProg->addUniform("V");
		trackProg->addUniform("M");
		trackProg->addUniform("color_change");
		trackProg->addAttribute("vertPos");
		trackProg->addAttribute("vertNorTrack");

		// Initialize the GLSL program.
		heightshader = std::make_shared<Program>();
		heightshader->setVerbose(true);
		heightshader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl");
		if (!heightshader->init()) {
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightshader->addUniform("P");
		heightshader->addUniform("V");
		heightshader->addUniform("M");
		heightshader->addUniform("camoff");
		heightshader->addUniform("campos");
		heightshader->addAttribute("vertPos");
		heightshader->addAttribute("vertTex");
	}

	//
	// 
	//
	// now the procedural generation part (read bottom to top)
	//
	//
	//

	std::vector<glm::vec3> positions;
	std::vector<float> rotations;
	std::vector<int> directions;
	float quarter = pi<float>() / 2;

	void autocomplete_perpendicular(int starting_dir, int current_dir, vec3 end_coords, vec3 starting_coords) {
		vec3 temp = end_coords;
		if (starting_dir == 0 || starting_dir == 2) {
			if (abs(end_coords.y - starting_coords.y) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);
			}
			for (int i = 0; i < abs(end_coords.y - starting_coords.y); i++) {
				temp.y += (end_coords.y > starting_coords.y) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter * starting_dir);
				directions.push_back( (end_coords.y > starting_coords.y) ? 0 : 2);
			}
			if (abs(end_coords.x - starting_coords.x) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);

			}
			for (int i = 0; i < abs(end_coords.x - starting_coords.x); i++) {
				temp.x += (end_coords.x > starting_coords.x) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter * ((starting_dir + 1) % 4));
				directions.push_back((end_coords.x > starting_coords.x) ? 1 : 3);
			}
		}
		else {
			if (abs(end_coords.x - starting_coords.x) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);

			}
			for (int i = 0; i < abs(end_coords.x - starting_coords.x); i++) {
				temp.x += (end_coords.x > starting_coords.x) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter * starting_dir);
				directions.push_back((end_coords.x > starting_coords.x) ? 1 : 3);
			}
			if (abs(end_coords.y - starting_coords.y) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);
			}
			for (int i = 0; i < abs(end_coords.y - starting_coords.y); i++) {
				temp.y += (end_coords.y > starting_coords.y) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter * ((starting_dir + 1) % 4));
				directions.push_back((end_coords.y > starting_coords.y) ? 0 : 2);
			}
		}
	}

	void autocomplete_parallel(int starting_dir, int current_dir, vec3 end_coords, vec3 starting_coords) {
		vec3 intermediate = vec3(0);
		int padding_dir = 0;
		vec3 temp = vec3(0);
		if (starting_dir == 0 || starting_dir == 2) {
			intermediate.x = (end_coords.x > 0) ? 2 : -2;
			padding_dir = (end_coords.x > 0) ? 1 : 3;
			autocomplete_perpendicular(padding_dir, current_dir, end_coords, intermediate);
			temp = intermediate;
			if (abs(intermediate.x) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);
			}
			for (int i = 0; i < abs(intermediate.x); i++) {
				temp.x += (intermediate.x > 0) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter);
				directions.push_back((intermediate.x > 0) ? 0 : 2);
			}
		}
		else {
			intermediate.y = (end_coords.y > 0) ? 2 : -2;
			padding_dir = (end_coords.y > 0) ? 0 : 2;
			autocomplete_perpendicular(padding_dir, current_dir, end_coords, intermediate);
			temp = intermediate;
			if (abs(intermediate.y) > 1) {
				positions.erase(positions.end() - 1);
				rotations.erase(rotations.end() - 1);
				directions.erase(directions.end() - 1);
			}
			for (int i = 0; i < abs(intermediate.y); i++) {
				temp.y += (intermediate.y > 0) ? -1 : 1;
				positions.push_back(temp);
				rotations.push_back(quarter * 2.f);
				directions.push_back((intermediate.y > 0) ? 1 : 3);
			}
		}
	}
	
	void autocomplete(int starting_dir, int current_dir, glm::vec3 end_coords) {
		glm::vec3 starting_coords = positions[0];
		int angle = abs(current_dir - starting_dir);
		if (angle == 0 || angle == 2) {
			autocomplete_parallel(starting_dir, current_dir, end_coords, starting_coords);
		}
		else {
			autocomplete_perpendicular(starting_dir, current_dir, end_coords, starting_coords);
		}
	}


	glm::vec3 near_border(glm::vec3 coords, int dimensions, int *dir, int *straight) {
		if ((*dir == 0 || *dir == 2) && dimensions - abs(coords.y) < 3) {
			return generate_turn(coords, dir, dimensions, straight);
		}
		else if ((*dir == 1 || *dir == 3) && dimensions - abs(coords.x) < 3) {
			return generate_turn(coords, dir, dimensions, straight);
		}
		return coords;
	}

	vec3 shift_coords(vec3 coords, int dir) {
		if (dir == 0) {
			coords.y += 1;
		}
		else if (dir == 1) {
			coords.x += 1;
		}
		else if (dir == 2) {
			coords.y -= 1;
		}
		else {
			coords.x -= 1;
		}
		return coords;
	}

	vec3 generate_turn(vec3 coords, int *dir, int dimensions, int *straight) {
		int turn_length = floor(sqrt(dimensions));
		
		//int turn_dir = rand() % 2; // 0 or 1
		int old_dir = *dir;

		// turn buffer (dont push to position vector)
		coords = shift_coords(coords, *dir);

		// turn towards further border
		if (old_dir == 0 || old_dir == 2) {
			*dir = (coords.x > 0) ? 3 : 1;
		}
		else if (old_dir == 1 || old_dir == 3) {
			*dir = (coords.y > 0) ? 2 : 0;
		}

		for (int i = 0; i < turn_length; i++) {
			coords = shift_coords(coords, *dir);
			positions.push_back(coords);
			rotations.push_back(quarter * *dir);
			directions.push_back(*dir);
		}

		// turn buffer
		coords = shift_coords(coords, *dir);
		// turn around
		*dir = (old_dir + 2) % 4;
		// move back into the border zone
		if (*dir == 1 || *dir == 3) {
			for (int i = 0; i < abs(coords.x) - (dimensions - 4); i++) {
				coords = shift_coords(coords, *dir);
				positions.push_back(coords);
				rotations.push_back(quarter * *dir);
				directions.push_back(*dir);
			}
		}
		else {
			for (int i = 0; i < abs(coords.y) - (dimensions - 4); i++) {
				coords = shift_coords(coords, *dir);
				positions.push_back(coords);
				rotations.push_back(quarter * *dir);
				directions.push_back(*dir);
			}
		}
		// set the straight factor to max to promote leaving the border
		*straight = 130;
		return coords;
	}

	glm::vec3 generate_straightaway(vec3 coords, int min_segment_length, int dimensions, int* dir, int* straight) {
		for (int i = 0; i < min_segment_length; i++) {
			coords = shift_coords(coords, *dir);
			positions.push_back(coords);
			rotations.push_back(quarter * *dir);
			directions.push_back(*dir);
			coords = near_border(coords, dimensions, dir, straight);
		}
		return coords;
	}

	// length = distance path to generate
	// straightrate = 0 - 100 odds of just continuing current dir
	// The code to create the path is a modified and bounded variant of the "drunken walk" algorithm that also includes an autocomplete feature to ensure complete loops
	void createPath(int length, int straight_rate, int straight_deterioration, int border_size, int min_segment_length) {
		vec3 coords = vec3(0);
		positions.push_back(coords);
		// starting direction
		int dir = rand() % 4; // 0 = N; 1 = E; 2 = S; 3 = W;
		rotations.push_back(quarter * dir);
		directions.push_back(dir);
		// save the starting direction for autocomplete
		int start_dir = dir;
		// given percentage for turning, deteriorates over time until a turn is made
		int straight = straight_rate;

		coords = generate_straightaway(coords, min_segment_length, border_size / 2, &dir, &straight);
		for (int i = 0; i < length; i++) {
			int checkFeature = rand() % 100;
			int checkstraight = rand() % 100;
			
			if (checkstraight < straight) {
				// continue going straight, decay straight percentage
				straight -= straight_deterioration;
			}
			else {
				// reset straight percentage
				straight = straight_rate;
				int turn = rand() % 4;
					
				if ((turn + 2) % 4 == dir) {
					// reject 180 degree turns
					turn = (turn + (rand() % 2 * 2 + 1)) % 4;
				}
				dir = turn;
				// after turning, create a straightaway
				coords = generate_straightaway(coords, min_segment_length, border_size / 2, &dir, &straight);
			}
	
			// move one unit in our current direction
			coords = shift_coords(coords, dir);
			positions.push_back(coords);
			rotations.push_back(quarter * dir);
			directions.push_back(dir);

			// correct if the path is approaching the border
			coords = near_border(coords, border_size / 2, &dir, &straight);
		}
		// connect the end to the start
		autocomplete(start_dir, dir, coords);
		positions.erase(positions.end() - 1);
		rotations.erase(rotations.end() - 1);
		directions.erase(directions.end() - 1);
		positions.erase(positions.begin());
		rotations.erase(rotations.begin());
		directions.erase(directions.begin());

		printf("diff in vectors: %i\n", positions.size() - directions.size());
	}

	std::vector<double> heightmap;
	std::vector<float> slopes;
	std::vector<glm::vec3> dirs;

	void createHeights() {
		PerlinNoise pn;
		float y_val = (float)(rand() % 100) / 100.f;	
		float z_val = (float)(rand() % 100) / 100.f;	
		for (int i = 0; i < positions.size(); i++) {
			double noise = pn.noise((double)i * 0.1, y_val, z_val);
			noise *= 10;
			heightmap.push_back(noise);
		}
	}

	// takes in a distance from each end to smooth on
	void smoothHeights(int buffer) {
		double midpoint = (heightmap[0] + heightmap[heightmap.size() - 1]) / 2;
		double smoothness = 1.0 / buffer;

		for (int i = 0; i < buffer; i++)
			heightmap[i] += ((midpoint - heightmap[i]) * (1.0 - i * smoothness));
		for (int i = 0; i < buffer; i++)
			heightmap[heightmap.size() - 1 - i] += ((midpoint - heightmap[heightmap.size() - 1 - i]) * (1.0 - i * smoothness));
	}

	void calculateSlopes() {
		float curr_height, next_height;
		for (int i = 0; i < heightmap.size() - 1; i++) {
			curr_height = heightmap[i];
			next_height = heightmap[i + 1];

			slopes.push_back(glm::atan(next_height - curr_height));
		}
		curr_height = heightmap[heightmap.size() - 1];
		next_height = heightmap[0];
		slopes.push_back(glm::atan(next_height - curr_height));

		printf("diff in vecs 2: %i\n", positions.size() - slopes.size());
	}

	void vectorizeDirections() {
		for (int dir : directions) {
			if (dir == 0)
				dirs.push_back(glm::vec3(-1, 0, 0));
			else if (dir == 2)
				dirs.push_back(glm::vec3(1, 0, 0));
			else if (dir == 1)
				dirs.push_back(glm::vec3(0, 0, 1));
			else
				dirs.push_back(glm::vec3(0, 0, -1));
		}
	}

	void vectorLengths() {
		printf("pos: %i\n", positions.size());
		printf("rot: %i\n", rotations.size());
		printf("dir: %i\n", directions.size());
		printf("dirs: %i\n", dirs.size());
		printf("height: %i\n", heightmap.size());
	}

	glm::mat4 tracePath(int steps_per_position) {
		// index along the overall position vector
		static int position_location = 0;
		// stepper for making smaller steps between each index in the position vector
		static int position_step = 0;

		// calculate position x and z values from position vector
		glm::vec3 curr_pos = positions[position_location];
		glm::vec3 next_pos = positions[(position_location + 1) % positions.size()];

		glm::vec3 diff = next_pos - curr_pos;
		glm::vec3 step_size = diff / (float)steps_per_position;
		glm::vec3 step_location = curr_pos + (step_size * (float)position_step);

		// calculate position y value from heightmap
		double curr_height = heightmap[position_location];
		double next_height = heightmap[(position_location + 1) % positions.size()];

		double height_diff = next_height - curr_height;
		double height_size = height_diff / (double)steps_per_position;
		double height_loc = curr_height + (height_size * (double)position_step);


		position_step = (position_step + 1) % steps_per_position; // increment intra position stepper
		if (position_step == 0) {
			position_location = (position_location + 1) % positions.size(); // move to next position index
			step_location = next_pos;
			height_loc = next_height;
		}

		//return glm::translate(glm::mat4(1.0f), glm::vec3(
		//	positions[(int)(glfwGetTime() * 5) % (positions.size() - 1)].x,
		//	heightmap[(int)(glfwGetTime() * 5) % (positions.size() - 1)] + 3.5, 
		//	positions[(int)(glfwGetTime() * 5) % (positions.size() - 1)].y));
		return glm::translate(glm::mat4(1.0f), glm::vec3(step_location.x, height_loc, step_location.y));
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render() {
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height) {
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926/2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M =  TransZ * S;
		V = mycam.process(frametime, positions, rotations, directions, heightmap, slopes, dirs);
		
		trackProg->bind();
		glBindVertexArray(CubeArrayID);

		glUniformMatrix4fv(trackProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(trackProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		glm::mat4 moveUp = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3, 0));
		glm::mat4 scaleRail = glm::scale(glm::mat4(1.0f), glm::vec3(.75, .2, 1));

		// draw the coaster components
		for (int i = 0; i < positions.size(); i++) {
			glm::mat4 transRail = glm::translate(glm::mat4(1.0f), glm::vec3(positions[i].x, heightmap[i], positions[i].y));
			glm::mat4 rotateRail = glm::rotate(glm::mat4(1.0f), rotations[i], glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 slopeRail = glm::rotate(glm::mat4(1.0f), slopes[i], dirs[i]);
			M = moveUp * transRail * slopeRail * rotateRail* scaleRail;
			glUniformMatrix4fv(trackProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform1f(trackProg->getUniform("color_change"), (1.f / positions.size()) * i);
			glDrawElements(GL_TRIANGLES,36, GL_UNSIGNED_SHORT, (void*)0);
		}

		// draw the cart in positions of coaster moving with time
		glm::mat4 transCart = tracePath(10);
		moveUp = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3.5, 0));
		M = transCart * moveUp;
		glUniform1f(trackProg->getUniform("color_change"), 1.);
		glUniformMatrix4fv(trackProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		trackProg->unbind();
		//prog->bind();

		//glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		//glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		//glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);

		//prog->unbind();

		//heightshader->bind();

		////glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -3.0f, -50));
		//M = TransY;
		//glUniformMatrix4fv(heightshader->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//glUniformMatrix4fv(heightshader->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		//glUniformMatrix4fv(heightshader->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		//
		//vec3 offset = mycam.pos;
		//offset.y = 0;
		//offset.x = (int)offset.x;
		//offset.z = (int)offset.z;
		//glUniform3fv(heightshader->getUniform("camoff"), 1, &offset[0]);
		//glUniform3fv(heightshader->getUniform("campos"), 1, &mycam.pos[0]);
		//glBindVertexArray(VertexArrayID);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, HeightTex);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//glDrawElements(GL_TRIANGLES, MESHSIZE*MESHSIZE*6, GL_UNSIGNED_SHORT, (void*)0);

		//heightshader->unbind();
	}
};
//******************************************************************************************
int main(int argc, char **argv) {
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	srand(time(0));
	//srand(56534532);

	// track length, straight rate, straight deterioration, border size, min_segment_length
	application->createPath(100, 80, 10, 40, 2);

	// generate 1D perlin noise to apply slopes to the track
	application->createHeights();
	application->smoothHeights(5);
	application->calculateSlopes();
	application->vectorizeDirections();

	application->vectorLengths();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle())) {
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

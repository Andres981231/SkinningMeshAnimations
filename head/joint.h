#include <windef.h>
#include <glad/glad.h>  
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include "../3rdLibs/glm/glm/glm.hpp"
#include "../3rdLibs/glm/glm/gtc/matrix_transform.hpp"
#include "../3rdLibs/glm/glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Joint
{
public:
	int ID;
	int parent_ID;
	std::vector<int> child_list;
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
	glm::mat4 LocalMatrix;
	glm::mat4 GlobalMatrix;

	Joint(int id, int pid, glm::vec3 pos) {
		ID = id;
		parent_ID = pid;
		position = pos;
		// to check here
		forward = glm::vec3(1.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::vec3(0.0f, 0.0f, 1.0f);
		LocalMatrix[0] = glm::vec4(forward, 0.0f);
		LocalMatrix[1] = glm::vec4(up, 0.0f);
		LocalMatrix[2] = glm::vec4(right, 0.0f);
		LocalMatrix[3] = glm::vec4(position, 1.0f);
		GlobalMatrix = glm::mat4(1.0f);
	}
};
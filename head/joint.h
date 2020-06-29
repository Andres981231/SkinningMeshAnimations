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
	glm::vec3 position;				// global position at rest pos, will not change
	glm::vec3 local_position;		// position in father joint's coordinate, will change
	glm::vec3 forward;				// local forward, will change
	glm::vec3 up;					// local up, will change
	glm::vec3 right;				// local right, will change
	glm::mat4 LocalMatrix;			// local matrix, will change
	glm::mat4 GlobalMatrix;			// lobal matrix, will change
	glm::mat4 restGlobalMatrix;		// global matrix for rest pos, will not change

	Joint(int id, int pid, glm::vec3 pos) {
		ID = id;
		parent_ID = pid;
		position = pos;
		local_position = glm::vec3(0.0f);
		// to check here
		forward = glm::vec3(1.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::vec3(0.0f, 0.0f, 1.0f);
		/*LocalMatrix[0] = glm::vec4(forward, 0.0f);
		LocalMatrix[1] = glm::vec4(up, 0.0f);
		LocalMatrix[2] = glm::vec4(right, 0.0f);*/
		// LocalMatrix[3] = glm::vec4(position, 1.0f);
		LocalMatrix = glm::mat4(1.0f);
		GlobalMatrix = glm::mat4(1.0f);
		restGlobalMatrix = glm::mat4(1.0f);
	}
};
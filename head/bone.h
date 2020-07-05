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

class Bone
{
public:
	int parentID;
	int childID;
	glm::vec3 centerPos;

	Bone(int parent, int child, glm::vec3 pos) {
		parentID = parent;
		childID = child;
		centerPos = pos;
	};

};
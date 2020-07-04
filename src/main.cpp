
#include <windef.h>
#include <glad/glad.h>  
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <iostream>
#include "../3rdLibs/glm/glm/glm.hpp"
#include "../3rdLibs/glm/glm/gtc/matrix_transform.hpp"
#include "../3rdLibs/glm/glm/gtc/type_ptr.hpp"


#include "../head/my_texture.h"
#include "../head/shader_m.h"
#include "tiny_obj_loader.h"
#include "../head/joint.h"
#include <time.h>			// for frame animation


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The .obj .mtl and images are in Dir "model".                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------*/
//Here are some mouse and keyboard function. You can change that.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// set light parameters
void setLightPara(Shader shader, glm::vec3* pointLightPositions);
void renderJoint(Shader shader, Joint joint,glm::mat4 view, glm::mat4 projection);
void addJoint();
void setDefaultJoints();
void buildJoints();
void updateJoint();
glm::mat4 get_parent_globalM(Joint joint);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float degreeX = (360 * lastX / 400);
float degreeY = (360 * lastY / 300);
bool firstMouse = true;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float OX = 0;//should be update to a new coordinate
float OY = 0;
float OZ = 0;
// camera parameter
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
float currentFrame;
float yaw = -90.0;
float pitch = 0;
// static 
int mode = -1; //0-camera mode, 1-edit mode, 2-paint mode, 3-FK mode, 4-IK mode, 5-playmode;
int currentParent = -1;
int currentID = 0;
// which shader to render model
bool isBuilt = false;
Shader* model_shader;
std::vector<Joint> Joint_List;
// record frames
int frameChoose = 0;
std::vector<std::vector<Joint>> Frame_List;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (mode == 1) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			// add joint at current position
			addJoint();
		}
	}
}

float lastScrollY = 0.0f;
bool firstScroll = true;
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (firstScroll) {
		lastScrollY = yoffset;
		firstScroll = false;
	}
	float temp = yoffset - lastScrollY;
	/*if (mode == 1) {
		if (temp > 0) {
			currentID += 1;
			if (currentID > Joint_List.size()) { currentID = Joint_List.size(); }
		}
		else if (temp < 0) {
			currentID -= 1;
			if (currentID < -1) { currentID = -1; }
		}
		printf("current parent joint: %d, ", currentParent);
		printf("current choosen joint: %d\n", currentID);
	}*/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;
	float sensitivity = 0.1;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	glm::vec3 front;//why not in global 
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFront = glm::normalize(front);
	//std::cout << yaw << " " << pitch << std::endl;
}

int transparencySwitch = 0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		transparencySwitch = 1;
		//printf("transparent now\n");
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		transparencySwitch = 0;
		//printf("not transparent now\n");
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		mode = 0;
		printf("camera mode, use WASD to move and mouse to look around.\n");
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		if (isBuilt) {
			printf("can not edit bones!\n");
			return;
		}
		mode = 1;
		printf("edit mode: set joint manually\n");
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		if (mode == 1) {
			// after all joints are set, build local position and matrix for all of them
			buildJoints();
		}
		mode = 3;
		currentID = 0;
		printf("operate mode: control joints to move or rotate\n");
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { // play mode
		if (Frame_List.size() == 0) {
			printf("no frame recorded, can not play!\n"); return;
		}
		mode = 5;
		printf("play!\n");
	}
	if (mode == 0) {
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			currentID += 1;
			if (currentID > Joint_List.size() - 1) { currentID = 0; }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			currentID -= 1;
			if (currentID < 0) { currentID = Joint_List.size() - 1; }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
	}
	if (mode == 1) {
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			if (currentID != Joint_List.size())
				currentParent = currentID;
			printf("current parent joint: %d\n", currentParent);
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			currentID += 1;
			if (currentID > Joint_List.size()) { currentID = -1; }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			currentID -= 1;
			if (currentID < -1) { currentID = Joint_List.size(); }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			currentID = Joint_List.size();
			printf("click to create a new joint\n");
		}
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
			if (currentID != -1 && currentID != Joint_List.size()) {
				Joint temp = Joint_List[currentID];
				printf("joint information:\nid: %d,parent id: %d\nposition: %f, %f, %f\n\n", temp.ID, temp.parent_ID, temp.position.x, temp.position.y, temp.position.z);
			}
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			for (int i = 0; i < Joint_List.size(); i++) {
				Joint temp = Joint_List[i];
				printf("joint information:\nid: %d,parent id: %d\nposition: %f, %f, %f\n\n", temp.ID, temp.parent_ID, temp.position.x, temp.position.y, temp.position.z);
			}
		}
	}
	if (mode == 3) {
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			currentID += 1;
			if (currentID > Joint_List.size()-1) { currentID = 0; }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			currentID -= 1;
			if (currentID < 0) { currentID = Joint_List.size()-1; }
			printf("current parent joint: %d, ", currentParent);
			printf("current choosen joint: %d\n", currentID);
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			Frame_List.push_back(Joint_List);
			printf("frame recorded!\n");
		}
	}
	if (mode == 3 || mode == 4) {
		if (Frame_List.size() != 0) { 
			if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
				frameChoose -= 1;
				if (frameChoose <= -1) { frameChoose = Frame_List.size() - 1; }
				printf("choose frame: %d\n", frameChoose);
				Joint_List = Frame_List[frameChoose];
			}
			else if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
				frameChoose += 1;
				if (frameChoose >= Frame_List.size()) {frameChoose = 0;}
				printf("choose frame: %d\n", frameChoose);
				Joint_List = Frame_List[frameChoose];
			}
			if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
				printf("frame %d replaced!\n", frameChoose);
				Frame_List[frameChoose] = Joint_List;
			}
		}
	}
}

void processInput(GLFWwindow* window)
{
	/*currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;*/
	if (mode != 3) {	// move camera
		float cameraSpeed = 1.0f * deltaTime; // adjust accordingly
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (mode == 3) {
		float rotateSpeed = 15.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { // around forward axis
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(rotateSpeed), Joint_List[currentID].forward);
			Joint_List[currentID].up = glm::vec3(ro * glm::vec4(Joint_List[currentID].up,0.0f));
			Joint_List[currentID].right = glm::vec3(ro * glm::vec4(Joint_List[currentID].right, 0.0f));
			Joint_List[currentID].LocalMatrix[1] = glm::vec4(Joint_List[currentID].up,0.0f);
			Joint_List[currentID].LocalMatrix[2] = glm::vec4(Joint_List[currentID].right, 0.0f);
		}	
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(-rotateSpeed), Joint_List[currentID].forward);
			Joint_List[currentID].up = glm::vec3(ro * glm::vec4(Joint_List[currentID].up, 0.0f));
			Joint_List[currentID].right = glm::vec3(ro * glm::vec4(Joint_List[currentID].right, 0.0f));
			Joint_List[currentID].LocalMatrix[1] = glm::vec4(Joint_List[currentID].up, 0.0f);
			Joint_List[currentID].LocalMatrix[2] = glm::vec4(Joint_List[currentID].right, 0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(rotateSpeed), Joint_List[currentID].up);
			Joint_List[currentID].forward = glm::vec3(ro * glm::vec4(Joint_List[currentID].forward, 0.0f));
			Joint_List[currentID].right = glm::vec3(ro * glm::vec4(Joint_List[currentID].right, 0.0f));
			Joint_List[currentID].LocalMatrix[0] = glm::vec4(Joint_List[currentID].forward, 0.0f);
			Joint_List[currentID].LocalMatrix[2] = glm::vec4(Joint_List[currentID].right, 0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(-rotateSpeed), Joint_List[currentID].up);
			Joint_List[currentID].forward = glm::vec3(ro * glm::vec4(Joint_List[currentID].forward, 0.0f));
			Joint_List[currentID].right = glm::vec3(ro * glm::vec4(Joint_List[currentID].right, 0.0f));
			Joint_List[currentID].LocalMatrix[0] = glm::vec4(Joint_List[currentID].forward, 0.0f);
			Joint_List[currentID].LocalMatrix[2] = glm::vec4(Joint_List[currentID].right, 0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(rotateSpeed), Joint_List[currentID].right);
			Joint_List[currentID].forward = glm::vec3(ro * glm::vec4(Joint_List[currentID].forward, 0.0f));
			Joint_List[currentID].up = glm::vec3(ro * glm::vec4(Joint_List[currentID].up, 0.0f));
			Joint_List[currentID].LocalMatrix[0] = glm::vec4(Joint_List[currentID].forward, 0.0f);
			Joint_List[currentID].LocalMatrix[1] = glm::vec4(Joint_List[currentID].up, 0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			glm::mat4 ro = glm::mat4(1.0f);
			ro = glm::rotate(ro, glm::radians(-rotateSpeed), Joint_List[currentID].right);
			Joint_List[currentID].forward = glm::vec3(ro * glm::vec4(Joint_List[currentID].forward, 0.0f));
			Joint_List[currentID].up = glm::vec3(ro * glm::vec4(Joint_List[currentID].up, 0.0f));
			Joint_List[currentID].LocalMatrix[0] = glm::vec4(Joint_List[currentID].forward, 0.0f);
			Joint_List[currentID].LocalMatrix[1] = glm::vec4(Joint_List[currentID].up, 0.0f);
		}
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
// no use
void initPMV()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60, SCR_WIDTH / SCR_HEIGHT, 0.1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt
	(
		3, 3, 3,
		0, 0, 0,
		0, 1, 0
	);

}
// no use
void changePMV()
{


}
/*-----------------------------------------------------------------------*/



//This an function to get v, vt and vn. 
bool make_face(std::vector<float> v, std::vector<float> vt, std::vector<float> vn, std::vector<unsigned int> f,
	std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs)
{
	if (f.size() % 3 != 0)
		return false;
	// i trangles
	for (int i = 0; i < f.size()/3; i += 1)
	{
		int k = i * 3;
		// three vertexs of triangle
		for (int j = 0; j < 3; j++)
		{
			points.push_back(glm::vec3(v[f[k + j] * 3], v[f[k + j] * 3 + 1], v[f[k + j] * 3 + 2]));
			normals.push_back(glm::vec3(vn[f[k + j] * 3], vn[f[k + j] * 3 + 1], vn[f[k + j] * 3 + 2]));
			//uvs.push_back(glm::vec2(vt[f[k + j] * 2], vt[f[k + j] * 2 + 1]));
		}
		
	}
}
// no use
void get_vec3(std::vector<float> list, std::vector<glm::vec3> &vec)
{
	int n = list.size() / 3;
	for (int i = 0; i < n; i++)
	{
		vec.push_back(glm::vec3(list[i], list[i + 1], list[i + 2]));
	}
}
// no use
void get_vec2(std::vector<float> list, std::vector<glm::vec2>& vec)
{
	int n = list.size() / 2;
	for (int i = 0; i < n; i++)
	{
		vec.push_back(glm::vec2(list[i], list[i + 1]));
	}
}



int main()
{
	//
	/*glm::vec3 t1(1, 0, 0);
	glm::vec3 t2(0, 1, 0);
	glm::vec3 t3(1, 1, 1);
	glm::mat3 m;
	m[0] = t1; m[1] = t2; m[2] = t3;
	glm::vec3 t(1, 1, 1);
	glm::mat4 result = m * m;

	printf("%f, %f, %f\n", result[0][0], result[0][1], result[0][2]);
	printf("%f, %f, %f\n", result[1][0], result[1][1], result[1][2]);
	printf("%f, %f, %f\n", result[2][0], result[2][1], result[2][2]);*/
	//mode process
	if (mode == -1) {
		mode = 0;
		printf("camera mode, use wasd and mouse to look around.\n");
	}
	// set default joints with global position
	setDefaultJoints();
	
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	// key
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);

    gladLoadGL();  
    
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Here you need to fill construct function of class Shader. And you need to understand other funtions in Shader.//
	// Then, write code in shader_m.vs, shader_m.fs and shader_m.gs to finish the tasks.                             //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Shader first_shader("../src/shader_begin.vs","../src/shader_begin.fs");
	Shader my_shader(
		"../src/shader_m.vs",
		"../src/shader_m.fs"
	);
	//A shader for light visiable source
	Shader lampShader("../src/lamp.vs", "../src/lamp.fs");
	Shader jointShader("../src/joint.vs", "../src/joint.fs");
	


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// tiny::LoadObj is a function to load obj file. The output is shape_t and material_t.                         //
	// "shape.mesh" is a mesh struct. "mesh.positions", "mesh.normals", "mesh.texcoords" corresponds to v,vn,vt.   //
	// For example:                                                                                                //
	// positions[0],positions[1],positions[2] -> v 0,0,1                                                           //
	// positions[3],positions[4],positions[5] -> v 0,1,0                                                           //
	// "mesh.indice" corresponds to f, but it is different from f. Each element is an index for all of v,vn,vt.    //
	// positions[0],positions[1],positions[2] -> v 0,0,1  positions[0],positions[1],positions[2] -> v 0,0,1        //
	// You can read tiny_obj_loader.h to get more specific information.                                            //
	//                                                                                                             //
	// I have write make_face for you.  It will return v, vt, vn in vec form (each element if for one point).      //
	// These informations can help you to do normal mapping.  (You can calculate tangent here)                     //
	// Since i did not assign uv for noraml map, you just need use vt as uv for normal map, but you will find it is//
	//  ugly. So please render a box to show a nice normal mapping. (Do normal mapping on obj and box)             //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// how many shapes
	std::vector<tinyobj::shape_t> shapes;
	// no use
	std::vector<tinyobj::material_t> materials;
	/*char path[100];
	std::cin >> path;*/
	std::string if_load_succeed = tinyobj::LoadObj(shapes, materials,
		"../model/horse.obj"
	);
	//std::string if_load_succeed = tinyobj::LoadObj(shapes, materials, path);
	//printf("%s\n", if_load_succeed);
	// three list for infomation of each shape
	std::vector<unsigned int> obj_VBO_l, obj_VAO_l, obj_NUM_l;
	// temp
	unsigned int tVBO, tVAO;
	// calculation helper
	//glm::vec3 edge1, edge2, tangent, bitangent;
	//glm::vec2 deltaUV1, deltaUV2;
	float f;
	// fixed data for exception
	//glm::vec3 bakTangent(1, 0, 0);
	//glm::vec3 bakBitangent(0, 1, 0);
	//glm::vec3 lastT, lastB;
	for (int i = 0; i < shapes.size(); i++)
	{
		
		std::vector < glm::vec3 > out_vertices;
		std::vector < glm::vec2 > out_uvs;
		std::vector < glm::vec3 > out_normals;

		// out_vertices, out_uvs, out_normals will get v, vt and vn.
		make_face(shapes[i].mesh.positions, shapes[i].mesh.texcoords, shapes[i].mesh.normals, shapes[i].mesh.indices,
			out_vertices, out_normals, out_uvs);
		unsigned int tVBO, tVAO;
		// temp tVertices able to change if i > 1? Yes
		std::vector<float> tVertices;
		// all vertices of one shape
		for (int j = 0; j < out_vertices.size(); j++) {
			// pos
			tVertices.push_back(out_vertices[j].x); tVertices.push_back(out_vertices[j].y); tVertices.push_back(out_vertices[j].z);
			// normal
			tVertices.push_back(out_normals[j].x); tVertices.push_back(out_normals[j].y); tVertices.push_back(out_normals[j].z);
			// uvs
			//tVertices.push_back(out_uvs[j].x); tVertices.push_back(out_uvs[j].y);
			// T B
			// one triangle one calculation
			
			// the other two vertices

			// find nearest joint first
			float nearest_dist = 9999;
			int nearest_joint;
			for (int k = 0; k < Joint_List.size(); k++) {
				float dist = pow(pow((out_vertices[j].x - Joint_List[k].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[k].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[k].position.z), 2), 0.5f);
				if (dist < nearest_dist) {
					nearest_dist = dist;
					nearest_joint = k;
				}
			}
			// find nearest child joint
			nearest_dist = 9999;
			int nearest_child = -1;
			std::vector<int> childList = Joint_List[nearest_joint].child_list;
			for (int k = 0; k < childList.size(); k++) {
				int child_id = childList[k];
				float dist = pow(pow((out_vertices[j].x - Joint_List[child_id].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[child_id].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[child_id].position.z), 2), 0.5f);
				if (dist < nearest_dist) {
					nearest_dist = dist;
					nearest_child = child_id;
				}
			}

			// calc bone's weight for the vertice
			float bone1_weight, bone2_weight;
			float bone1_dist = 9999;
			float bone2_dist = 9999;

			if (nearest_child != -1)
			{
				glm::vec3 bone1_center((Joint_List[nearest_joint].position.x + Joint_List[nearest_child].position.x) / 2,
					(Joint_List[nearest_joint].position.y + Joint_List[nearest_child].position.y) / 2,
					(Joint_List[nearest_joint].position.z + Joint_List[nearest_child].position.z) / 2);
				bone1_dist = pow(pow((out_vertices[j].x - bone1_center.x), 2) +
					pow((out_vertices[j].y - bone1_center.y), 2) +
					pow((out_vertices[j].z - bone1_center.z), 2), 0.5f);
			}
			int parent_id = Joint_List[nearest_joint].parent_ID;
			if (parent_id != -1)
			{
				glm::vec3 bone2_center((Joint_List[nearest_joint].position.x + Joint_List[parent_id].position.x) / 2,
					(Joint_List[nearest_joint].position.y + Joint_List[parent_id].position.y) / 2,
					(Joint_List[nearest_joint].position.z + Joint_List[parent_id].position.z) / 2);
				bone2_dist = pow(pow((out_vertices[j].x - bone2_center.x), 2) +
					pow((out_vertices[j].y - bone2_center.y), 2) +
					pow((out_vertices[j].z - bone2_center.z), 2), 0.5f);
			}
			// calc bones' weight according to the distance from vertice to bone's center
			bone1_weight = 1 - (bone1_dist / (bone1_dist + bone2_dist));
			bone2_weight = 1 - bone1_weight;

			// calc joint weight to weight painting
			float joint_weight1, joint_weight2;
			float joint_dist1, joint_dist2;
			if (bone1_dist > bone2_dist)
			{
				joint_dist1 = pow(pow((out_vertices[j].x - Joint_List[parent_id].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[parent_id].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[parent_id].position.z), 2), 0.5f);
				joint_dist2 = pow(pow((out_vertices[j].x - Joint_List[nearest_joint].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[nearest_joint].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[nearest_joint].position.z), 2), 0.5f);
				joint_weight1 = 1 - (joint_dist1 / (joint_dist1 + joint_dist2));
				joint_weight2 = 1 - joint_weight1;

				// push three joints
				tVertices.push_back(parent_id);
				tVertices.push_back(nearest_joint);
				tVertices.push_back(nearest_child);
				// push joint weights
				tVertices.push_back(joint_weight1);
				tVertices.push_back(joint_weight2);
				// push bone weights
				tVertices.push_back(bone2_weight);
				tVertices.push_back(bone1_weight);
			}
			else {
				joint_dist1 = pow(pow((out_vertices[j].x - Joint_List[nearest_child].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[nearest_child].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[nearest_child].position.z), 2), 0.5f);
				joint_dist2 = pow(pow((out_vertices[j].x - Joint_List[nearest_joint].position.x), 2) +
					pow((out_vertices[j].y - Joint_List[nearest_joint].position.y), 2) +
					pow((out_vertices[j].z - Joint_List[nearest_joint].position.z), 2), 0.5f);
				joint_weight1 = 1 - (joint_dist1 / (joint_dist1 + joint_dist2));
				joint_weight2 = 1 - joint_weight1;

				// push three joints
				tVertices.push_back(nearest_child);
				tVertices.push_back(nearest_joint);
				tVertices.push_back(parent_id);
				// push joint weights
				tVertices.push_back(joint_weight1);
				tVertices.push_back(joint_weight2);
				// push bone weights
				tVertices.push_back(bone1_weight);
				tVertices.push_back(bone2_weight);
			}
		}
		// set attributes for tVAO tVBO
		glGenVertexArrays(1, &tVAO);
		glGenBuffers(1, &tVBO);
		glBindVertexArray(tVAO);
		glBindBuffer(GL_ARRAY_BUFFER, tVBO);
		glBufferData(GL_ARRAY_BUFFER, tVertices.size()*sizeof(float), &tVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0); // pos
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2); // related joint id
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3); // joint weights
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(11 * sizeof(float)));
		glEnableVertexAttribArray(4); // bone weights
		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		//glEnableVertexAttribArray(2); 
		// push to VAO,VBO,NUM list
		obj_VBO_l.push_back(tVBO); obj_VAO_l.push_back(tVAO); obj_NUM_l.push_back(out_vertices.size());
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render a box to show nice normal mapping.                                                                   //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float vertices_cube_0[] = {

		// positions          // normals           // texture coords

		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 

	};
	// operation on cube vertices
	//std::vector<glm::vec3> cube_vertices, cube_normals;
	//std::vector<glm::vec2> cube_uvs;
	

	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cube_0), &vertices_cube_0[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coords
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// You need to fill this function which is defined in my_texture.h. The parameter is the path of your image.   //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// cat face
	//unsigned int texture1 = loadTexture("../../model/2.jpg");
	//unsigned int texturePika = loadTexture("../../model/p_r.jpg");
	//unsigned int textureNormal = loadTexture("../../model/normal_map.jpg");
	//unsigned int textureEye = loadTexture("../../model/eye.jpg");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Here we defined pointlights in shader and passed some parameter for you. You can take this as an example.   //
	// Or you can change it if you like.                                                                           //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	glm::vec3 pointLightPositions[] = {
		glm::vec3(5.7f,  5.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	
	setLightPara(first_shader, pointLightPositions);
	setLightPara(my_shader, pointLightPositions);
	setLightPara(jointShader, pointLightPositions);
	// normal map switch
	//my_shader.setBool("useNormalMap", true);

	clock_t ltime = 0;
	clock_t ctime = 0;
	double duration = 0;
	float clip = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
		// clock counter
		ctime = clock();

		if (mode == 5) {
			duration = ((double)ctime - (double)ltime) / CLK_TCK;
			if (duration >= 1.0 / 30.0) {   // 30 here is the total interpolation in one second
				ltime = ctime;
				clip += 1.0f / 30.0f;		// 30 here is the interpolation times between two frames
				int frameNum = (int)clip;
				if (frameNum >= Frame_List.size()) {
					clip -= frameNum;
					frameNum = 0;
				}
				float t = clip - frameNum;
				//printf("%d, %f\n", frameNum, clip);
				for (int i = 0; i < Joint_List.size(); i++) {

					Joint_List[i].forward = glm::normalize((1 - t) * Frame_List[frameNum][i].forward + t * Frame_List[(frameNum + 1) % Frame_List.size()][i].forward);
					Joint_List[i].up = glm::normalize((1 - t) * Frame_List[frameNum][i].up + t * Frame_List[(frameNum + 1) % Frame_List.size()][i].up);
					Joint_List[i].right = glm::normalize((1 - t) * Frame_List[frameNum][i].right + t * Frame_List[(frameNum + 1) % Frame_List.size()][i].right);
					Joint_List[i].LocalMatrix[0] = glm::vec4(Joint_List[i].forward, 0.0f);
					Joint_List[i].LocalMatrix[1] = glm::vec4(Joint_List[i].up, 0.0f);
					Joint_List[i].LocalMatrix[2] = glm::vec4(Joint_List[i].right, 0.0f);
				}
			}
		}
        // input
        // -----
        processInput(window);
		// update global matrix
		if (isBuilt) {
			model_shader = &my_shader;
			updateJoint();
		}
		else {
			model_shader = &first_shader;
		}
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
		//Update Camera Matrix
		glFlush();
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glLightModeli(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// for transparency
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::mat4 projection = glm::perspective(0.785f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 model = glm::mat4(1.0f); // not used
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//  Render the lamp cubes                                                                                      //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f); // re-model
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lampShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//  Render joint and target                                                                                    //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// joints
		for (int i = 0; i < Joint_List.size(); ++i) {
			renderJoint(jointShader, Joint_List[i], view, projection);
		}
		// mode 1 cube cursor
		if (mode == 1) {
			jointShader.use();
			jointShader.setMat4("projection", projection);
			jointShader.setMat4("view", view);
			jointShader.setVec3("Color", 0.4, 0.4, 0.8);
			model = glm::mat4(1.0f);
			model = glm::translate(model, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront));
			model = glm::scale(model, glm::vec3(0.01f));
			jointShader.setMat4("model", model);
			jointShader.setFloat("alpha", 0.5);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			// render three direction
			jointShader.setVec3("Color", 1.0, 0, 0);
			model = glm::mat4(1.0f);
			model = glm::translate(model, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront) + glm::vec3(0.01f, 0, 0));
			model = glm::scale(model, glm::vec3(0.005f));
			jointShader.setMat4("model", model);
			jointShader.setFloat("alpha", 0.5);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			//
			jointShader.setVec3("Color", 0, 1, 0);
			model = glm::mat4(1.0f);
			model = glm::translate(model, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront) + glm::vec3(0, 0.01f, 0));
			model = glm::scale(model, glm::vec3(0.005f));
			jointShader.setMat4("model", model);
			jointShader.setFloat("alpha", 0.5);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			//
			jointShader.setVec3("Color", 0, 0, 1);
			model = glm::mat4(1.0f);
			model = glm::translate(model, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront) + glm::vec3(0, 0, 0.01f));
			model = glm::scale(model, glm::vec3(0.005f));
			jointShader.setMat4("model", model);
			jointShader.setFloat("alpha", 0.5);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//  Render the object in .obj file. You need to set materials and wrap texture for objects.                    //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		(*model_shader).use();
		(*model_shader).setInt("mat.diffuse", 1);
		(*model_shader).setInt("mat.specular", 1);
		//my_shader.setInt("normalMap", 2);
		/*glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texturePika);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureEye);*/
		//my_shader.setVec3("Color", 0.8f,0.5f,0.5f);
		// pass weight painting colors to the shader
		if (isBuilt) {
			for (int i = 0; i < Joint_List.size(); ++i) {
				std::string joint_color("jointList[].color");
				joint_color.insert(10, std::to_string(i));
				std::string joint_globalMat("jointList[].globalMat");
				joint_globalMat.insert(10, std::to_string(i));
				std::string joint_offsetMat("jointList[].offsetMat");
				joint_offsetMat.insert(10, std::to_string(i));
				(*model_shader).setVec3(joint_color, Joint_List[i].color);
				(*model_shader).setMat4(joint_globalMat, Joint_List[i].GlobalMatrix);
				(*model_shader).setMat4(joint_offsetMat, glm::inverse(Joint_List[i].restGlobalMatrix));
			}
		}
		else {
			// give color to begin shader
			(*model_shader).setVec3("Color", glm::vec3(0.8f,0.5f,0.5f));
		}

		if (transparencySwitch == 1) {
			(*model_shader).setFloat("alpha", 0.3);
		}
		else {
			(*model_shader).setFloat("alpha", 1.0);
		}
		

		(*model_shader).setMat4("projection", projection);
		(*model_shader).setMat4("view", view);
		model = glm::mat4(1.0f);
		(*model_shader).setMat4("model", model);
		(*model_shader).setVec3("viewPos", cameraPos);
		//my_shader.setVec3("viewPos2VS", cameraPos);
		for (int i = 0; i < obj_VAO_l.size(); i++) {
			glBindVertexArray(obj_VAO_l[i]);
			//printf("%d  %d\n", obj_VAO_l[i], obj_NUM_l[i]);
			glDrawArrays(GL_TRIANGLES, 0, obj_NUM_l[i]);
		}

		
		/////////////////////////////////////////////////////////////////////
		
		/////////////////////////////end/////////////////////////////////////



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	for (int i = 0; i < obj_VAO_l.size(); i++) {
		glDeleteVertexArrays(1, &obj_VAO_l[i]);
		glDeleteBuffers(1, &obj_VBO_l[i]);
	}
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void setLightPara(Shader shader, glm::vec3* pointLightPositions) {
	shader.use();
	shader.setVec3("dirLight.direction", glm::vec3(1.01f, 1.01f, 1.01f));
	//my_shader.setVec3("dirLightDir2VS", glm::vec3(1.01f, 1.01f, 1.01f));
	shader.setVec3("dirLight.ambient", glm::vec3(0.01f, 0.01f, 0.02f));
	shader.setVec3("dirLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	// point light 1
	shader.setVec3("pointLights[0].position", pointLightPositions[0]);
	//my_shader.setVec3("lightPos[0]", pointLightPositions[0]);
	shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[0].constant", 1.0f);
	shader.setFloat("pointLights[0].linear", 0.09);
	shader.setFloat("pointLights[0].quadratic", 0.032);
	// point light 2
	shader.setVec3("pointLights[1].position", pointLightPositions[1]);
	//my_shader.setVec3("lightPos[1]", pointLightPositions[1]);
	shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[1].constant", 1.0f);
	shader.setFloat("pointLights[1].linear", 0.09);
	shader.setFloat("pointLights[1].quadratic", 0.032);
	// point light 3
	shader.setVec3("pointLights[2].position", pointLightPositions[2]);
	//my_shader.setVec3("lightPos[2]", pointLightPositions[2]);
	shader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[2].diffuse", 0.6f, 0.1f, 0.8f);
	shader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[2].constant", 1.0f);
	shader.setFloat("pointLights[2].linear", 0.09);
	shader.setFloat("pointLights[2].quadratic", 0.032);
	// point light 4
	shader.setVec3("pointLights[3].position", pointLightPositions[3]);
	//my_shader.setVec3("lightPos[3]", pointLightPositions[3]);
	shader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[3].diffuse", 0.1f, 1.1f, 0.8f);
	shader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[3].constant", 1.0f);
	shader.setFloat("pointLights[3].linear", 0.09);
	shader.setFloat("pointLights[3].quadratic", 0.032);
	// alpha
	shader.setFloat("alpha", 1.0);
}

void renderJoint(Shader shader, Joint joint ,glm::mat4 view, glm::mat4 projection) {
	// target
	shader.use();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	if (joint.ID == currentID) {
		shader.setVec3("Color", 0.4, 0.8, 0.4);
	}
	else if (joint.ID == currentParent) {
		shader.setVec3("Color", 0.9, 0.2, 0.2);
	}
	else {
		shader.setVec3("Color", 0.4, 0.4, 0.8);
	}
	
	glm::mat4 model = glm::mat4(1.0f);
	glm::vec3 fp, f1, f2, f3;
	glm::vec4 temp = get_parent_globalM(joint) * glm::vec4(joint.local_position, 1.0f);
	//printf("%f, %f, %f\n", temp[0], temp[1], temp[2]);
	glm::vec3 changedPosition = glm::vec3(temp);
	if (isBuilt)
		fp = changedPosition;
	else
		fp = joint.position;
	model = glm::translate(model, fp);
	model = glm::scale(model, glm::vec3(0.01f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	// render three direction
	shader.setVec3("Color", 1.0, 0, 0);
	model = glm::mat4(1.0f);
	temp = get_parent_globalM(joint) * glm::vec4(joint.forward, 0.0f);
	if (isBuilt)
		f1 = 0.01f * joint.forward;
	else
		f1 = 0.01f * glm::normalize(glm::vec3(temp));
	model = glm::translate(model, fp + f1);
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//
	shader.setVec3("Color", 0, 1, 0);
	model = glm::mat4(1.0f);
	temp = get_parent_globalM(joint) * glm::vec4(joint.up, 0.0f);
	if (isBuilt)
		f2 = 0.01f * joint.up;
	else
		f2 = 0.01f * glm::normalize(glm::vec3(temp));
	model = glm::translate(model, fp + f2);
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//
	shader.setVec3("Color", 0, 0, 1);
	model = glm::mat4(1.0f);
	temp =  get_parent_globalM(joint) * glm::vec4(joint.right, 0.0f);
	if (isBuilt)
		f3 = 0.01f * joint.right;
	else
		f3 = 0.01f * glm::normalize(glm::vec3(temp));
	model = glm::translate(model, fp + f3);
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void addJoint() {
	// new joint
	if (currentID == Joint_List.size()) {
		Joint temp(currentID, currentParent, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront), glm::vec3(0,0,0));
		// push to list
		Joint_List.push_back(temp);
		currentParent = currentID;
		currentID = Joint_List.size();
		if (currentParent != -1) {
			Joint_List[currentParent].child_list.push_back(currentID);
		}
		printf("current parent joint: %d, ", currentParent);
		printf("current choosen joint: %d\n", currentID);
	}
	else {
		if(currentID != -1)
			Joint_List[currentID].position = cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront);
	}
}

void setDefaultJoints() {
	float r = ((double)rand() / (RAND_MAX));
	float g = ((double)rand() / (RAND_MAX));
	float b = ((double)rand() / (RAND_MAX));
	glm::vec3 c(r, g, b);
	Joint newj(0, -1, glm::vec3(0.001691f, 0.566665f, 0.021455f), c);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(1, 0, glm::vec3(-0.000819, 0.481337, 0.121768), c);
	Joint_List[0].child_list.push_back(1);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(2, 1, glm::vec3(-0.001076, 0.654315, 0.155140), c);
	Joint_List[1].child_list.push_back(2);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(3, 2, glm::vec3(0.000657, 0.735756, 0.256516), c);
	Joint_List[2].child_list.push_back(3);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(4, 3, glm::vec3(-0.001294, 0.635072, 0.362139), c);
	Joint_List[3].child_list.push_back(4);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(5, 0, glm::vec3(-0.043955, 0.440327, 0.161949), c);
	Joint_List[0].child_list.push_back(5);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(6, 5, glm::vec3(-0.077542, 0.370297, 0.094476), c);
	Joint_List[5].child_list.push_back(6);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(7, 6, glm::vec3(-0.075867, 0.210244, 0.046379), c);
	Joint_List[6].child_list.push_back(7);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(8, 7, glm::vec3(-0.070106, 0.089600, 0.017584), c);
	Joint_List[7].child_list.push_back(8);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(9, 8, glm::vec3(-0.069601, 0.045441, 0.021094), c);
	Joint_List[8].child_list.push_back(9);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(10, 0, glm::vec3(0.052980, 0.442547, 0.160829), c);
	Joint_List[0].child_list.push_back(10);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(11, 10, glm::vec3(0.073885, 0.371108, 0.159506), c);
	Joint_List[10].child_list.push_back(11);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(12, 11, glm::vec3(0.070374, 0.232418, 0.290771), c);
	Joint_List[11].child_list.push_back(12);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(13, 12, glm::vec3(0.070706, 0.152662, 0.378457), c);
	Joint_List[12].child_list.push_back(13);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(14, 13, glm::vec3(0.069522, 0.105653, 0.415685), c);
	Joint_List[13].child_list.push_back(14);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(15, 0, glm::vec3(0.000392, 0.584464, -0.234979), c);
	Joint_List[0].child_list.push_back(15);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(16, 15, glm::vec3(-0.082763, 0.527925, -0.253043), c);
	Joint_List[15].child_list.push_back(16);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(17, 16, glm::vec3(-0.077468, 0.402789, -0.333127), c);
	Joint_List[16].child_list.push_back(17);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(18, 17, glm::vec3(-0.065810, 0.296157, -0.466499), c);
	Joint_List[17].child_list.push_back(18);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(19, 18, glm::vec3(-0.069273, 0.141381, -0.531791), c);
	Joint_List[18].child_list.push_back(19);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(20, 19, glm::vec3(-0.064983, 0.126961, -0.584126), c);
	Joint_List[19].child_list.push_back(20);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(21, 15, glm::vec3(0.081760, 0.524731, -0.259197), c);
	Joint_List[15].child_list.push_back(21);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(22, 21, glm::vec3(0.065342, 0.392328, -0.304268), c);
	Joint_List[21].child_list.push_back(22);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(23, 22, glm::vec3(0.069011, 0.247179, -0.386741), c);
	Joint_List[22].child_list.push_back(23);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(24, 23, glm::vec3(0.063263, 0.091644, -0.419300), c);
	Joint_List[23].child_list.push_back(24);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(25, 24, glm::vec3(0.061767, 0.040162, -0.433945), c);
	Joint_List[24].child_list.push_back(25);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(26, 15, glm::vec3(0.001431, 0.575117, -0.384786), c);
	Joint_List[15].child_list.push_back(26);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(27, 26, glm::vec3(0.007507, 0.619909, -0.439229), c);
	Joint_List[26].child_list.push_back(27);
	Joint_List.push_back(newj);
	r = ((double)rand() / (RAND_MAX));
	g = ((double)rand() / (RAND_MAX));
	b = ((double)rand() / (RAND_MAX));
	c = glm::vec3(r, g, b);
	newj = Joint(28, 27, glm::vec3(0.015131, 0.634682, -0.548828), c);
	Joint_List[27].child_list.push_back(28);
	Joint_List.push_back(newj);
}

// after all joints are set, build local position and matrix for all of them
void buildJoints() {
	printf("buinding joints...\n");
	for (int i = 0; i < Joint_List.size(); ++i)
	{
		
		if (Joint_List[i].parent_ID == -1) {
			Joint_List[i].local_position = Joint_List[i].position;
			Joint_List[i].LocalMatrix[0] = glm::vec4(Joint_List[i].forward, 0.0f);
			Joint_List[i].LocalMatrix[1] = glm::vec4(Joint_List[i].up, 0.0f);
			Joint_List[i].LocalMatrix[2] = glm::vec4(Joint_List[i].right, 0.0f);
			Joint_List[i].LocalMatrix[3] = glm::vec4(Joint_List[i].local_position, 1.0f);
			Joint_List[i].GlobalMatrix = Joint_List[i].LocalMatrix;
			Joint_List[i].restGlobalMatrix = Joint_List[i].GlobalMatrix;
		}
		else {
			Joint_List[i].local_position = Joint_List[i].position - Joint_List[Joint_List[i].parent_ID].position;
			Joint_List[i].LocalMatrix[0] = glm::vec4(Joint_List[i].forward, 0.0f);
			Joint_List[i].LocalMatrix[1] = glm::vec4(Joint_List[i].up, 0.0f);
			Joint_List[i].LocalMatrix[2] = glm::vec4(Joint_List[i].right, 0.0f);
			Joint_List[i].LocalMatrix[3] = glm::vec4(Joint_List[i].local_position, 1.0f);
			Joint_List[i].GlobalMatrix =  Joint_List[Joint_List[i].parent_ID].GlobalMatrix * Joint_List[i].LocalMatrix;
			Joint_List[i].restGlobalMatrix = Joint_List[i].GlobalMatrix;
		}
	}
	isBuilt = true;
	printf("build done!\n");
}

void updateJoint() {
	for (int i = 0; i < Joint_List.size(); ++i)
	{
		if (Joint_List[i].parent_ID == -1) {
			Joint_List[i].GlobalMatrix = Joint_List[i].LocalMatrix;
		}
		else {
			Joint_List[i].GlobalMatrix = Joint_List[Joint_List[i].parent_ID].GlobalMatrix * Joint_List[i].LocalMatrix;
		}
	}
}

glm::mat4 get_parent_globalM(Joint joint) {
	if (joint.parent_ID == -1) {
		return glm::mat4(1.0f);
	}
	else {
		return Joint_List[joint.parent_ID].GlobalMatrix;
	}
}
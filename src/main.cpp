
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The .obj .mtl and images are in Dir "model".                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------*/
//Here are some mouse and keyboard function. You can change that.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Shader my_shader);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// set light parameters
void setLightPara(Shader shader, glm::vec3* pointLightPositions);
void renderJoint(Shader shader, Joint joint,glm::mat4 view, glm::mat4 projection);
void addJoint();

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
int mode = -1; //0-camera mode, 1-edit mode
int currentParent = -1;
int currentID = 0;
std::vector<Joint> Joint_List;

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
	if (mode == 1) {
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
	}
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
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		mode = 1;
		printf("edit mode: set joint manually\n");
	}
	if (mode == 1) {
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			if (currentID != Joint_List.size())
				currentParent = currentID;
			printf("current parent joint: %d\n", currentParent);
		}
	}
}

void processInput(GLFWwindow* window, Shader my_shader)
{
	/*currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;*/
	float cameraSpeed = 2.0f * deltaTime; // adjust accordingly
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
	//mode process
	if (mode == -1) {
		mode = 0;
		printf("camera mode, use wasd and mouse to look around.\n");
	}
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
	std::string if_load_succeed = tinyobj::LoadObj(shapes, materials,
		"../model/horse.obj"
	);
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
			
		}
		// set attributes for tVAO tVBO
		glGenVertexArrays(1, &tVAO);
		glGenBuffers(1, &tVBO);
		glBindVertexArray(tVAO);
		glBindBuffer(GL_ARRAY_BUFFER, tVBO);
		glBufferData(GL_ARRAY_BUFFER, tVertices.size()*sizeof(float), &tVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0); // pos
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1); // normal
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
	
	setLightPara(my_shader, pointLightPositions);
	setLightPara(jointShader, pointLightPositions);
	// normal map switch
	//my_shader.setBool("useNormalMap", true);

    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window,my_shader);

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
		// target
		jointShader.use();
		jointShader.setMat4("projection", projection);
		jointShader.setMat4("view", view);
		jointShader.setVec3("Color", 0.4, 0.4, 0.8);
		model = glm::mat4(1.0f);
		model = glm::translate(model, cameraPos + glm::vec3(0.2f)*glm::normalize(cameraFront));
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


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//  Render the object in .obj file. You need to set materials and wrap texture for objects.                    //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		my_shader.use();
		my_shader.setInt("mat.diffuse", 1);
		my_shader.setInt("mat.specular", 1);
		//my_shader.setInt("normalMap", 2);
		/*glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texturePika);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureEye);*/
		my_shader.setVec3("Color", 0.8f,0.5f,0.5f);
		if (transparencySwitch == 1) {
			my_shader.setFloat("alpha", 0.3);
		}
		else {
			my_shader.setFloat("alpha", 1.0);
		}
		

		my_shader.setMat4("projection", projection);
		my_shader.setMat4("view", view);
		model = glm::mat4(1.0f);
		my_shader.setMat4("model", model);
		my_shader.setVec3("viewPos", cameraPos);
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
	model = glm::translate(model, joint.position);
	model = glm::scale(model, glm::vec3(0.01f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	// render three direction
	shader.setVec3("Color", 1.0, 0, 0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, joint.position + glm::vec3(0.01f, 0, 0));
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//
	shader.setVec3("Color", 0, 1, 0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, joint.position + glm::vec3(0, 0.01f, 0));
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//
	shader.setVec3("Color", 0, 0, 1);
	model = glm::mat4(1.0f);
	model = glm::translate(model, joint.position + glm::vec3(0, 0, 0.01f));
	model = glm::scale(model, glm::vec3(0.005f));
	shader.setMat4("model", model);
	shader.setFloat("alpha", 1);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void addJoint() {
	// new joint
	if (currentID == Joint_List.size()) {
		Joint temp(currentID, currentParent, cameraPos + glm::vec3(0.2f) * glm::normalize(cameraFront));
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
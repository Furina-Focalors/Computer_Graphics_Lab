#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL2\soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp> // glm::value_ptr
#include <glm\gtc\matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Sphere.h"
#include "Utils.h"
#include "Cylinder.h"
using namespace std;

#define numVAOs 3
#define numVBOs 8

//float cameraX, cameraY, cameraZ;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);// 相机坐标
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);// 相机朝向
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);// 相机上方向

float yaw = -90.0f;// 偏航角 绕y轴旋转
float pitch = 0.0f;// 俯仰角 绕x轴旋转
float lastX, lastY;// 指针上次停留的位置
bool firstMouse = true;// 初始化

GLuint renderingProgram, lightShaderProgram, skyboxShaderProgram, sphRenderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint sphereTexture;
GLuint cylinderTexture;
GLuint skyboxTexture;
float rotAmt = 0.0f;

float fovy = 1.0472f;// 视场垂直角度，越大可显示的范围越广
float near = 0.1f;// 离摄像机最近的可见点距离，过近不渲染
float far = 1000.0f;// 离摄像机最远的可见点距离，过远不渲染

glm::vec3 lightLoc = glm::vec3(0.0f, 5.0f, 2.0f);// 光源位置

// variable allocation for display
GLuint mvLoc, projLoc, viewLoc, nLoc, lLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;// 光照、纹理
int width, height;
float aspect;// 宽高比
glm::mat4 pMat, vMat, mMat, mvMat, view, invTrMat;
glm::vec3 currentLightPos, transformed;// 光源位置
glm::vec3 lightShift = glm::vec3(0.0f, 0.0f, 0.0f);// 光源位置交互平移
float lightPos[3];

// white light
float globalAmbient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
float lightAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// gold material
float* matAmb = Utils::silverAmbient();
float* matDif = Utils::silverDiffuse();
float* matSpe = Utils::silverSpecular();
float matShi = Utils::silverShininess();

int prec = 48;
Sphere mySphere = Sphere(prec);
Cylinder myCylinder = Cylinder(prec);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void setupVertices(void);
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void installLights(glm::mat4 vMatrix, GLint rp);

void installLights(glm::mat4 vMatrix, GLint rp) {
	transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(rp, "globalAmbient");
	ambLoc = glGetUniformLocation(rp, "light.ambient");
	diffLoc = glGetUniformLocation(rp, "light.diffuse");
	specLoc = glGetUniformLocation(rp, "light.specular");
	posLoc = glGetUniformLocation(rp, "light.position");
	mambLoc = glGetUniformLocation(rp, "material.ambient");
	mdiffLoc = glGetUniformLocation(rp, "material.diffuse");
	mspecLoc = glGetUniformLocation(rp, "material.specular");
	mshiLoc = glGetUniformLocation(rp, "material.shininess");

	//  set the uniform light and material values in the shader
	glProgramUniform4fv(rp, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(rp, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(rp, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(rp, specLoc, 1, lightSpecular);
	glProgramUniform3fv(rp, posLoc, 1, lightPos);
	glProgramUniform4fv(rp, mambLoc, 1, matAmb);
	glProgramUniform4fv(rp, mdiffLoc, 1, matDif);
	glProgramUniform4fv(rp, mspecLoc, 1, matSpe);
	glProgramUniform1f(rp, mshiLoc, matShi);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 0.05f;
	float lightSensitivity = 0.05f;
	// 相机控制
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
	// 光源控制
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(0.0f, 0.0f, 1.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(0.0f, 0.0f, 1.0f);
}

/*
 * @brief 指针位置回调函数
 *
*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw = yaw + xoffset;
	pitch = pitch + yoffset;

	// 防止 pitch 超出范围（避免万向锁）
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	// 更新视图矩阵
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


/**
 * 按键回调函数
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_LEFT_ALT:
			glfwSetCursorPosCallback(window, nullptr);// 取消视角旋转
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// 恢复鼠标指针
			glfwSetKeyCallback(window, nullptr);// 取消视角平移
			glfwSetScrollCallback(window, nullptr);// 取消滚轮视野变换
			break;
		default:
			break;
		}
	}
}

/**
 * @brief 鼠标按键回调函数
 *
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		firstMouse = true;
		glfwSetCursorPosCallback(window, mouse_callback);// 重启鼠标指针回调函数
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);// 隐藏鼠标指针
		glfwSetKeyCallback(window, key_callback);// 重启键盘回调函数
		glfwSetScrollCallback(window, scroll_callback);// 重启滚轮视野变换
	}
}

/**
 * @brief 滚轮回调函数
 * @param yoffset		竖直方向滚动的大小
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	const float sensitivity = 0.1f;
	// 更新视距
	if (fovy >= 1.0f && fovy <= 2.0f)
		fovy -= sensitivity * yoffset;
	if (fovy <= 1.0f)
		fovy = 1.0f;
	if (fovy >= 2.0f)
		fovy = 2.0f;
	pMat = glm::perspective(fovy, aspect, near, far);
}

void setupVertices(void) {
	// Sphere
	std::vector<int> ind = mySphere.getIndices();
	std::vector<glm::vec3> vert = mySphere.getVertices();
	std::vector<glm::vec2> tex = mySphere.getTexCoords();
	std::vector<glm::vec3> norm = mySphere.getNormals();
	std::vector<glm::vec3> tangents = mySphere.getTangents();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	std::vector<float> tgvalues;

	int numIndices = mySphere.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		pvalues.push_back((vert[ind[i]]).x);
		pvalues.push_back((vert[ind[i]]).y);
		pvalues.push_back((vert[ind[i]]).z);
		tvalues.push_back((tex[ind[i]]).s);
		tvalues.push_back((tex[ind[i]]).t);
		nvalues.push_back((norm[ind[i]]).x);
		nvalues.push_back((norm[ind[i]]).y);
		nvalues.push_back((norm[ind[i]]).z);
		tgvalues.push_back(tangents[ind[i]].x);
		tgvalues.push_back(tangents[ind[i]].y);
		tgvalues.push_back(tangents[ind[i]].z);
	}

	glGenVertexArrays(numVAOs, vao);
	//glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);// 绑定并激活vao[0]
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, tgvalues.size() * 4, &tgvalues[0], GL_STATIC_DRAW);

	//Cylinder
	ind = myCylinder.getIndices();
	vert = myCylinder.getVertices();
	tex = myCylinder.getTexCoords();
	norm = myCylinder.getNormals();

	std::vector<float> cy_pvalues;
	std::vector<float> cy_tvalues;
	std::vector<float> cy_nvalues;

	numIndices = myCylinder.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		cy_pvalues.push_back((vert[ind[i]]).x);
		cy_pvalues.push_back((vert[ind[i]]).y);
		cy_pvalues.push_back((vert[ind[i]]).z);
		cy_tvalues.push_back((tex[ind[i]]).s);
		cy_tvalues.push_back((tex[ind[i]]).t);
		cy_nvalues.push_back((norm[ind[i]]).x);
		cy_nvalues.push_back((norm[ind[i]]).y);
		cy_nvalues.push_back((norm[ind[i]]).z);
	}
	//std::cout << "aaa";
	//glGenVertexArrays(1, vao + 1);
	glBindVertexArray(vao[1]);// 绑定并激活vao[1]

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, cy_pvalues.size() * 4, &cy_pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, cy_tvalues.size() * 4, &cy_tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, cy_nvalues.size() * 4, &cy_nvalues[0], GL_STATIC_DRAW);

	//skybox
	float cubeVertexPositions[108] =// 12 triangles * 3 dims
	{ -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};
	glBindVertexArray(vao[2]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);

}

void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	lightShaderProgram = Utils::createShaderProgram("lightVert.glsl", "lightFrag.glsl");
	skyboxShaderProgram = Utils::createShaderProgram("vertCShader.glsl", "fragCShader.glsl");
	sphRenderingProgram = Utils::createShaderProgram("vertShaderSph.glsl", "fragShaderSph.glsl");
	//cameraX = 0.0f; cameraY = 0.0f; cameraZ = 3.0f;
	//sphLocX = 0.0f; sphLocY = 0.0f; sphLocZ = -1.0f;
	//cameraPos.x = 0.0f; cameraPos.y = 0.0f; cameraPos.z = 3.0f;

	// 初始化view矩阵
	/*glUseProgram(renderingProgram);
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));*/

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(fovy, aspect, near, far);

	glEnable(GL_DEPTH_TEST); //启用深度测试
	//glEnable(GL_LIGHT0);

	setupVertices();
	sphereTexture = Utils::loadTexture("download.jpg");
	cylinderTexture = Utils::loadTexture("alum.jpg");
	skyboxTexture = Utils::loadCubeMap("skybox");
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// 观察矩阵，使相机位于cameraPos，朝向Pos+Front
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);


	// draw cube map
	glUseProgram(skyboxShaderProgram);

	viewLoc = glGetUniformLocation(skyboxShaderProgram, "view_matrix");
	projLoc = glGetUniformLocation(skyboxShaderProgram, "proj_matrix");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	// 禁用深度测试
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);	
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);














	// 光源
	glUseProgram(lightShaderProgram);

	mvLoc = glGetUniformLocation(lightShaderProgram, "mv_matrix");
	projLoc = glGetUniformLocation(lightShaderProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(lightShaderProgram, "view_matrix");

	glm::mat4 lightTrans = glm::translate(glm::mat4(1.0f), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	glm::mat4 lightScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(lightTrans * lightScale));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	
	// 计算光照
	currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);
	glm::mat4 lightTransl = glm::translate(glm::mat4(1.0f), lightShift);
	//currentLightPos = glm::vec3(rMat * glm::vec4(currentLightPos, 1.0f));

	// 物体1
	matAmb = Utils::silverAmbient();
	matDif = Utils::silverDiffuse();
	matSpe = Utils::silverSpecular();
	matShi = Utils::silverShininess();
	installLights(lightTransl, renderingProgram);
	installLights(lightTransl, sphRenderingProgram);

	glUseProgram(sphRenderingProgram);

	mvLoc = glGetUniformLocation(sphRenderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(sphRenderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(sphRenderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(sphRenderingProgram, "norm_matrix");
	// view传递给着色器
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// 身体（球）
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.0));
	mMat = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, 1.0, 0.0)); //自身逆时针旋转
	glm::mat4 sMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));//放缩
	glm::mat4 upwards = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.5f, 0.0));//向上0.5
	//glm::mat4 mMat2 = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, -1.0, 0.0));//自转
	//mvMat = vMat2 * mMat * vMat * mMat2 * sMat;
	mvMat = upwards * vMat * sMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);// 0 = vertShader中 in 的下标，3 = 顶点属性的大小
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//glBindVertexArray(vao[0]); // 为什么不能多次绑定？？？
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	// 水柱
	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	// view传递给着色器
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 waterScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 2.0f));//放缩
	glm::mat4 waterVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMat = upwards * vMat * waterVMat * waterScaleMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// 关闭背景剔除，它会忽略背面（法向量朝内）的三角形
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());

	// 底座（柱）
	//glm::mat4 mMat3 = glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0));//自转
	mMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));// 绕x轴旋转90度
	glm::mat4 bodyScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.6f, 1.25f));//放缩
	glm::mat4 bodyVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
	mvMat = bodyVMat * vMat * bodyScaleMat * mMat;
	//mvMat = vMat2 * vMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// 关闭背景剔除，它会忽略背面（法向量朝内）的三角形
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());









	// 物体2
	matAmb = Utils::goldAmbient();
	matDif = Utils::goldDiffuse();
	matSpe = Utils::goldSpecular();
	matShi = Utils::goldShininess();
	installLights(lightTransl, renderingProgram);
	installLights(lightTransl, sphRenderingProgram);

	// 身体（球）
	glUseProgram(sphRenderingProgram);

	mvLoc = glGetUniformLocation(sphRenderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(sphRenderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(sphRenderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(sphRenderingProgram, "norm_matrix");

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 5.0));
	sMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));//放缩
	upwards = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.5f, 0.0));//向上0.5
	mvMat = upwards * vMat * sMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);// 0 = vertShader中 in 的下标，3 = 顶点属性的大小
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//glBindVertexArray(vao[0]); // 为什么不能多次绑定？？？
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	// 水柱
	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	waterScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 2.0f));//放缩
	waterVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	mvMat = upwards * vMat * waterVMat * waterScaleMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// 关闭背景剔除，它会忽略背面（法向量朝内）的三角形
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());

	// 底座（柱）
	mMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));// 绕x轴旋转90度
	bodyScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.6f, 1.25f));//放缩
	bodyVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
	mvMat = bodyVMat * vMat * bodyScaleMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// 关闭背景剔除，它会忽略背面（法向量朝内）的三角形
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());
}

void window_size_callback(GLFWwindow* win, int newWidth, int newHeight)
{
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	//pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);// 1.0472 rad=60 degrees
	pMat = glm::perspective(fovy, aspect, near, far);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);// 启用双缓冲

	GLFWwindow* window = glfwCreateWindow(600, 600, "YuanShen", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);

	// 注册回调函数
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);// 处理视角平移
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();

		/*GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL Error: " << error << std::endl;
		}*/
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
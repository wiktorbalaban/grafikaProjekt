#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

//obj::Model shipModel;
obj::Model fishFrontModel;
obj::Model fishBackModel;
obj::Model ogonModel;
obj::Model pletwa1Model;
obj::Model pletwa2Model;
obj::Model sphereModel;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);
glm::quat fishFrontRot = glm::quat(1, 0, 0, 0);

GLuint textureAsteroid;
GLuint fishBackTexture;
GLuint ogonTexture;
GLuint pletwa1Texture;

const int asteroidsTransSize = 10;
glm::vec3 asteroidsTrans [asteroidsTransSize];

glm::vec2 mouseOldCords;
glm::vec2 mouseDiff;

float mouseSensitivity=0.01f;

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 10.0f;
	float moveSpeed =0.1f;
	glm::quat quatZ;
	switch(key)
	{
	case 'z': 
		quatZ = glm::angleAxis(angleSpeed * mouseSensitivity, glm::vec3(0.0f, 0.0f, -1.0f));
		rotation = glm::normalize(quatZ * rotation);
		cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'x': 
		quatZ = glm::angleAxis(angleSpeed * mouseSensitivity, glm::vec3(0.0f, 0.0f, 1.0f));
		rotation = glm::normalize(quatZ * rotation);
		cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	mouseDiff.x += mouseOldCords.x - x;
	mouseDiff.y += mouseOldCords.y - y;
	mouseOldCords.x = x;
	mouseOldCords.y = y;
}

glm::mat4 createCameraMatrix()
{
	/*cameraDir = glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0.0f, sinf(cameraAngle - glm::radians(90.0f)));
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrix(cameraPos, cameraDir, up);*/
	/* Poruszanie statkiem myszk¹*/
	glm::quat quatX = glm::angleAxis(mouseDiff.y * mouseSensitivity, glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::quat quatY = glm::angleAxis(mouseDiff.x * mouseSensitivity, glm::vec3(0.0f, -1.0f, 0.0f));
	glm::quat rotationChange = quatY*quatX;
	mouseDiff.x = 0.0f;
	mouseDiff.y = 0.0f;
	rotation = glm::normalize(rotationChange * rotation);
	cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
	return Core::createViewMatrixQuat(cameraPos, rotation);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
glm::mat4 rotate(float angle, glm::vec3 axis, glm::vec3 position) {
	return glm::translate(position) * glm::rotate(glm::radians(angle), axis) * glm::translate(-position);
}
void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	//glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0,-0.25f,0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	//glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * /*glm::mat4_cast(glm::inverse(rotation)) * */ shipInitialTransformation;
	//drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	float fishInitRotY = 180.0f;
	float fishInitRotX = 90.0f;
	float przesunX = 0.3f;
	float time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	float fishFrontRadius = 4.0f;
	float pletwaRadius = 15.0f;


	glm::mat4 fishFrontInitialTransformation = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(fishInitRotX), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(fishInitRotY), glm::vec3(0, 1, 0)) 
		* glm::translate(glm::vec3(0, 0, 0.3f*0.25f)) * glm::rotate(glm::radians(sinf(time)*fishFrontRadius), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(0, 0, -0.3f*0.25f)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 fishFrontModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * fishFrontInitialTransformation;
	drawObjectTexture(&fishFrontModel, fishFrontModelMatrix, fishBackTexture);

	glm::mat4 fishBackInitialTransformation = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(fishInitRotX), glm::vec3(1, 0, 0))  * glm::rotate(glm::radians(fishInitRotY), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 fishBackModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * fishBackInitialTransformation;
	drawObjectTexture(&fishBackModel, fishBackModelMatrix, fishBackTexture);

	glm::mat4 ogonInitialTransformation = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(fishInitRotX), glm::vec3(1, 0, 0))  * glm::rotate(glm::radians(fishInitRotY), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 ogonModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * ogonInitialTransformation;
	drawObjectTexture(&ogonModel, ogonModelMatrix, ogonTexture);

	glm::mat4 pletwa1InitialTransformation = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(fishInitRotX), glm::vec3(1, 0, 0))  * glm::rotate(glm::radians(fishInitRotY), glm::vec3(0, 1, 0)) 
		* glm::translate(glm::vec3(0, 0, 0.3f*0.25f)) * glm::rotate(glm::radians(sinf(time)*fishFrontRadius), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(0, 0, -0.3f*0.25f))
		* glm::translate(glm::vec3(0.15802f*0.25f, 0.01582*0.25f, 0.0f)) * glm::rotate(glm::radians(sinf(time)*pletwaRadius+15.0f), glm::vec3(0, 0, 1)) * glm::translate(glm::vec3(-0.15802f*0.25f, -0.01582*0.25f, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 pletwa1ModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * pletwa1InitialTransformation;
	drawObjectTexture(&pletwa1Model, pletwa1ModelMatrix, pletwa1Texture);

	glm::mat4 pletwa2InitialTransformation = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(fishInitRotX), glm::vec3(1, 0, 0))  * glm::rotate(glm::radians(fishInitRotY), glm::vec3(0, 1, 0)) 
		* glm::translate(glm::vec3(0, 0, 0.3f*0.25f)) * glm::rotate(glm::radians(sinf(time)*fishFrontRadius), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(0, 0, -0.3f*0.25f))
		* glm::translate(glm::vec3(-0.15802f*0.25f, -0.01582*0.25f, 0.0f)) * glm::rotate(glm::radians(-sinf(time)*pletwaRadius - 15.0f), glm::vec3(0, 0, 1)) * glm::translate(glm::vec3(0.15802f*0.25f, 0.01582*0.25f, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 pletwa2ModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * pletwa2InitialTransformation;
	drawObjectTexture(&pletwa2Model, pletwa2ModelMatrix, pletwa1Texture);


	for (int i = 0; i < asteroidsTransSize; i++) {
		drawObjectTexture(&sphereModel, glm::translate(asteroidsTrans[i]), textureAsteroid);
	}

	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	fishFrontModel = obj::loadModelFromFile("models/fish/fishFront.obj");
	fishBackModel = obj::loadModelFromFile("models/fish/fishBack.obj");
	ogonModel = obj::loadModelFromFile("models/fish/ogon.obj");
	pletwa1Model = obj::loadModelFromFile("models/fish/pletwa1.obj");
	pletwa2Model = obj::loadModelFromFile("models/fish/pletwa2.obj");
	fishBackTexture = Core::LoadTexture("textures/fish/sphere7_auv.png");
	ogonTexture = Core::LoadTexture("textures/fish/octahedron1_auv.png");
	pletwa1Texture = Core::LoadTexture("textures/fish/cone3_auv.png");
	textureAsteroid = Core::LoadTexture("textures/fish/sphere7_auv.png");
	for (int i = 0; i < asteroidsTransSize; i++) {
		asteroidsTrans[i] = glm::ballRand(20.0f);
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Shader.h"

using namespace glm;

struct cs_uniforms {

	GLuint vec4_startFinish, uvec2_samples, uint_seed;

	vec4 startFinish;
	uvec2 samples;
	uint seed;

	void getLocations(ComputeShaderProgram& program) {
		vec4_startFinish = program.getUniformLocation("startFinish");
		uvec2_samples = program.getUniformLocation("samples");
		uint_seed = program.getUniformLocation("seed");
	}

	void set() {
		glUniform4f(vec4_startFinish, startFinish.x, startFinish.y, startFinish.z, startFinish.w);
		glUniform2ui(uvec2_samples, samples.x, samples.y);
		glUniform1ui(uint_seed, seed);
	}

};

struct vs_uniforms {

	GLuint vec4_startFinish, uvec2_samples, uvec2_meshSize, mat4_view, mat4_proj, vec3_pos;

	vec4 startFinish;
	uvec2 samples;
	uvec2 meshSize;
	mat4 view;
	mat4 proj;
	vec3 pos;

	void getLocations(ShaderProgram& program) {
		vec4_startFinish = program.getUniformLocation("startFinish");
		uvec2_samples = program.getUniformLocation("samples");
		uvec2_meshSize = program.getUniformLocation("meshSize");
		mat4_view = program.getUniformLocation("view");
		mat4_proj = program.getUniformLocation("proj");
		vec3_pos = program.getUniformLocation("pos");
	}

	void set() {
		glUniform4f(vec4_startFinish, startFinish.x, startFinish.y, startFinish.z, startFinish.w);
		glUniform2ui(uvec2_samples, samples.x, samples.y);
		glUniform2ui(uvec2_meshSize, meshSize.x, meshSize.y);
		glUniformMatrix4fv(mat4_view, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(mat4_proj, 1, GL_FALSE, value_ptr(proj));
		glUniform3f(vec3_pos, pos.x, pos.y, pos.z);
	}

};

mat4 getViewMatrix(vec3 camPos, vec3 camDir) {
	return lookAt(camPos, camPos + camDir, vec3(0.0f, 1.0f, 0.0f));
}

int main() {

	if (!glfwInit()) {

		std::cout << "Failed to initialize GLFW.\n";
		return -1;

	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const GLFWvidmode* displayInfo = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int screenWidth = displayInfo->width; int screenHeight = displayInfo->height;
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Dear ImGui GLFW+OpenGL3 example", glfwGetPrimaryMonitor(), NULL);
	if (!window) {

		std::cout << "Failed to create GLFW window.\n";
		glfwTerminate();
		return -1;

	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {

		std::cout << "Failed to load OpenGL extensions.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;

	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.7f, 0.4f, 0.3f, 1.0f);
	glViewport(0, 0, screenWidth, screenHeight);

	ComputeShaderProgram heightMap("shader.comp");
	if (!heightMap.success()) {

		std::cout << heightMap.getInfoLog();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;

	}

	heightMap.use();
	cs_uniforms csUniforms;
	csUniforms.startFinish = vec4(-7.5f, -7.5f, 7.5f, 7.5f);
	csUniforms.samples = uvec2(1000, 4000);
	csUniforms.seed = 0;
	csUniforms.getLocations(heightMap);
	csUniforms.set();

	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * csUniforms.samples.x * csUniforms.samples.y, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	
	heightMap.compute(csUniforms.samples.x, csUniforms.samples.y);

	ShaderProgram program("shader.vs", "shader.fs");
	if (!program.success()) {

		std::cout << program.getInfoLog();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;

	}

	program.use();
	vs_uniforms vsUniforms;
	vsUniforms.getLocations(program);

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);

	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 camPos(0.0f, 3.0f, 0.0f);
	vec3 camPosOld = camPos;
	vec3 camVel(0.0f, 0.0f, 0.0f);
	vec3 camDir;
	vec2 camRot(-90.0f, 0.0f);
	vec2 camRotVel(0.0f, 0.0f);
	mat4 proj = perspective(45.0f, (float)screenWidth / (float)screenHeight, 0.1f, 12.5f);
	mat4 view;
	float camVelMultiplier = 0.0025f;
	float camVelDecay = 0.9f;
	float camRotVelMultiplier = 0.13f;
	float camRotVelDecay = 0.7f;

	vsUniforms.startFinish = vec4(-15.0f, -15.0f, 15.0f, 15.0f);
	vsUniforms.samples = csUniforms.samples;
	vsUniforms.meshSize = uvec2(1000, 1000);
	vsUniforms.proj = proj;

	int numPoints = 6 * (vsUniforms.meshSize.x - 1) * (vsUniforms.meshSize.y - 1);

	double mx, my, mxLast = 0.0, myLast = 0.0;
	bool first = true;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	bool cursor = false;
	bool mPressed = false, mPressedLast = false;

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

		mPressed = glfwGetKey(window, GLFW_KEY_M);
		if (mPressed && !mPressedLast) {
			glfwSetInputMode(window, GLFW_CURSOR, (cursor) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
			cursor = !cursor;
		}
		mPressedLast = mPressed;

		glfwGetCursorPos(window, &mx, &my);
		if (first) first = false;
		else if (!cursor) {
			camRotVel.x += (mx - mxLast) * camRotVelMultiplier;
			camRotVel.y += (myLast - my) * camRotVelMultiplier;
		}
		camRotVel *= camRotVelDecay; camRot += camRotVel;
		if (camRot.y > 80.0f) camRot.y = 80.0f;
		if (camRot.y < -80.0f) camRot.y = -80.0f;
		mxLast = mx; myLast = my;

		camDir.x = cos(glm::radians(camRot.x)) * cos(glm::radians(camRot.y));
		camDir.y = sin(glm::radians(camRot.y));
		camDir.z = sin(glm::radians(camRot.x)) * cos(glm::radians(camRot.y));

		if (glfwGetKey(window, GLFW_KEY_W)) camVel += normalize(camDir * (1.0f - up)) * camVelMultiplier;
		if (glfwGetKey(window, GLFW_KEY_S)) camVel -= normalize(camDir * (1.0f - up)) * camVelMultiplier;
		if (glfwGetKey(window, GLFW_KEY_D)) camVel += normalize(cross(camDir, up)) * camVelMultiplier;
		if (glfwGetKey(window, GLFW_KEY_A)) camVel -= normalize(cross(camDir, up)) * camVelMultiplier;
		if (glfwGetKey(window, GLFW_KEY_E)) camVel += up * camVelMultiplier;
		if (glfwGetKey(window, GLFW_KEY_Q)) camVel -= up * camVelMultiplier;
		camVel *= camVelDecay; camPos += camVel;

		if (abs(camPos.x - camPosOld.x) >= 2.5f || abs(camPos.z - camPosOld.z) >= 2.5f) {

			vec4 swizzle = vec4(camPos.x, camPos.z, camPos.x, camPos.z);
			vsUniforms.startFinish = vec4(-15.0f, -15.0f, 15.0f, 15.0f) + swizzle;
			
			heightMap.use();
			csUniforms.startFinish = vec4(-7.5f, -7.5f, 7.5f, 7.5f) + swizzle / 2.0f;
			csUniforms.set();
			heightMap.compute(csUniforms.samples.x, csUniforms.samples.y);

			camPosOld = camPos;

		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();
		vsUniforms.view = getViewMatrix(camPos, camDir);
		vsUniforms.pos = camPos;
		vsUniforms.set();
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, numPoints);

		glfwSwapBuffers(window);

	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;

}

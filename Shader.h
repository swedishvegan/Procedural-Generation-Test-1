#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <fstream>

class ShaderProgram {
private:

	char* vertexShaderCode;
	char* fragmentShaderCode;
	char* geometryShaderCode;
	int vertexFileLength;
	int fragmentFileLength;
	int geometryFileLength;
	GLuint program;

	bool hasErrors;
	char infoLog[1024];

	void checkIfShaderCompiled(unsigned int shader) {

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		hasErrors = hasErrors || !success;

	}

	void checkIfProgramLinked() {

		int success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (!success) glGetProgramInfoLog(program, 1024, NULL, infoLog);
		else infoLog[0] = '\0';

		hasErrors = hasErrors || !success;

	}
	
public:

	ShaderProgram() {};

	void loadFromFiles(const char* vertex_shader_path, const char* fragment_shader_path, const char* geometry_shader_path = nullptr) {

		hasErrors = false;

		std::ifstream vertexShaderFile, fragmentShaderFile;
		vertexShaderFile.open(vertex_shader_path, std::ios::binary);
		fragmentShaderFile.open(fragment_shader_path, std::ios::binary);

		vertexShaderFile.seekg(0, std::ios::end);
		fragmentShaderFile.seekg(0, std::ios::end);
		vertexFileLength = vertexShaderFile.tellg();
		fragmentFileLength = fragmentShaderFile.tellg();

		vertexShaderCode = new char[vertexFileLength + 1];
		fragmentShaderCode = new char[fragmentFileLength + 1];
		vertexShaderFile.seekg(0, std::ios::beg);
		fragmentShaderFile.seekg(0, std::ios::beg);
		vertexShaderFile.read(vertexShaderCode, vertexFileLength);
		fragmentShaderFile.read(fragmentShaderCode, fragmentFileLength);
		vertexShaderCode[vertexFileLength] = '\0';
		fragmentShaderCode[fragmentFileLength] = '\0';

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
		glCompileShader(vertexShader);
		checkIfShaderCompiled(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		checkIfShaderCompiled(fragmentShader);

		GLuint geometryShader;
		std::ifstream geometryShaderFile;
		if (geometry_shader_path != nullptr) {

			geometryShaderFile.open(geometry_shader_path, std::ios::binary);

			geometryShaderFile.seekg(0, std::ios::end);
			geometryFileLength = geometryShaderFile.tellg();

			geometryShaderCode = new char[geometryFileLength + 1];
			geometryShaderFile.seekg(0, std::ios::beg);
			geometryShaderFile.read(geometryShaderCode, geometryFileLength);
			geometryShaderCode[geometryFileLength] = '\0';

			geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometryShader, 1, &geometryShaderCode, NULL);
			glCompileShader(geometryShader);
			checkIfShaderCompiled(geometryShader);

		}

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		if (geometry_shader_path != nullptr) glAttachShader(program, geometryShader);
		glLinkProgram(program);
		checkIfProgramLinked();

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		delete[] vertexShaderCode;
		delete[] fragmentShaderCode;

		vertexShaderFile.close();
		fragmentShaderFile.close();

		if (geometry_shader_path != nullptr) {
			glDeleteShader(geometryShader);
			delete[] geometryShaderCode;
			geometryShaderFile.close();
		}

	}

	void loadFromStrings(const char* vertex_shader_source, const char* fragment_shader_source, const char* geometry_shader_source = nullptr) {

		hasErrors = false;

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
		glCompileShader(vertexShader);
		checkIfShaderCompiled(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
		glCompileShader(fragmentShader);
		checkIfShaderCompiled(fragmentShader);

		GLuint geometryShader;
		if (geometry_shader_source != nullptr) {

			geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometryShader, 1, &geometry_shader_source, NULL);
			glCompileShader(geometryShader);
			checkIfShaderCompiled(geometryShader);

		}

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		if (geometry_shader_source != nullptr) glAttachShader(program, geometryShader);
		glLinkProgram(program);
		checkIfProgramLinked();

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		delete[] vertexShaderCode;
		delete[] fragmentShaderCode;

		if (geometry_shader_source != nullptr) {
			glDeleteShader(geometryShader);
			delete[] geometryShaderCode;
		}

	}

	ShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, const char* geometry_shader_path = nullptr) {
		loadFromFiles(vertex_shader_path, fragment_shader_path, geometry_shader_path);
	}

	void use() {
		glUseProgram(program);
	}

	GLuint getUniformLocation(const char* name) {
		return glGetUniformLocation(program, name);
	}

	GLuint programID() {
		return program;
	}

	char* getInfoLog() {
		return infoLog;
	}

	bool success() {
		return !hasErrors;
	}

};

class ComputeShaderProgram {
private:

	std::ifstream file;
	char* code;
	int length;
	GLuint program;

	bool hasErrors;
	char infoLog[1024];

	void checkIfShaderCompiled(unsigned int shader) {

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		hasErrors = hasErrors || !success;

	}

	void checkIfProgramLinked() {

		int success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (!success) glGetProgramInfoLog(program, 1024, NULL, infoLog);
		else infoLog[0] = '\0';

		hasErrors = hasErrors || !success;

	}

public:

	ComputeShaderProgram() {};

	ComputeShaderProgram(const char* path) {

		hasErrors = false;

		file.open(path, std::ios::binary);

		file.seekg(0, std::ios::end);
		length = file.tellg();

		code = new char[length + 1];
		file.seekg(0, std::ios::beg);
		file.read(code, length);
		code[length] = '\0';

		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &code, NULL);
		glCompileShader(shader);
		checkIfShaderCompiled(shader);

		program = glCreateProgram();

		glAttachShader(program, shader);
		glLinkProgram(program);
		checkIfProgramLinked();

		glDeleteShader(shader);
		delete[] code;
		file.close();

	}

	void use() {
		glUseProgram(program);
	}

	void compute(GLuint x_groups, GLuint y_groups = 1, GLuint z_groups = 1, bool memory_barrier = true) {

		glDispatchCompute(x_groups, y_groups, z_groups);
		if (memory_barrier) glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	}

	GLuint getUniformLocation(const char* name) {
		return glGetUniformLocation(program, name);
	}

	GLuint programID() {
		return program;
	}

	char* getInfoLog() {
		return infoLog;
	}

	bool success() {
		return !hasErrors;
	}

	int xMaxWorkGroupSize() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &n);
		return n;

	}
	int yMaxWorkGroupSize() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &n);
		return n;

	}
	int zMaxWorkGroupSize() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &n);
		return n;

	}

	int xMaxWorkGroupCount() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &n);
		return n;

	}
	int yMaxWorkGroupCount() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &n);
		return n;

	}
	int zMaxWorkGroupCount() {

		int n;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &n);
		return n;

	}

};
// example2.cpp

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "GL/glew.h"
#include "GL/glut.h"
#include "GL/gl.h"

#include <expressions/expressions.h>
#include <expressions/Exception.h>


const std::string fragHead(
	"#version 330\n"
	"uniform float value;\n"
	"uniform float x;\n"
	"uniform float y;\n"
	"uniform float pi;\n"
	"out vec4 outputColor;\n"
	"\n"
);

const std::string fragMain(
	"void main()\n"
	"{\n"
	"    if (value == calculate())\n"
	"    {\n"
	"         outputColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
	"    }\n"
	"    else\n"
	"    {\n"
	"         outputColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
	"    }\n"
	"}\n"
);

const float vertexPositions[] =
{
		0.75f, 0.75f, 0.0f, 1.0f,
		0.75f, -0.75f, 0.0f, 1.0f,
		-0.75f, -0.75f, 0.0f, 1.0f,
};

GLuint program;
GLuint positionBuffer;


GLuint CreateShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch(eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		std::cerr << "Compile failure in " << strShaderType << "shader: " << strInfoLog << std::endl;
		delete[] strInfoLog;
	}

	return shader;
}


GLuint CreateProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		std::cerr << "Linker failure: " << strInfoLog << std::endl;
		delete[] strInfoLog;
	}

	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}


GLuint CompileProgram(std::string fragmentShader)
{
	std::vector<GLuint> shaderList;

	shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, fragmentShader));

	GLuint program = CreateProgram(shaderList);

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	return program;
}


void initialize(int argc, char *argv[])
{
		glutInit(&argc, argv);
		glutCreateWindow("Shader compilation test");
		glewExperimental=true;
		GLenum err=glewInit();
		if(err!=GLEW_OK)
		{
			//Problem: glewInit failed, something is seriously wrong.
			throw expr::Exception("glewInit failed, aborting.");
		}
}


void render()
{
	glClearColor(0.0f, 0.0f,0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
	initialize(argc, argv);

	// Expression to evaluate
	const char * expression = "min(y,8) < max(y,8) && x % y == 2 ? (ceil(cos(60*pi/180) + sin(30*pi/180) + tan(45*pi/180)) + sqrt(floor(16.5)) + log2(16)) * log10(100) : 0";

	// Construct the AST from the given expression
	expr::Parser<float> parser; 
	expr::Evaluator<float>::VariableMap vm; vm["pi"] = 3.14159; vm["x"] = 10; vm["y"] = 4;

	expr::ASTNode* ast = parser.parse(expression);
	float value = expr::Evaluator<float>(ast, &vm).evaluate();
	std::cout << "Output value is: " << value << std::endl << std::endl;

	// Generate the GLSL fragment shader code
	expr::ShaderGenerator<float> generator;
	std::string shader;
	shader += fragHead;
	shader += "float calculate()\n{\n\treturn ";
	shader += generator.generate(ast);
	shader += ";\n}\n";
	shader += std::string("\n") += fragMain;

	std::cout << shader << std::endl;


	// Compile the shader
	program = CompileProgram(shader);

	// Assign the uniform value in the shader to the output of the CPU evaluation for comparison
	glUseProgram(program);
	GLuint valueLocation = glGetUniformLocation(program, "value");
	glUniform1f(valueLocation, value);
	glUseProgram(0);

	// Assign uniform values for each of the values in variablemap
	glUseProgram(program);
	for (expr::Parser<float>::VariableMap::iterator it=vm.begin(); it!= vm.end(); it++)
	{
		GLuint loc = glGetUniformLocation(program, it->first.c_str());
		glUniform1f(loc, it->second);
	}
	glUseProgram(0);


	// Make a pretty triangle for us to see the color on
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// render
	glutDisplayFunc(render);
	glutMainLoop();

	return 0;
}

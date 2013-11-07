#ifndef _LIGHT_H_
#define _LIGHT_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "model.hpp"

class Light {
	private:
		GLuint id;
		GLfloat position[4];
		GLfloat ambient[4];
		GLfloat color[4];
	public:
		Light(GLuint id);
		Light();
		void setPosition(float x, float y, float z);
		void setAmbient(float r, float g, float b);
		void setColor(float r, float g, float b);
		void use();
		Vec3d getPos();
};

#endif
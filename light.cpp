#include "light.hpp"

Light::Light(GLuint id) {this->id=id;}
Light::Light() {id=GL_LIGHT0;}
void Light::setPosition(float x, float y, float z) {
	position[0]=x;
	position[1]=y;
	position[2]=z;
	position[3]=1.0f;
}
void Light::setAmbient(float r, float g, float b) {
	ambient[0]=r;
	ambient[1]=g;
	ambient[2]=b;
	ambient[3]=1.0f;
	glLightfv(id, GL_AMBIENT, ambient);
}
void Light::setColor(float r, float g, float b) {
	color[0]=r;
	color[1]=g;
	color[2]=b;
	color[3]=1.0f;
	glLightfv(id, GL_DIFFUSE, color);
	glLightfv(id, GL_SPECULAR, color);
}
void Light::use() {
	glEnable(id);
	glLightfv(id, GL_POSITION, position);
}

Vec3d Light::getPos() {
	return Vec3d(position[0],position[1],position[2]);
}
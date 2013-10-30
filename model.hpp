#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>

// C++11 Support
#if __cplusplus <= 199711L
	#include <map>
	#define hashmap map
#else
	#include <unordered_map>
	#define hashmap unordered_map	
#endif
#include <vector>
#include <string>
#include <iostream>
//#include "devil_cpp_wrapper.hpp"
using namespace std;

#define USING_VBO true

struct VBO {
	GLuint id;
	int elems;
	int vertPerFace;
	vector<double> data;
	string material;
};

struct Texture {
	GLuint id;
	unsigned char* image;
	Texture(string filename);
	Texture();
};

struct Vec3d {
	double x;
	double y;
	double z;
	Vec3d(double x=0.0,double y=0.0,double z=0.0) {
		this->x=x;this->y=y;this->z=z;
	}
};

struct Tri {
	Vec3d vertex[3];
	Vec3d normal[3];
	Vec3d uvw[3];
	string material;
};

struct Quad {
	Vec3d vertex[4];
	Vec3d normal[4];
	Vec3d uvw[4];
	string material;
};

struct Poly {
	vector<Vec3d> vertex;
	vector<Vec3d> normal;
	vector<Vec3d> uvw;
	string material;
};

struct Material {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float transmittance[4];
	float emission[4];
	float shininess[1];
	Material() {
		ambient[0]  = 0.7;
		ambient[1]  = 0.7;
		ambient[2]  = 0.7;
		ambient[3]  = 1.0;
		diffuse[0]  = 0.9;
		diffuse[1]  = 0.9;
		diffuse[2]  = 0.9;
		diffuse[3]  = 1.0;
		specular[0]  = 0.3;
		specular[1]  = 0.3;
		specular[2]  = 0.3;
		specular[3]  = 1.0;
		shininess[0] = 0.5;
		hasTexture=false;
	}
	bool hasTexture;
	Texture texture;
};

class Model {
	private:
		hashmap<string,Material> materials;
		string lastUsedMaterial;
		vector<Tri> tris;
		vector<Quad> quads;
		vector<Poly> polys;
		vector<VBO> vbos;
	public:
		Model();
		Model(string filename);
		void applyMaterial(string name);
		void render();
		void addMaterial(Material m);
		void addMaterial(string name, Material m);
		void addTri(Tri t);
		void addQuad(Quad q);
		void addPoly(Poly p);
		void generateVBOs();
};

Model newHole(int sides,double radius,double depth);

#endif

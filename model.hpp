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
#include <cmath>
#include "bullet/include/btBulletDynamicsCommon.h"
using namespace std;

#define USING_VBO 1
#define COMPARE_DELTA 0.025
#define SHADOW_EXT 5

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
	bool operator== (const Vec3d &b) const {
		return fabs(this->x-b.x)<COMPARE_DELTA && fabs(this->y-b.y)<COMPARE_DELTA && fabs(this->z-b.z)<COMPARE_DELTA;
	}
	double operator* (Vec3d b) {
		return this->x*b.x + this->y*b.y + this->z*b.z;
	}
	bool operator< (const Vec3d &b) const {
		if (this->x == b.x) {
			if (this->y == b.y) {
				if (this->z == b.z) {
					return false;
				}
				return this->z < b.z;
			}
			return this->y < b.y;
		}
		return this->x < b.x;
	}
	Vec3d transform(GLfloat *M) const {
		GLfloat res[4];
	    res[0]=M[ 0]*this->x+M[ 4]*this->y+M[ 8]*this->z+M[12]*1;
	    res[1]=M[ 1]*this->x+M[ 5]*this->y+M[ 9]*this->z+M[13]*1;
	    res[2]=M[ 2]*this->x+M[ 6]*this->y+M[10]*this->z+M[14]*1;
	    //res[3]=M[ 3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];
	    return Vec3d(res[0],res[1],res[2]);
	}
};


struct Tri {
	Vec3d vertex[3];
	Vec3d normal[3];
	Vec3d uvw[3];
	Vec3d fNormal;
	string material;
};

struct Quad {
	Vec3d vertex[4];
	Vec3d normal[4];
	Vec3d uvw[4];
	Vec3d fNormal;
	string material;
};

struct Poly {
	vector<Vec3d> vertex;
	vector<Vec3d> normal;
	vector<Vec3d> uvw;
	Vec3d fNormal;
	string material;
};

struct Edge
{
	Vec3d vert1;
	Vec3d vert2;
	size_t poly1Index;
	size_t poly2Index;
	
	int vertF1;
	int vertF2;
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
		vector<Edge> edges;
		vector<VBO> vbos;
	public:
		Model();
		Model(string filename);
		void applyMaterial(string name);
		void render();
		void renderShadow(Vec3d lightPos);
		void addMaterial(Material m);
		void addMaterial(string name, Material m);
		void addTri(Tri t);
		void addQuad(Quad q);
		void addPoly(Poly p);
		void generateVBOs();
		void generateEdges();
		static float cameraMatrix[16];
};

Model newHole(int sides,double radius,double depth);

#endif

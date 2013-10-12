#ifndef _MODEL_H_
#define _MODEL_H_
#include <vector>
#include <string>
using namespace std;

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
};

struct Quad {
	Vec3d vertex[4];
	Vec3d normal[4];
};

struct Poly {
	vector<Vec3d> vertex;
	vector<Vec3d> normal;
};

struct Material {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float shine[1];
};

class Model {
	public:
		Material mat;
		vector<Tri> tris;
		vector<Quad> quads;
		vector<Poly> polys;
	//public:
		Model();
		Model(string filename);
		void applyMaterial();
		void render();
		void setMaterial(Material m);
		void addTri(Tri t);
		void addQuad(Quad q);
		void addPoly(Poly p);
};

Model newHole(int sides,double radius,double depth);

#endif

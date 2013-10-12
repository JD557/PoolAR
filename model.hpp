#ifndef _MODEL_H_
#define _MODEL_H_

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
	string material;
};

struct Quad {
	Vec3d vertex[4];
	Vec3d normal[4];
	string material;
};

struct Poly {
	vector<Vec3d> vertex;
	vector<Vec3d> normal;
	string material;
};

struct Material {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float transmittance[4];
	float emission[4];
	float shininess[1];
};

class Model {
	private:
		hashmap<string,Material> materials;
		string lastUsedMaterial;
		vector<Tri> tris;
		vector<Quad> quads;
		vector<Poly> polys;
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
};

Model newHole(int sides,double radius,double depth);

#endif

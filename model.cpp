#include "model.hpp"
#include "tiny_obj_loader.hpp"
#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace tinyobj;

Model::Model() {
	addMaterial("default",Material());
}

Model::Model(string filename) {
	addMaterial("default",Material());

	string folder="";
	size_t lastSlash=filename.find_last_of('/');
	if (lastSlash!=string::npos) {
		folder=filename.substr(0,lastSlash+1);
	}
	vector<shape_t> shapes;
	std::cout << LoadObj(shapes,filename.c_str(),folder.c_str()) << std::endl;
	for (size_t i=0;i<shapes.size();++i) {
		Material tempMaterial;
		for (int j=0;j<3;++j) {
			tempMaterial.ambient[j]=shapes[i].material.ambient[j];
			tempMaterial.diffuse[j]=shapes[i].material.diffuse[j];
			tempMaterial.specular[j]=shapes[i].material.specular[j];
			tempMaterial.emission[j]=shapes[i].material.emission[j];
			tempMaterial.transmittance[j]=shapes[i].material.transmittance[j];
		}
		tempMaterial.ambient[3]=1.0f;
		tempMaterial.diffuse[3]=1.0f;
		tempMaterial.specular[3]=1.0f;
		tempMaterial.emission[3]=1.0f;
		tempMaterial.transmittance[3]=1.0f;
		tempMaterial.shininess[0]=shapes[i].material.shininess;

		addMaterial(shapes[i].material.name,tempMaterial);

		int numVert = shapes[i].mesh.positions.size()/3;
		if (numVert==3) {
			Tri t;
			t.material=shapes[i].material.name;

			t.normal[0]=Vec3d(0.0,0.0,1.0);
			t.vertex[0].x=shapes[i].mesh.positions[0];
			t.vertex[0].y=shapes[i].mesh.positions[1];
			t.vertex[0].z=shapes[i].mesh.positions[2];

			t.normal[1]=Vec3d(0.0,0.0,1.0);
			t.vertex[1].x=shapes[i].mesh.positions[3];
			t.vertex[1].y=shapes[i].mesh.positions[4];
			t.vertex[1].z=shapes[i].mesh.positions[5];

			t.normal[2]=Vec3d(0.0,0.0,1.0);
			t.vertex[2].x=shapes[i].mesh.positions[6];
			t.vertex[2].y=shapes[i].mesh.positions[7];
			t.vertex[2].z=shapes[i].mesh.positions[8];
			tris.push_back(t);
		}
		else if (numVert==4) {
			Quad q;
			q.material=shapes[i].material.name;

			q.normal[0]=Vec3d(0.0,0.0,1.0);
			q.vertex[0].x=shapes[i].mesh.positions[0];
			q.vertex[0].y=shapes[i].mesh.positions[1];
			q.vertex[0].z=shapes[i].mesh.positions[2];

			q.normal[1]=Vec3d(0.0,0.0,1.0);
			q.vertex[1].x=shapes[i].mesh.positions[3];
			q.vertex[1].y=shapes[i].mesh.positions[4];
			q.vertex[1].z=shapes[i].mesh.positions[5];

			q.normal[2]=Vec3d(0.0,0.0,1.0);
			q.vertex[2].x=shapes[i].mesh.positions[6];
			q.vertex[2].y=shapes[i].mesh.positions[7];
			q.vertex[2].z=shapes[i].mesh.positions[8];

			q.normal[3]=Vec3d(0.0,0.0,0.0);
			q.vertex[3].x=shapes[i].mesh.positions[9];
			q.vertex[3].y=shapes[i].mesh.positions[10];
			q.vertex[3].z=shapes[i].mesh.positions[11];
			quads.push_back(q);
		}
		else {
			Poly p;
			p.material=shapes[i].material.name;

			for (int j=0;j<numVert;++j) {
				p.normal.push_back(Vec3d(0.0,0.0,1.0));
				p.vertex.push_back(Vec3d(
					shapes[i].mesh.positions[3*j],
					shapes[i].mesh.positions[3*j+1],
					shapes[i].mesh.positions[3*j+2]
				));
			}
			polys.push_back(p);
		}
	}
}

void Model::applyMaterial(string name) {
	if (name=="") {name="default";}
	if (name==lastUsedMaterial) {return;}
	Material mat = materials[name];
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat.shininess);
}

void Model::render() {
	lastUsedMaterial="";
	// Render tris
	glBegin(GL_TRIANGLES);
	for (int i=0;i<tris.size();++i) {
		applyMaterial(tris[i].material);
		for (int j=0;j<3;++j) {
			glNormal3f(tris[i].normal[j].x,tris[i].normal[j].y,tris[i].normal[j].z);
			glVertex3f(tris[i].vertex[j].x,tris[i].vertex[j].y,tris[i].vertex[j].z);
		}
	}
	glEnd();

	// Render quads
	glBegin(GL_QUADS);
	for (int i=0;i<quads.size();++i) {
		applyMaterial(quads[i].material);
		for (int j=0;j<4;++j) {
			glNormal3f(quads[i].normal[j].x,quads[i].normal[j].y,quads[i].normal[j].z);
			glVertex3f(quads[i].vertex[j].x,quads[i].vertex[j].y,quads[i].vertex[j].z);
		}
	}
	glEnd();

	// Render polys
	for (int i=0;i<polys.size();++i) {
		applyMaterial(polys[i].material);
		glBegin(GL_POLYGON);
		for (int j=0;j<polys[i].vertex.size();++j) {
			glNormal3f(polys[i].normal[j].x,polys[i].normal[j].y,polys[i].normal[j].z);
			glVertex3f(polys[i].vertex[j].x,polys[i].vertex[j].y,polys[i].vertex[j].z);
		}
		glEnd();
	}
}


void Model::addMaterial(Material m) {
	materials["default"]=m;
}
void Model::addMaterial(string name, Material m) {
	materials[name]=m;
}
void Model::addTri(Tri t) {tris.push_back(t);}
void Model::addQuad(Quad q) {quads.push_back(q);}
void Model::addPoly(Poly p) {polys.push_back(p);}

Model newHole(int sides,double radius,double depth) {
	Model m;
	Poly p;
	for (int i=0;i<sides;++i) {
		double angle=2.0*3.14159*(double)i/(double)sides;
		double nextangle=2.0*3.14159*(double)(i+1)/(double)sides;
		Quad q;
		q.normal[0]=Vec3d(-cos(angle),-sin(angle),0.0);
		q.vertex[0]=Vec3d(radius*cos(angle),radius*sin(angle),0.0);

		q.normal[1]=Vec3d(-cos(angle),-sin(angle),0.0);
		q.vertex[1]=Vec3d(radius*cos(angle),radius*sin(angle),-1.0*depth);

		q.normal[2]=Vec3d(-cos(nextangle),-sin(nextangle),0.0);
		q.vertex[2]=Vec3d(radius*cos(nextangle),radius*sin(nextangle),-1.0*depth);

		q.normal[3]=Vec3d(-cos(nextangle),-sin(nextangle),0.0);
		q.vertex[3]=Vec3d(radius*cos(nextangle),radius*sin(nextangle),0.0);
				
		p.normal.push_back(Vec3d(0.0,0.0,1.0));
		p.vertex.push_back(Vec3d(radius*cos(-angle),radius*sin(-angle),-1.0*depth));

		m.addQuad(q);
	}
	m.addPoly(p);

	Material mat;
	mat.ambient[0]  = 0.7;
	mat.ambient[1]  = 0.7;
	mat.ambient[2]  = 0.7;
	mat.ambient[3]  = 1.0;
	mat.diffuse[0]  = 0.9;
	mat.diffuse[1]  = 0.9;
	mat.diffuse[2]  = 0.9;
	mat.diffuse[3]  = 1.0;
	mat.specular[0]  = 0.3;
	mat.specular[1]  = 0.3;
	mat.specular[2]  = 0.3;
	mat.specular[3]  = 1.0;
	mat.shininess[0] = 0.5;
	m.addMaterial(mat);
	return m;
}

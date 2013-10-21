#ifdef _WIN32
#include <windows.h>
#endif
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
		if (shapes[i].material.diffuse_texname.size()>0) {
			glEnable(GL_TEXTURE_2D);
			tempMaterial.texture.Load(shapes[i].material.diffuse_texname.c_str());
			tempMaterial.hasTexture=true;
			glBindTexture(GL_TEXTURE_2D, tempMaterial.texture.GetId());
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
			std::cout << "" << std::endl;
			glTexImage2D(GL_TEXTURE_2D, 0, tempMaterial.texture.Bpp(), tempMaterial.texture.Width(), tempMaterial.texture.Height(), 0, tempMaterial.texture.Format(), GL_UNSIGNED_BYTE, tempMaterial.texture.GetData());
			glDisable(GL_TEXTURE_2D);

		}
		tempMaterial.ambient[3]=1.0f;
		tempMaterial.diffuse[3]=1.0f;
		tempMaterial.specular[3]=1.0f;
		tempMaterial.emission[3]=1.0f;
		tempMaterial.transmittance[3]=1.0f;
		tempMaterial.shininess[0]=shapes[i].material.shininess;

		addMaterial(shapes[i].material.name,tempMaterial);
		std::vector<float> positions = shapes[i].mesh.positions;
		std::vector<float> normals   = shapes[i].mesh.normals;
		std::vector<float> uvs       = shapes[i].mesh.texcoords;
		std::vector<unsigned int> indices   = shapes[i].mesh.indices;
		size_t numNorm = normals.size()/3;
		size_t numFaces = indices.size()/3;
		for (size_t j=0;j<numFaces;++j) {
			Tri t;
			t.material=shapes[i].material.name;
			for (size_t k=0;k<3;++k) {
				size_t index = indices[j*3+k];
				if (index<numNorm) {
					t.normal[k].x=normals[index*3+0];
					t.normal[k].y=normals[index*3+1];
					t.normal[k].z=normals[index*3+2];
				}
				else {
					t.normal[k]=Vec3d(0.0,0.0,1.0);
				}
				t.vertex[k].x=positions[index*3+0];
				t.vertex[k].y=positions[index*3+1];
				t.vertex[k].z=positions[index*3+2];
			}
			tris.push_back(t);
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
    if (mat.hasTexture) {
    	glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mat.texture.GetId());
    }
    else {
    	glDisable(GL_TEXTURE_2D);
    }
}

void Model::render() {
	lastUsedMaterial="";
	// Render tris
	glBegin(GL_TRIANGLES);
	for (int i=0;i<tris.size();++i) {
		applyMaterial(tris[i].material);
		for (int j=0;j<3;++j) {
			glTexCoord2f(tris[i].uvw[j].x,tris[i].uvw[j].y);
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
			glTexCoord2f(tris[i].uvw[j].x,tris[i].uvw[j].y);
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
			glTexCoord2f(tris[i].uvw[j].x,tris[i].uvw[j].y);
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

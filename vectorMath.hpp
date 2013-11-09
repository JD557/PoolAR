#ifndef _VECMATH_H_
#define _VECMATH_H_

#include <cmath>

#define COMPARE_DELTA 0.0025

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
	Vec3d transform(float *M) const {
		float res[4];
	    res[0]=M[ 0]*this->x+M[ 4]*this->y+M[ 8]*this->z+M[12]*1;
	    res[1]=M[ 1]*this->x+M[ 5]*this->y+M[ 9]*this->z+M[13]*1;
	    res[2]=M[ 2]*this->x+M[ 6]*this->y+M[10]*this->z+M[14]*1;
	    return Vec3d(res[0],res[1],res[2]);
	}
};

bool invertMatrix(float* m, float* invOut);

#endif
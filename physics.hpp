#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "bullet/include/btBulletDynamicsCommon.h"

#include <vector>


class Physics {
	public:
	//private:
		btBroadphaseInterface* broadphase;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* dynamicsWorld;
		btRigidBody* club;
		btCollisionShape* ballShape;
		btCollisionShape* groundShape;
		btGeneric6DofConstraint* dof6;
		std::vector<btRigidBody*> balls;

		btRigidBody* createBall(int n);
		void createFloor();

		void initClub();

		void updateClub(float x, float y, float z);
		

	public:
		Physics();
		~Physics();

		std::vector<btRigidBody*> getBalls();
};

#endif
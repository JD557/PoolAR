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

		btCollisionShape* ballShape;
		btCollisionShape* groundShape;

		std::vector<btRigidBody*> balls;

		btRigidBody* createBall(int n);
		void createFloor();

	public:
		Physics();
		~Physics();

		std::vector<btRigidBody*> getBalls();
};

#endif
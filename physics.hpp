#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "bullet/include/btBulletDynamicsCommon.h"

class Physics {
	//private:
	public:
		btBroadphaseInterface* broadphase;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* dynamicsWorld;

		btCollisionShape* groundShape;
		btCollisionShape* fallShape;
		btCollisionShape* box;

		btRigidBody* fallRigidBody;

	//public:
		Physics();
		~Physics();
	
};

#endif
#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#define BIT(x) (1<<(x))

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
		
		static void myTickCallback(btDynamicsWorld *world, btScalar timeStep);

		enum CollisionTypes{
			COL_NOTHING=0,
			COL_BALL=BIT(0),
			COL_TABLE=BIT(1),
			COL_CLUB=BIT(2)
		};

		static int ballCollidesWith;
		static int tableCollidesWith;
		static int clubCollidesWith;

	public:
		Physics();
		~Physics();

		std::vector<btRigidBody*> getBalls();
};

#endif
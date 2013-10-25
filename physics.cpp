#include "physics.hpp"
//#include "bullet/include/btBulletCollisionCommon.h"
#include <iostream>

Physics::Physics(){
	broadphase = new btDbvtBroadphase();

    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-10,0));

	//groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
	
    fallShape = new btSphereShape(1);

	

	groundShape = new btBoxShape(btVector3(btScalar(100.),btScalar(50.),btScalar(100.)));
	
	//m_collisionShapes.push_back(groundShape);

	btTransform groundTransformm;
	groundTransformm.setIdentity();
	groundTransformm.setOrigin(btVector3(0,-50,0));
	//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
	btScalar massg(0.);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (massg != 0.f);

	btVector3 localInertiaaa(0,0,0);
	if (isDynamic)
		groundShape->calculateLocalInertia(massg,localInertiaaa);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionStatee = new btDefaultMotionState(groundTransformm);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(massg,myMotionStatee,groundShape,localInertiaaa);
	btRigidBody* body = new btRigidBody(rbInfo);
	//body->setFriction(1);
	//body->setRollingFriction(1);
	//add the body to the dynamics world
	dynamicsWorld->addRigidBody(body);




	//ball
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(15,10,30));
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btScalar mass = 1.2;
 
    btVector3 localInertia(0,0,0);
	fallShape->calculateLocalInertia(mass,localInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyInfo(mass,myMotionState,fallShape,localInertia);
    fallRigidBody = new btRigidBody(fallRigidBodyInfo);
	fallRigidBody->setFriction(0.5f);
	fallRigidBody->setRestitution(0.01f);

	fallRigidBody->setDamping(0.2f,0.1f);

	fallRigidBody->setRollingFriction(0.5f);
	//fallRigidBody->setAnisotropicFriction(fallShape->getAnisotropicRollingFrictionDirection(),btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);
    dynamicsWorld->addRigidBody(fallRigidBody);
	
	
 
	

	//fallRigidBody->setLinearVelocity( btVector3(0,0,0) );
	
}

Physics::~Physics(){
	delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}
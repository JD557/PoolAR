#include "physics.hpp"
//#include "bullet/include/btBulletCollisionCommon.h"
#include <iostream>
#include "Assets\floor.h"

Physics::Physics(){
	broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-10,0));

	balls.reserve(15);

	//shapes
    ballShape = new btSphereShape(3.5);//new btSphereShape(4.53);
	groundShape = new btBoxShape(btVector3(btScalar(100.),btScalar(50.),btScalar(100.)));
	

	//m_collisionShapes.push_back(groundShape);
	/*
	btTransform groundTransformm;
	groundTransformm.setIdentity();
	groundTransformm.setOrigin(btVector3(0,-50,0));
	btScalar massg(0.);
	bool isDynamic = (massg != 0.f);
	btVector3 localInertiaaa(0,0,0);
	btDefaultMotionState* myMotionStatee = new btDefaultMotionState(groundTransformm);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(massg,myMotionStatee,groundShape,localInertiaaa);
	btRigidBody* body = new btRigidBody(rbInfo);
	dynamicsWorld->addRigidBody(body);
	*/



	//ball
	for(int i=0; i<15; ++i){
		btRigidBody* ballRigidBody = createBall(i);
		balls.push_back(ballRigidBody);
		dynamicsWorld->addRigidBody(ballRigidBody);
	}
	
	
	createFloor();
	

	//fallRigidBody->setLinearVelocity( btVector3(0,0,0) );
	
}

Physics::~Physics(){
	delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}

btRigidBody* Physics::createBall(int n){
	btVector3 position = btVector3(1,0,1);

	switch (n)
	{
	case 1:
		position = btVector3(1,0,1);
		break;
	case 2:
		position = btVector3(6,0,1);
		break;
	case 3:
		position = btVector3(11,0,1);
		break;
	case 4:
		position = btVector3(16,0,1);
		break;
	case 5:
		position = btVector3(1,0,6);
		break;
	case 6:
		position = btVector3(6,0,6);
		break;
	case 7:
		position = btVector3(11,0,6);
		break;
	case 8:
		position = btVector3(16,0,6);
		break;
	case 9:
		position = btVector3(1,0,11);
		break;
	case 10:
		position = btVector3(6,0,11);
		break;
	case 11:
		position = btVector3(11,0,11);
		break;
	case 12:
		position = btVector3(16,0,11);
		break;
	case 13:
		position = btVector3(1,0,16);
		break;
	case 14:
		position = btVector3(6,0,16);
		break;
	case 15:
		position = btVector3(11,0,16);
		break;
	default:
		break;
	}
	/*
	position*=(float)n*12;
	if(position[0]>50){
		position[0]=-50;
		position[2]+=0;
	}
	*/
	position[1]=5;
	position[0]+=20;
	position[2]+=20;
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(position);
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btScalar mass = 1.8;
    btVector3 localInertia(0,0,0);
	ballShape->calculateLocalInertia(mass,localInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyInfo(mass,myMotionState,ballShape,localInertia);
    btRigidBody* ballRigidBody = new btRigidBody(fallRigidBodyInfo);
	ballRigidBody->setFriction(0.4f);
	ballRigidBody->setRestitution(0.95f);
	ballRigidBody->setDamping(0.2f,0.1f);
	ballRigidBody->setRollingFriction(0.4f);
	return ballRigidBody;
}

std::vector<btRigidBody*> Physics::getBalls(){
	return balls;
}

void Physics::createFloor(){
	btTriangleIndexVertexArray* floor;
	
	floor = new btTriangleIndexVertexArray(198,floor_faces,sizeof(btScalar)*3,205,floor_verts,sizeof(int)*3);
	
	btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(floor, true);

	btTransform groundTransformm;
	groundTransformm.setIdentity();
	groundTransformm.setOrigin(btVector3(0,0,0));
	groundTransformm.setRotation(btQuaternion(btVector3(1,0,0),btRadians(-90)));
	btScalar massg(0.);
	btVector3 localInertiaaa(0,0,0);
	btDefaultMotionState* myMotionStatee = new btDefaultMotionState(groundTransformm);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(massg,myMotionStatee,trimesh,localInertiaaa);
	btRigidBody* body = new btRigidBody(rbInfo);
	dynamicsWorld->addRigidBody(body);
}
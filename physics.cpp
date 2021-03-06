#include "physics.hpp"
//#include "bullet/include/btBulletCollisionCommon.h"
#include <iostream>
#include "Assets/floor.h"

int Physics::ballCollidesWith = COL_BALL|COL_TABLE|COL_CLUB;
int Physics::tableCollidesWith = COL_BALL;
int Physics::clubCollidesWith = COL_BALL;




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
	//groundShape = new btBoxShape(btVector3(btScalar(100.),btScalar(50.),btScalar(100.)));
	

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
	for(int i=0; i<=15; ++i){
		btRigidBody* ballRigidBody = createBall(i);
		balls.push_back(ballRigidBody);
		dynamicsWorld->addRigidBody(ballRigidBody, COL_BALL, Physics::ballCollidesWith);
	}
	
	
	createFloor();
	initClub();

	dynamicsWorld->setInternalTickCallback(myTickCallback, static_cast<void *>(this));

	
	
	
	//fallRigidBody->setLinearVelocity( btVector3(0,0,0) );
	
}

void Physics::myTickCallback(btDynamicsWorld *world, btScalar timeStep){
	Physics *w = static_cast<Physics *>(world->getWorldUserInfo());
    
	for(int i=0; i< w->getBalls().size(); ++i){
		btRigidBody* b = w->getBalls()[i];
		btVector3 velocity = b->getLinearVelocity();
		if(velocity[1] > 0) velocity[1]=0;
		b->setLinearVelocity(velocity);
	}
}

Physics::~Physics(){
	if(dof6!=NULL){
		dynamicsWorld->removeConstraint(dof6);
		delete dof6;
	}
	delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}

btVector3 linePos(int n,int l){
	int nr = 5-l;
	return btVector3( 7.0+7.0*(n%nr)+0.1+3.5*l, 3.6, 7*l+0.1);
}

btVector3 calcBallPosition(int n){
	if(n>0 && n<=5){
		return linePos(n,0);
	} else if(n>5 && n<= 9){
		return linePos(n,1);
	} else if(n>9 && n<=12){
		return linePos(n,2);
	} else if(n>12 && n<=14){
		return linePos(n,3);
	} else if(n==15){
		return linePos(n,4);
	} else {
		btVector3 b = linePos(14,4);
		b[2]=60;
		return b;
	} 
}

btRigidBody* Physics::createBall(int n){
	btVector3 position = btVector3(1,0,1);
	/*
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
	
	position[1]=5;
	position[0]+=20;
	position[2]+=20;
	*/

	position=calcBallPosition(n);
	position[0]+=16;
	position[2]+=23;

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(position);
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btScalar mass = 0.75;
    btVector3 localInertia(0,0,0);
	ballShape->calculateLocalInertia(mass,localInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyInfo(mass,myMotionState,ballShape,localInertia);
    btRigidBody* ballRigidBody = new btRigidBody(fallRigidBodyInfo);
	ballRigidBody->setFriction(0.1f);
	ballRigidBody->setRestitution(0.9f);
	//ballRigidBody->setDamping(0.2f,0.1f);
	ballRigidBody->setRollingFriction(0.1f);
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
	dynamicsWorld->addRigidBody(body,COL_TABLE,Physics::tableCollidesWith);
	body->setFriction(3.5f);
	body->setRestitution(0.7);
}

void Physics::initClub(){
	btVector3 position = btVector3(0,0,0);
	
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(position);
	btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
    btScalar mass = 0.5;
    btVector3 localInertia(0,0,0);
	ballShape->calculateLocalInertia(mass,localInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyInfo(mass,myMotionState,ballShape,localInertia);
    club = new btRigidBody(fallRigidBodyInfo);
	club->setFriction(0.6f);
	club->setRestitution(1);
	//club->setDamping(0.2f,0.1f);
	//club->setRollingFriction(0.6f);

	club->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(club,COL_CLUB,Physics::clubCollidesWith);

	btVector3 localPivot = club->getCenterOfMassTransform().inverse() * btVector3(-3,2,0);

	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(localPivot);

	dof6 = new btGeneric6DofConstraint(*club, tr,false);
	dof6->setLinearLowerLimit(btVector3(0,0,0));
	dof6->setLinearUpperLimit(btVector3(0,0,0));
	dof6->setAngularLowerLimit(btVector3(0,0,0));
	dof6->setAngularUpperLimit(btVector3(0,0,0));

	dynamicsWorld->addConstraint(dof6,true);

	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,0);
	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,1);
	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,2);
	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,3);
	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,4);
	dof6->setParam(BT_CONSTRAINT_STOP_CFM,0.8,5);

	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,0);
	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,1);
	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,2);
	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,3);
	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,4);
	dof6->setParam(BT_CONSTRAINT_STOP_ERP,0.1,5);
}

void Physics::updateClub(float x, float y, float z){

	btGeneric6DofConstraint* pickCon = static_cast<btGeneric6DofConstraint*>(dof6);
	if (pickCon)
	{
		//keep it at the same picking distance
		/*
		btVector3 newRayTo = getRayTo(x,y);
		btVector3 rayFrom;
		btVector3 oldPivotInB = pickCon->getFrameOffsetA().getOrigin();

		btVector3 newPivotB;
		if (m_ortho)
		{
			newPivotB = oldPivotInB;
			newPivotB.setX(newRayTo.getX());
			newPivotB.setY(newRayTo.getY());
		} else
		{
			rayFrom = m_cameraPosition;
			btVector3 dir = newRayTo-rayFrom;
			dir.normalize();
			dir *= gOldPickingDist;

			newPivotB = rayFrom + dir;
		}*/
		if(y<10) y=5;
		pickCon->getFrameOffsetA().setOrigin(btVector3(x,y,z));
		
	}
}

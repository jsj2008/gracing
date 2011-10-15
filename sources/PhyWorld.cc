//  gracing - a idiot (but physically powered) racing game 
//  Copyright (C) 2010 gianni masullo
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <assert.h>

#include "PhyWorld.h"
#include "gmlog.h"

using namespace irr;

CFG_PARAM_D(glob_frameRate)=60.;

static inline void irr2bt(const core::vector3df & irrVertex,
    btVector3 & btVertex)
{
  btVertex.setX(irrVertex.X);
  btVertex.setY(irrVertex.Y);
  btVertex.setZ(irrVertex.Z);
}

PhyWorld* PhyWorld::buildMe()
{
  PhyWorld * self;

  btBroadphaseInterface               *broadPhase;
  btCollisionDispatcher               *dispatcher;
  btSequentialImpulseConstraintSolver *solver;
  btDefaultCollisionConfiguration     *collisionConfiguration;

  broadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  solver = new btSequentialImpulseConstraintSolver();

  self=new PhyWorld(broadPhase,dispatcher,solver,collisionConfiguration);

  return self;
}

PhyWorld::PhyWorld(
  btBroadphaseInterface               *broadPhase,
  btCollisionDispatcher               *dispatcher,
  btSequentialImpulseConstraintSolver *solver,
  btDefaultCollisionConfiguration     *collisionConfiguration)
  : btDiscreteDynamicsWorld(dispatcher, broadPhase, solver, collisionConfiguration)
{
  CFG_INIT_D(m_frameDuration,1.f/glob_frameRate);
  CFG_INIT_D(m_frameSubsteps,1);
  CFG_INIT_V3(m_gravity,0.f,-10.f,0.f);
  CFG_INIT_D(m_defaultContactProcessingThreshold,BT_LARGE_FLOAT);
	setGravity(btVector3(
        m_gravity[0],
        m_gravity[1],
        m_gravity[2]));
}

btRigidBody * PhyWorld::createRigidBody(
    irr::scene::ISceneNode * node,
    float mass, 
    const btTransform& startTransform, 
    btCollisionShape* shape,
    btMotionState* motionState)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

  if(motionState==0) 
    motionState=new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,motionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

	return body;
}


PhyWorld::~PhyWorld()
{
}

/* 
 * add a static mesh to the physics engine 
 */
btRigidBody * PhyWorld::addStaticMesh(scene::ISceneNode * meshNode)
{
  const core::vector3df &pos   = meshNode->getPosition();
  const core::vector3df &hpr   = meshNode->getRotation();
  const core::vector3df &scale = meshNode->getScale();

  core::matrix4 mat;
  mat.setRotationDegrees(hpr);
  mat.setTranslation(pos);
  core::matrix4 mat_scale;
  // Note that we can't simply call mat.setScale, since this would
  // overwrite the elements on the diagonal, making any rotation incorrect.
  mat_scale.setScale(scale);
  mat *= mat_scale;

  assert(meshNode->getType()==scene::ESNT_MESH || 
      meshNode->getType()==scene::ESNT_ANIMATED_MESH ||
      meshNode->getType()==scene::ESNT_OCTREE);

  scene::IMesh * mesh;

  switch(meshNode->getType()) {
    case scene::ESNT_MESH:
    case scene::ESNT_OCTREE:
      mesh=((scene::IMeshSceneNode*)meshNode)->getMesh();
      break;
    case scene::ESNT_ANIMATED_MESH:
      mesh=((scene::IAnimatedMeshSceneNode*)meshNode)->getMesh();
      break;
#if 0
      mesh=((scene::IOctreeSceneNode*)meshNode)->getMesh();
      break;
#endif
    default:
      assert(0);
      break;
  }

  unsigned int n_buffers;
  unsigned int n_vertices;
  unsigned int n_triangles;

  n_buffers=mesh->getMeshBufferCount();
  u16 * indices;

  meshNode->grab();

  btVector3 btv1,btv2,btv3;
  btTriangleMesh *  trimesh=new btTriangleMesh();

  for(unsigned int i=0; i<n_buffers; i++) {
    scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
    n_vertices=mb->getIndexCount();
    n_triangles=n_vertices/3;
    indices=mb->getIndices();
    for(unsigned int j=0; j<n_vertices; j+=3) {
      core::vector3df & v1=mb->getPosition(indices[j]);
      core::vector3df & v2=mb->getPosition(indices[j+1]);
      core::vector3df & v3=mb->getPosition(indices[j+2]);
      irr2bt(v1,btv1);
      irr2bt(v2,btv2);
      irr2bt(v3,btv3);
      trimesh->addTriangle(btv1,btv2,btv3);
    }
  }


  btScalar mass(0.);
  btVector3 localInertia(0,0,0);

	btCollisionShape* shape = 
	   new btBvhTriangleMeshShape(trimesh,false);
	btTransform transform;
	transform.setIdentity();
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(
      mass,motionState,
      shape,localInertia);
  btRigidBody* body = new btRigidBody(rbInfo);
  addRigidBody(body);

  return body;
}

btRigidBody * PhyWorld::addDynamicSphere(irr::scene::ISceneNode * node, 
    float px,float py, float pz,
    float radius, float mass)
{
  btCollisionShape* shape = new btSphereShape(btScalar(radius));

  btTransform startTransform;
  startTransform.setIdentity();

  btScalar	btMass(mass);

  bool isDynamic = (btMass != 0.f);

  btVector3 localInertia(0,0,0);
  if (isDynamic)
    shape->calculateLocalInertia(btMass,localInertia);

  startTransform.setOrigin(btVector3(px,py,pz));

  //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
  btDefaultMotionState* motionState = 
    new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo 
    rbInfo(btMass,motionState,shape,localInertia);
  btRigidBody* body = new btRigidBody(rbInfo);


  addRigidBody(body);
  return body;
}

void PhyWorld::clear()
{
  for(unsigned i=0; i<m_rigidBodies.size(); i++) {
    btRigidBody * body = m_rigidBodies[i];
    btDiscreteDynamicsWorld::removeRigidBody(body);
  }
  m_rigidBodies.clear();
}

void PhyWorld::addRigidBody(btRigidBody * body)
{
  m_rigidBodies.push_back(body);
  btDiscreteDynamicsWorld::addRigidBody(body);
}

void PhyWorld::removeRigidBody(btRigidBody * body)
{
  //for(unsigned i=0; i<m_rigidBodies.size(); i++) 
  std::vector<btRigidBody*>::iterator it;
  for(it=m_rigidBodies.begin(); it != m_rigidBodies.end(); it++)
    if(*it == body) {
      m_rigidBodies.erase(it);
      break;
    }
  btDiscreteDynamicsWorld::removeRigidBody(body);
}

void PhyWorld::step()
{
  stepSimulation(m_frameDuration,m_frameSubsteps,m_frameDuration);
}

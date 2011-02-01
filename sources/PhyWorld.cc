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

static inline void irr2bt(const core::vector3df & irrVertex,
    btVector3 & btVertex)
{
  btVertex.setX(irrVertex.X);
  btVertex.setY(irrVertex.Y);
  btVertex.setZ(irrVertex.Z);
}

PhyWorld::PhyWorld()
{
  CFG_INIT_D(m_frameRate,1.f/240.f);
  CFG_INIT_D(m_frameSubsteps,1);




  CFG_INIT_V3(m_gravity,0.f,-10.f,0.f);

  m_broadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
  m_collisionConfiguration = new btDefaultCollisionConfiguration();
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
  m_solver = new btSequentialImpulseConstraintSolver();
  m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadPhase, m_solver, m_collisionConfiguration);

	m_world->setGravity(btVector3(
        m_gravity[0],
        m_gravity[1],
        m_gravity[2]));
}

PhyWorld::~PhyWorld()
{
  delete m_broadPhase;
  delete m_collisionConfiguration;
  delete m_dispatcher;
  delete m_solver;
  delete m_world;
}

/* 
 * add a static mesh to the physics engine 
 */
void PhyWorld::addStaticMesh(scene::ISceneNode * meshNode)
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
      meshNode->getType()==scene::ESNT_ANIMATED_MESH);

  scene::IMesh * mesh;

  switch(meshNode->getType()) {
    case scene::ESNT_MESH:
      mesh=((scene::IMeshSceneNode*)meshNode)->getMesh();
      break;
    case scene::ESNT_ANIMATED_MESH:
      mesh=((scene::IAnimatedMeshSceneNode*)meshNode)->getMesh();
      break;
  }

  unsigned int n_buffers;
  unsigned int n_vertices;
  unsigned int n_triangles;

  n_buffers=mesh->getMeshBufferCount();
  u16 * indices;
  meshBinder * mbinder=new meshBinder();

  mbinder->irrNode=meshNode;

  GM_LOG("adding mesh: %d buffers\n",n_buffers);

  btVector3 btv1,btv2,btv3;
  btTriangleMesh *  trimesh=new btTriangleMesh();

  for(unsigned int i=0; i<n_buffers; i++) {
    scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
    n_vertices=mb->getIndexCount();
    n_triangles=n_vertices/3;
    indices=mb->getIndices();
    GM_LOG("buffer %d: %d vertices, %d faces\n",i,n_vertices,n_triangles);
    for(unsigned int j=0; j<n_vertices; j+=3) {
      core::vector3df & v1=mb->getPosition(indices[j]);
      core::vector3df & v2=mb->getPosition(indices[j+1]);
      core::vector3df & v3=mb->getPosition(indices[j+2]);
      irr2bt(v1,btv1);
      irr2bt(v2,btv2);
      irr2bt(v3,btv3);
      trimesh->addTriangle(btv1,btv2,btv3);
#if 0
      GM_LOG(" [%02d] %d<%3.2f,%3.2f,%3.2f>, %d<%3.2f,%3.2f,%3.2f>, %d<%3.2f,%3.2f,%3.2f>\n",
          j/3,
          indices[j],v1.X,v1.Y,v1.Z,
          indices[j+1],v2.X,v2.Y,v2.Z,
          indices[j+2],v3.X,v3.Y,v3.Z);
#endif
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
  mbinder->body=body;
  m_world->addRigidBody(body);

  m_binds.push_back(mbinder);
}

void PhyWorld::addDynamicSphere(irr::scene::ISceneNode * node, 
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

  meshBinder * mbinder=new meshBinder(body,node);
  m_binds.push_back(mbinder);

  m_world->addRigidBody(body);

}

void PhyWorld::step()
{
  m_world->stepSimulation(m_frameRate,m_frameSubsteps);
  for (int j=m_binds.size()-1; j>=0 ;j--) {
    btRigidBody* body = m_binds[j]->body;
    if (body && body->getMotionState()) {
      btTransform trans;
      btQuaternion quaternion;
      body->getMotionState()->getWorldTransform(trans);
      quaternion=trans.getRotation();
      m_binds[j]->irrNode->setPosition(
          core::vector3df(
            float(trans.getOrigin().getX()),
            float(trans.getOrigin().getY()),
            float(trans.getOrigin().getZ())));
    }
  }
}

#include <assert.h>

#include "PhyWorld.h"
#include "gmlog.h"

using namespace irr;

PhyWorld::PhyWorld()
{
  m_broadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
  m_collisionConfiguration = new btDefaultCollisionConfiguration();
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
  m_solver = new btSequentialImpulseConstraintSolver();
  m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadPhase, m_solver, m_collisionConfiguration);
}

PhyWorld::~PhyWorld()
{
  delete m_broadPhase;
  delete m_collisionConfiguration;
  delete m_dispatcher;
  delete m_solver;
  delete m_world;
}

void PhyWorld::addMesh(scene::ISceneNode * meshNode)
{

  GM_LOG("node type: %X, other %X\n",
      meshNode->getType(),scene::ESNT_MESH);
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


  //btTriangleMesh *CollisionMesh = new btTriangleMesh(); 

  unsigned int n_buffers;
  unsigned int n_vertices;

  n_buffers=mesh->getMeshBufferCount();

  GM_LOG("adding mesh: %d buffers\n",n_buffers);

  for(unsigned int i=0; i<n_buffers; i++) {
    scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
    n_vertices=mb->getIndexCount();
    GM_LOG("buffer %d: %d vertices\n",i,n_vertices);
    for(unsigned int j=0; j<n_vertices; j+=3) {
      for(unsigned int k=0; k<3; k++) {
      }   // for k
    }
  }
}

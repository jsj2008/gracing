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

  GM_LOG("adding mesh: %d buffers\n",n_buffers);

  btTriangleMesh * triangleMesh; //=new btTriangleMesh();

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
      GM_LOG(" [%02d] %d<%3.2f,%3.2f,%3.2f>, %d<%3.2f,%3.2f,%3.2f>, %d<%3.2f,%3.2f,%3.2f>\n",
          j/3,
          indices[j],v1.X,v1.Y,v1.Z,
          indices[j+1],v2.X,v2.Y,v2.Z,
          indices[j+2],v3.X,v3.Y,v3.Z);
    }
  }
}

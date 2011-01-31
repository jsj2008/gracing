#ifndef PHYWORLD_H
#define PHYWORLD_H
#include <irrlicht.h>
#include "btBulletDynamicsCommon.h"


class PhyWorld
{
  public:
    PhyWorld();
    ~PhyWorld();


    void addMesh(irr::scene::ISceneNode * mesh);

  private:

    struct phyMesh {
      btVector3 * vertices;
      int *       indices;

      phyMesh(int n_vertices,int n_triangleIndices)
      {
        vertices=new btVector3[n_vertices];
        indices=3*n_triangleIndices;
      }

      ~phyMesh()
      {
        delete vertices;
        delete indices;
      }
    };

    btBroadphaseInterface               *m_broadPhase;
    btDefaultCollisionConfiguration     *m_collisionConfiguration;
    btCollisionDispatcher               *m_dispatcher;
    btSequentialImpulseConstraintSolver *m_solver;
    btDiscreteDynamicsWorld             *m_world;

    irr::core::array<phyMesh*>          *m_phyMeshes;

};

#endif



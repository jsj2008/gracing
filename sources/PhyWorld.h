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
      irr::scene::ISceneNode * 
        node;

      phyMesh(int n_vertices,int n_triangleIndices, irr::scene::ISceneNode * n)
      {
        vertices=new btVector3[n_vertices];
        indices=new int[3*n_triangleIndices];
        node=n;
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



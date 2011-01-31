#ifndef PHYWORLD_H
#define PHYWORLD_H
#include <irrlicht.h>
#include "config.h"
#include "btBulletDynamicsCommon.h"


class PhyWorld
{
  public:
    PhyWorld();
    ~PhyWorld();

    void addStaticMesh(irr::scene::ISceneNode * node);

    void addDynamicSphere(irr::scene::ISceneNode * node, 
        float radius, float mass);

    void addDynamicSphere(irr::scene::ISceneNode * node, 
        float px,float py, float pz,
        float radius, float mass);

    void step();

  private:

    struct meshBinder {
      btRigidBody *               body;
      irr::scene::ISceneNode *    irrNode;

      meshBinder() { body=0; irrNode=0; }
      meshBinder(
          btRigidBody * b,
          irr::scene::ISceneNode * i)
      { body=b; irrNode=i; }

      ~meshBinder()
      {
        if(body) delete body;
        if(irrNode) irrNode->drop();
      }
    };

    btBroadphaseInterface               *m_broadPhase;
    btDefaultCollisionConfiguration     *m_collisionConfiguration;
    btCollisionDispatcher               *m_dispatcher;
    btSequentialImpulseConstraintSolver *m_solver;
    btDiscreteDynamicsWorld             *m_world;

    irr::core::array<meshBinder*>        m_binds;

    CFG_PARAM_D(m_frameRate);
    CFG_PARAM_D(m_frameSubsteps);
    CFG_PARAM_V3(m_gravity);
};

#endif



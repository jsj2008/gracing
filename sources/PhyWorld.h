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



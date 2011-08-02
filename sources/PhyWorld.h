//  gracing - an idiot (but physically powered) racing game 
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


class PhyWorld : public btDiscreteDynamicsWorld           
{
  public:
    static PhyWorld * buildMe();

    btRigidBody * addDynamicSphere(irr::scene::ISceneNode * node, 
        float px,float py, float pz,
        float radius, float mass);

    btRigidBody * addStaticMesh(irr::scene::ISceneNode * meshNode);

    btRigidBody * createRigidBody(
        irr::scene::ISceneNode * node,
        float mass, const btTransform& startTransform, 
        btCollisionShape* shape,
        btMotionState * motionState=0);

    void step();

    ~PhyWorld();

    // debug stuff

    static inline void btTransformToIrrlichtMatrix(const btTransform& worldTrans, irr::core::matrix4 &matr) 
    {
      worldTrans.getOpenGLMatrix(matr.pointer()); 
    }
  private:
    PhyWorld(
        btBroadphaseInterface               *broadPhase,
        btCollisionDispatcher               *dispatcher,
        btSequentialImpulseConstraintSolver *solver,
        btDefaultCollisionConfiguration     *collisionConfiguration);

    CFG_PARAM_D(m_frameDuration);
    CFG_PARAM_D(m_frameSubsteps);
    CFG_PARAM_V3(m_gravity);
    CFG_PARAM_D(m_defaultContactProcessingThreshold);
};

#endif



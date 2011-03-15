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
#ifndef VEHICLE_H
#define VEHICLE_H

#include <irrlicht.h>
#include "PhyWorld.h"
#include "IVehicle.h"

class Vehicle : public IVehicle, public btActionInterface
{
  public:
    Vehicle(
        irr::scene::ISceneNode * parent,
        irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * source,
        irr::s32 id=-1);

    ~Vehicle();

    virtual void load();

    virtual void unload();

    virtual void use(unsigned int useFlags);

    virtual void unuse(unsigned int useFlags);

    virtual irr::core::vector3df getChassisPos();

    void reset(const irr::core::vector3d<float>& position);

    // from IVehicle
    virtual void throttleUp();
    virtual void throttleDown();
    virtual void throttleSet(double value);
    virtual void brake();
    virtual void steerLeft();
    virtual void steerRight();
    virtual void applyTorque(float x, float y, float z);
    virtual void step();

    // deprecated ?!?
    virtual void dumpDebugInfo() { };

    // from btActionInterface
    virtual void updateAction(btCollisionWorld* world, btScalar deltaTime);
    virtual void debugDraw(btIDebugDraw*);


  private:
    
    void initPhysics();
    void deinitPhysics();
    void deinitGraphics();
    void initGraphics();


    float m_steering;
    float m_steeringIncrement;
    float m_steeringClamp;
    float m_throttle;
    float m_throttleIncrement;
    float m_brake;
    
   
    float m_wheelFriction;
    float m_suspensionStiffness;
    float m_suspensionDamping;
    float m_suspensionCompression;
    float m_rollInfluence;
    float m_suspensionRestLength;
    float m_maxSuspensionTravelCm;
    float m_maxSuspensionForce;
	  float m_wheelsDampingCompression;
	  float m_wheelsDampingRelaxation;

    struct WheelData 
    {
      btVector3 hardPointCS;
      btVector3 hardPointWS;
      btVector3 directionWS;
      btVector3 axleWS;
      btVector3 contactPointWS;
      btVector3 contactNormalWS;
      btScalar  suspensionLength;
			btScalar  suspensionRelativeVelocity;
			btScalar  clippedInvContactDotSuspension;
			btScalar  suspensionForce;



      btVector3 position;
      btScalar  radius;
      bool      isInContact;
    };

    WheelData m_wheelsData[4];
    
    const char * m_sourceName;

    PhyWorld *  m_world;

    bool         m_loaded;

    irr::IrrlichtDevice *
                 m_device;

    irr::io::IFileSystem * 
                 m_filesystem;
    
    unsigned     m_using;


    enum {
      W_REAR_LEFT=0,
      W_REAR_RIGHT,
      W_FRONT_LEFT,
      W_FRONT_RIGHT,

      W_NUM_OF_WHEELS
    };

    inline bool isFrontWheel(int index) 
    { 
      return index==W_FRONT_RIGHT || index==W_FRONT_LEFT;
    }

    inline bool isLeftWheel(int index) 
    { 
      return index==W_FRONT_LEFT || index==W_REAR_LEFT;
    }

    inline void updateWheel(int index)
    {
      assert(index>=0 && index<4);

      irr::scene::ISceneNode * wheel=m_wheelsNodes[index];


      if(!wheel)
        return;
      assert(wheel);

      btTransform wheelTrans=btTransform::getIdentity();
      wheelTrans.setOrigin(m_wheelsData[index].hardPointWS);

      irr::core::matrix4 matr;
      PhyWorld::btTransformToIrrlichtMatrix(wheelTrans, matr);

      //wheel->setRotation(matr.getRotationDegrees());
      wheel->setPosition(matr.getTranslation());
    }

    btScalar raycast(WheelData&);

    // graphics part of the vehicle (irrlicht stuff)
    irr::core::array<irr::scene::IAnimatedMesh*>   
                         m_chassis;
    irr::scene::IAnimatedMesh* 
                         m_wheels[4];

    irr::scene::ISceneNode * m_chassisNode;

    irr::scene::ISceneNode * m_wheelsNodes[4];

    double m_wheelRadiuses[4];
    double m_wheelWidths[4];
    irr::core::vector3df m_wheelInitialPositions[4];
    
    // physics part of the vehicle (bullet stuff)
    btCollisionShape*     m_chassisShape;
    btCollisionShape*     m_wheelShapes[4];

    btRigidBody *         m_carBody;
    btVehicleRaycaster *  m_raycaster;

};

#endif

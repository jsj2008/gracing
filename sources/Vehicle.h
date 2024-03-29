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

class XmlNode;

class Vehicle : public IVehicle, public btActionInterface, public btMotionState
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

    const std::string & getName() { return m_name; }


    virtual irr::core::vector3df getChassisPos();
    virtual btVector3            getChassisForwardDirection();
    virtual btVector3            getChassisRightDirection();
    virtual btVector3            getChassisUpDirection();
    virtual bool                 getIfChassisIsTouchingTheGround();

    virtual double getRestHeight(/*float x, float y*/);

    void reset( const irr::core::vector3d<float>& position, double rotation);

    
    void getThrottleAndSteer(double & throttle, double & steer) {
      throttle=m_throttle;
      steer=m_steering;
    }

    virtual void setEnableControls(bool enable);

    virtual btRigidBody * getRigidBody();

    // deprecated ?!?
    virtual void dumpDebugInfo() { };

    // from btActionInterface
    virtual void updateAction(btCollisionWorld* world, btScalar deltaTime);
    virtual void debugDraw(btIDebugDraw*);

    // from btMotionState
    virtual void 	getWorldTransform (btTransform &worldTrans) const;
    virtual void 	setWorldTransform (const btTransform &worldTrans);

    typedef enum { // debug drawing flags
      db_raycastDirection= 0x01,
      db_wheelsAxle=       0x02,
      db_forwardImpulse=   0x04,
      db_sideImpulse=      0x08,
      db_suspensions=      0x10
    };

    void setDebugDrawFlags(unsigned flags);
    

    virtual void setSpeedOMeter(INumberOutput * speedometer);

    virtual double getSpeed();

  private:

    // in/out data
    INumberOutput * m_speedometer;

    // private types
    struct WheelData 
    {
      btVector3     hardPointCS;
      btVector3     hardPointWS;
      btVector3     directionWS;
      btVector3     axleWS;
      btVector3     contactPointWS;
      btVector3     contactNormalWS;
      btTransform   worldTransform;
      btScalar      suspensionLength;
			btScalar      suspensionRelativeVelocity;
			btScalar      clippedInvContactDotSuspension;
			btScalar      suspensionForce;
      btRigidBody * collidingObject;
      btScalar      frictionSlip;
      btScalar      skidInfo;

      btScalar      rotation;
      btScalar      deltaRotation;

      btVector3 position;
      btScalar  radius;
      bool      isInContact;
    };
    
    void initPhysics();
    void initGraphics();

    void deinitPhysics();
    void deinitGraphics();

    void updateFriction(btScalar);
    void step();

    void loadInfo(XmlNode *);

    inline bool isFrontWheel(int index) 
    { 
      return index==W_FRONT_RIGHT || index==W_FRONT_LEFT;
    }

    inline bool isLeftWheel(int index) 
    { 
      return index==W_FRONT_LEFT || index==W_REAR_LEFT;
    }

    inline void updateWheel(int index);
    inline void updateWheelPhysics(int index);

    btScalar raycast(WheelData&,int);


    // private member variables

    btTransform  m_chassisWorldTrans;

    bool  m_controlsEnabled;
    float m_steering;
    float m_steeringIncrement;
    float m_steeringClamp;
    float m_throttle;
    float m_throttleIncrement;
    float m_throttleDecrement;
    float m_brake;


    float m_wheelFriction;
    float m_suspensionStiffness;
    float m_suspensionDamping;
    float m_suspensionCompression;
    float m_rollInfluence;
    float m_suspensionRestLength;
    float m_maxSuspensionTravel;
    float m_maxSuspensionForce;
	  float m_wheelsDampingCompression;
	  float m_wheelsDampingRelaxation;
		btAlignedObjectArray<btVector3>	m_forwardWS;
		btAlignedObjectArray<btVector3>	m_axle;
		btAlignedObjectArray<btScalar>	m_forwardImpulse;
		btAlignedObjectArray<btScalar>	m_sideImpulse;


    WheelData m_wheelsData[4];
    unsigned  m_nWheelTouchGround;
    
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

    unsigned              m_debugDrawFlags;

    double                m_forwardSpeed;

    // vehicle description
    std::string           m_name;
    std::string           m_author;

};

#endif

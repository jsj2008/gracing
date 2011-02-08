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

class Vehicle : public IVehicle
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

  private:
    
    void initPhysics();
    void deinitPhysics();
    void deinitGraphics();
    void initGraphics();


    float m_gVehicleSteering;
    float m_steeringIncrement;
    float m_steeringClamp;
    float m_wheelRadius;
    float m_wheelWidth;
    float m_wheelFriction;
    float m_suspensionStiffness;
    float m_suspensionDamping;
    float m_suspensionCompression;
    float m_rollInfluence;

    
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

    // graphics part of the vehicle (irrlicht stuff)
    irr::core::array<irr::scene::IAnimatedMesh*>   
                         m_chassis;
    irr::scene::IAnimatedMesh* 
                         m_wheels[4];
    irr::core::array<irr::scene::ISceneNode *> 
                         m_irrNodes;

    double m_wheelRadiuses[4];
    double m_wheelWidths[4];
    irr::core::vector3df m_wheelPositions[4];
    
    // physics part of the vehicle (bullet stuff)
    btCompoundShape*      m_vehicleShape;
    btCollisionShape*     m_chassisShape;
    btCollisionShape*     m_wheelShapes[4];

    btRigidBody *         m_carBody;

		btVehicleRaycaster *  m_vehicleRayCaster;
		btRaycastVehicle *    m_raycastVehicle;
    
};

#endif

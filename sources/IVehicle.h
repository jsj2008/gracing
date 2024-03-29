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
#ifndef IVEHICLE_H
#define IVEHICLE_H

#include <string>

#include "CompoundSceneNode.h"
#include "INumberOutput.h"
#include "PhyWorld.h"

// !!! interface for a vehicle !!! //
class IVehicle : public CompoundSceneNode
{
  public:
  enum {
    USE_PHYSICS=0x1,
    USE_GRAPHICS=0x2,
  };

  IVehicle(irr::scene::ISceneNode * parent, irr::scene::ISceneManager * smgr, irr::s32 id=-1) 
      : CompoundSceneNode(parent,smgr,id)
  {
    // empty here
  }

  virtual btRigidBody * getRigidBody()=0;

  // i'm thinking the following 4 function
  // will be deprecated
  virtual void load()=0;
  virtual void unload()=0;
  virtual void use(unsigned int useFlags)=0;
  virtual void unuse(unsigned int useFlags)=0;
  //////////////////////////////////


  // position querying //
  virtual irr::core::vector3df getChassisPos()=0;

  virtual btVector3            getChassisForwardDirection()=0;
  virtual btVector3            getChassisRightDirection()=0;
  virtual btVector3            getChassisUpDirection()=0;
  virtual bool                 getIfChassisIsTouchingTheGround()=0;

  virtual double               getRestHeight()=0; 
  virtual double               getSpeed()=0;

  // position/physics reset //
  virtual void reset(const irr::core::vector3d<float>&pos, double rotation)=0;

  virtual void getThrottleAndSteer(double & throttle, double & steer)=0;

  // commands //
  struct VehicleCommands 
  {
    bool     controlsEnabled;
    double   throttling;
#ifdef ANALOG_CONTROLS
    double steering;
#else
    enum {
      steerNone,
      steerLeft,
      steerRite
    }        steering;
#endif

    bool      brake;

    bool      changeCamera;
    bool      cameraUp;
    bool      cameraDown;
    bool      cameraLeft;
    bool      cameraRight;

    void reset() { 
      memset(this,0,sizeof(*this));
      //throttling=0.; steering=steerNone; changeCamera=cameraUp=cameraDown=false;
    }
  } m_vehicleCommands;

  virtual void             setEnableControls(bool enable)=0;
  inline VehicleCommands & getVehicleCommands() { return m_vehicleCommands; }

  virtual const std::string & getName()=0;

  // debug //
  virtual void dumpDebugInfo()=0;

  // speedometer //
  virtual void setSpeedOMeter(INumberOutput * speedometer)=0;

};

#endif

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

#include "CompoundSceneNode.h"
#include "INumberOutput.h"
#include "PhyWorld.h"

// !!! interface for a vehicle !!!
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

  virtual void load()=0;

  virtual void unload()=0;

  virtual void use(unsigned int useFlags)=0;

  virtual void unuse(unsigned int useFlags)=0;

  virtual void reset(const irr::core::vector3d<float>&pos, double rotation)=0;

  // querying
  virtual irr::core::vector3df getChassisPos()=0;

  // commands (obsoleting?!?!?)
  struct VehicleCommands 
  {
    bool     controlsEnabled;
    double   throttling;
    enum {
      steerLeft,
      steerRite,
      steerNone
    }        steering;
  } m_vehicleCommands;
  virtual void setEnableControls(bool enable)=0;
  inline VehicleCommands & getVehicleCommands() { return m_vehicleCommands; }

#if 0
  virtual void throttleUp()=0;
  virtual void throttleDown()=0;
  virtual void throttleSet(double value)=0;
  virtual void brake()=0;
  virtual void steerLeft()=0;
  virtual void steerRight()=0;
#endif

  virtual double getStartHeight(float x, float y)=0;

  // debug
  virtual void dumpDebugInfo()=0;

  // the speedometer
  virtual void setSpeedOMeter(INumberOutput * speedometer)=0;

  // adding more later...
};

#endif

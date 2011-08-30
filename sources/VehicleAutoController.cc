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

#include "VehicleAutoController.h"
#include "Util.hh"

#include "GuiCommunicator.h"
extern GuiCommunicator * TEMP_communicator;
#ifdef DEBUG_AUTO_CONTROLLER
#define SHOW(fmt,...) do { if(TEMP_communicator) TEMP_communicator->show(fmt, ## __VA_ARGS__); } while(0)
#define ADD(fmt,...) do { if(TEMP_communicator) TEMP_communicator->add(true,fmt, ## __VA_ARGS__); } while(0)
#else 
#define SHOW(fmt,...) 
#define ADD(fmt,...) 
#endif

CFG_PARAM_D(glob_autocontrolAngleEpsilon)=.2;
CFG_PARAM_D(glob_autocontrolAngleBraking)=.5;
CFG_PARAM_D(glob_autocontrolDistance)=4.0;

VehicleAutoController::VehicleAutoController()
{
   m_initialized=false;
}


void VehicleAutoController::updateCommands(
    const SVehicleParameters & parameters,
    const std::vector<btVector3> & controlPoints,
    IVehicle::VehicleCommands &    commands)
{
  unsigned size = controlPoints.size();
  unsigned nextIndex = (m_currentIndex + 1) % size;

  if(!m_initialized)
    return;


  btVector3 dir = controlPoints[m_currentIndex] - parameters.vehiclePosition;
  btVector3 dir2 = controlPoints[nextIndex] - parameters.vehiclePosition;

  double dist = dir.length();
  double dist2 = dir2.length();

  if(dist2 < dist || dist < glob_autocontrolDistance) {
    m_currentIndex = nextIndex;
    dir = dir2;
    dist = dist2;
  }

  dir /= dist;

  double dot = parameters.vehicleRightDirection.dot( dir );

  static unsigned cnt=0;
  if(cnt ++ % 2 == 0)
    commands.throttling = 1.;

  const char * steering;

  steering = "none";
  if(dot < -glob_autocontrolAngleEpsilon) {
    commands.steering=IVehicle::VehicleCommands::steerRite;
    steering = "rite";
  }
  if(dot > glob_autocontrolAngleEpsilon) {
    commands.steering=IVehicle::VehicleCommands::steerLeft;
    steering = "left";
  }

  if(dot > glob_autocontrolAngleBraking || 
     dot < - glob_autocontrolAngleBraking ) {
    commands.throttling = -1.;
  }
  SHOW("s:%s,sp:%2.3f",steering,parameters.vehicleSpeed);
}

void VehicleAutoController::init(
        const std::vector<btVector3> & controlPoints,
        const btVector3 vehicleDirection,
        const btVector3 startPosition)
{
  unsigned size=0;
  size=controlPoints.size();
  double bestDistance=1000000.;
  unsigned bestDistanceIndex=0xffff;
  for(unsigned i=0; i<size; i++) {
    const btVector3 & cpnt = controlPoints[i];
    double dist =  (cpnt - startPosition).length();
    if(dist < bestDistance) {
      bestDistance = dist;
      bestDistanceIndex = i;
    }

#if 0
    GM_LOG("[%d] %2.2f %2.2f %2.2f - dist: %2.2f\n",
        i,
        cpnt.getX(),
        cpnt.getY(),
        cpnt.getZ(),dist);
#endif
  }

  unsigned otherIndex = (bestDistanceIndex + 1) % size;

  double dot = vehicleDirection.dot( controlPoints[bestDistanceIndex] - startPosition );

  if(dot < 0)
    m_currentIndex = otherIndex;
  else
    m_currentIndex = bestDistanceIndex;

  m_initialized=true;
}



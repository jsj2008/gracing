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
CFG_PARAM_D(glob_autocontrolDistance)=4.0;

// when the absolute value of speed times the 'angle' is 
// greater the Braking Factor, the autocontroller
// decides to brake the vehicle
CFG_PARAM_D(glob_autocontrolBrakingFactor)=30.5;

// when the absolute value of speed times the 'angle' is 
// greater the Braking Factor, the autocontroller
// decides to stop throttling.
// (please note that the Unthrottle Factor must
//  lesser than the Braking Factor)
CFG_PARAM_D(glob_autocontrolUnthrottleFactor)=25.5;


VehicleAutoController::VehicleAutoController()
{
   m_initialized=false;
}


void VehicleAutoController::updateCommands(
    const SVehicleParameters & parameters,
    const std::vector<btVector3> & controlPoints,
    IVehicle::VehicleCommands &    commands)
{
  if(!m_initialized)
    return;

}

void VehicleAutoController::init(
        const std::vector<btVector3> & controlPoints,
        const btVector3 vehicleDirection,
        const btVector3 startPosition)
{
  m_initialized=true;

  m_pathway.initialize(controlPoints,5.);

  btVector3 tangent;
  m_pathway.closestPoint(startPosition,tangent);
}



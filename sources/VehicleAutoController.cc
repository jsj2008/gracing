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


VehicleAutoController::VehicleAutoController()
{
   m_initialized=false;
}


static const float dt=1/60.;
static btVector3  pointToFollow;


void VehicleAutoController::updateCommands(
    const SVehicleParameters & parameters,
    const std::vector<btVector3> & controlPoints,
    IVehicle::VehicleCommands &    commands)
{
  if(!m_initialized)
    return;

  m_pathway.updateOnPath(parameters.vehiclePosition);
  btVector3 target;

  m_pathway.getTarget(target);

  target = target - parameters.vehiclePosition;

  double dot=target.dot(parameters.vehicleRightDirection);

#define EPS .3
#define DIVISOR 10.
  if(fabs(dot-parameters.steering) > EPS)
    commands.steering -= (dot-parameters.steering) / 10.;
  else
    commands.steering -= parameters.steering;
  commands.throttling=200.;
}

void VehicleAutoController::init(
        const std::vector<btVector3> & controlPoints,
        const btVector3 vehicleDirection,
        const btVector3 startPosition)
{
  m_initialized=true;

  m_pathway.initialize(controlPoints,startPosition,5.);
}



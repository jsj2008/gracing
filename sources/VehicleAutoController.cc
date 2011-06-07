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

static const double epsilon=2.5;  //1.1;

VehicleAutoController::VehicleAutoController()
{
  // ?!?!
}

static unsigned oo=55;

void VehicleAutoController::updateCommands(
        const btVector3 &              vehicleDirection,
        const btVector3 &              vehicleRightDirection,
        const btVector3 &              vehiclePosition,
        unsigned                       index,
        const std::vector<btVector3> & controlPoints,
        IVehicle::VehicleCommands &    commands)
{
  unsigned nindex=(index+1) % controlPoints.size(); 

  //btVector3 dir = controlPoints[nindex] - controlPoints[index];
  btVector3 dir = controlPoints[nindex] - vehiclePosition;

  btScalar dot = dir.dot(vehicleRightDirection);

  if(index != oo) {
    oo=index;
    GM_LOG("New index: %d, dir: %f,%f,%f, dot: %f\n",
        index,dir.getX(),dir.getY(),dir.getZ(),dot);
  }

  if(dot > epsilon) 
    commands.steering=IVehicle::VehicleCommands::steerLeft;

  if(dot < - epsilon) 
    commands.steering=IVehicle::VehicleCommands::steerRite;

  commands.throttling=1.;

}


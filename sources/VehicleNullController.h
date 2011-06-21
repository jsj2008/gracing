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
#ifndef IVEHICLE_NULL_CONTROLLER_H
#define IVEHICLE_NULL_CONTROLLER_H
#include "IVehicleController.h"
#include "EventReceiver.h"
#include <irrlicht.h>

class VehicleNullController : public IVehicleController
{
  public: 
    VehicleNullController()
    {
      // does nothing!
    }

    virtual void updateCommands(
        const btVector3 &              vehicleDirection,
        const btVector3 &              vehicleRightDirection,
        const btVector3 &              vehiclePosition,
        unsigned                       index,
        const std::vector<btVector3> & controlPoints,
        IVehicle::VehicleCommands &    commands)
    {
      // does nothing!
    }
  private:
};

#endif
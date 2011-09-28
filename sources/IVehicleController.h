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
#ifndef IVEHICLECONTROLLER_H
#define IVEHICLECONTROLLER_H
#include <vector>
#include <btBulletDynamicsCommon.h>

#include "IVehicle.h"

struct SVehicleParameters
{
  btVector3 vehicleDirection;
  btVector3 vehicleRightDirection;
  btVector3 vehiclePosition;
  double    vehicleSpeed;
#ifdef ANALOG_CONTROLS
  double    throttle;
  double    steering;
#endif

};

class IVehicleController
{
  public: 
    enum VehicleAction
    {
      va_steerLeft=0,
      va_steerRight,
      va_accelerate,
      va_decelerate,
      va_brake,

      va_changeCamera,
      va_cameraUp,
      va_cameraDown,
      va_cameraLeft,
      va_cameraRight,

      va_action1,
      va_action2,
      va_action3,

      va_numActions
    };
    virtual void init(
        const std::vector<btVector3> & controlPoints,
        const btVector3 vehicleForward,
        const btVector3 startPosition) {  };

    virtual void updateCommands(
        const SVehicleParameters &     vehicleParameters,
        const std::vector<btVector3> & controlPoints,
        IVehicle::VehicleCommands &    commands)=0;

    virtual ~IVehicleController() { };

    static const char *  getActionDefaultString(unsigned actionId);
    const char *         getActionString(unsigned actionId) { return getActionDefaultString(actionId); }
    virtual void         getActionSettingString(unsigned actionId, 
                                    std::string & outString) { outString = ""; }
    virtual unsigned     getNumActions() { return 0; }

    virtual void        startControlVehicle()=0;
    virtual void        stopControlVehicle()=0;

    virtual void        startLearnAction(unsigned actionId) { };
    virtual void        stopLearnAction() { };
    virtual bool        isLearningAction() { return false; }
};

#endif

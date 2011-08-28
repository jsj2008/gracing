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

#include "IVehicleController.h"

static const char * actionNames[IVehicleController::va_numActions]=
{
  "steer left",
  "steer right",
  "accelerate",
  "decelerate",
  "brake",
  "undefined"
};

const char * IVehicleController::getActionString(unsigned actionId)
{
  if(actionId < va_numActions) 
    return actionNames[actionId];
  return "undefined";
}

unsigned IVehicleController::getActionId(const char * name)
{
  for(unsigned i=0; i<va_numActions; i++) 
    if(strcmp(name,actionNames[i])==0)
      return i;

  return va_numActions;
}

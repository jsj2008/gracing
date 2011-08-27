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
#include <irrlicht.h>
#include "JoystickInterface.h"
#include "ResourceManager.h"

using namespace irr;

static JoystickInterface *        instance=0;
static core::array<SJoystickInfo> devices;

JoystickInterface * JoystickInterface::build(irr::IrrlichtDevice * device)
{
  if(instance==0) 
    instance = new JoystickInterface(device);

  return instance;
}

JoystickInterface::JoystickInterface(irr::IrrlichtDevice * device)
{
  if(device->activateJoysticks(devices)) {
    GM_LOG("Joystick support is enabled and %d sticks are prenset\n",devices.size());

    for(u32 joystick = 0; joystick < devices.size(); ++joystick) {
      GM_LOG("Joystick %d:", joystick);
      GM_LOG("\tName: '%s''\n",devices[joystick].Name.c_str());
      GM_LOG("\tAxes: %d\n",devices[joystick].Axes);
      GM_LOG("\tButtons: %d\n",devices[joystick].Buttons);

      GM_LOG("\tHat is: ");

      switch(devices[joystick].PovHat) {
        case SJoystickInfo::POV_HAT_PRESENT:
          GM_LOG("present\n");
          break;

        case SJoystickInfo::POV_HAT_ABSENT:
          GM_LOG("absent\n");
          break;

        case SJoystickInfo::POV_HAT_UNKNOWN:
        default:
          GM_LOG("unknown\n");
          break;
      }
    }
  } else {
    GM_LOG("Joystick support is not enabled\n");
  }
}

bool JoystickInterface::checkAxis(const irr::SEvent::SJoystickEvent & joystickEvent,unsigned axis)
{
  assert(axis < irr::SEvent::SJoystickEvent::NUMBER_OF_AXES);

  if(joystickEvent.Axis[axis] != m_axisPrevValues[axis]) {
    GM_LOG("a joystick event: %d on axis %d\n",joystickEvent.Axis[axis],axis);
    m_axisPrevValues[axis]=joystickEvent.Axis[axis];
    return true;
  }
  return false;
}

#define ibp(_s,_b)   ((_s) & (1 << (_b)))
bool JoystickInterface::checkButton(const irr::SEvent::SJoystickEvent & joystickEvent,unsigned button)
{
  assert(button < irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS);

  unsigned mask = 1 << button;
  bool s=ibp(joystickEvent.ButtonStates,button);

  if(s != m_buttonsPrevValues[button]) {
    GM_LOG("a joystick event on button %d\n",button);
    m_buttonsPrevValues[button]=s;
    return true;
  }
  return false;
}


void JoystickInterface::joystickEvent(const irr::SEvent::SJoystickEvent & joystickEvent)
{
  for(unsigned i=0; i < irr::SEvent::SJoystickEvent::NUMBER_OF_AXES; i++)
    checkAxis(joystickEvent,i);

  for(unsigned i=0; i < irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; i++)
    checkButton(joystickEvent,i);
}


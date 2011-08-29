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
enum {
  ac_none,
  ac_button,
  ac_axis
};

#define AXIS_UP_THRESHOLD     32000
#define AXIS_DOWN_THRESHOLD  -32000


//////////////////////////////////////////////////////////////////////////////////////////////
class Joystick : public IVehicleController, public IEventListener
{
  public: 
  struct ConfigElement
  {
    unsigned   action;
    JoystickInterface::JoystickAction  joyact;

    ConfigElement() {
      action = ac_none;
      joyact.type = ac_none;
    }
  };
 
  ConfigElement m_actions[va_numActions];
  bool          m_controllingVehicle;
  JoystickInterface * m_owner;

  // gui actions
  enum 
  {
    ga_up=0,
    ga_down,
    ga_left,
    ga_right,
    ga_enter,
    ga_numGuiActions
  };
  ConfigElement m_guiActions[ga_numGuiActions];
  

  Joystick(JoystickInterface * i)
  {
    m_controllingVehicle=false;
    m_owner=i;
  };

  void setAction(unsigned vaction,
      unsigned type,
      bool     analog,
      int      index,
      int      value)
  {
    if(vaction < va_numActions) {
      m_actions[vaction].action = vaction;
      m_actions[vaction].joyact.type = type;
      m_actions[vaction].joyact.analog = analog;
      m_actions[vaction].joyact.index = index;
      m_actions[vaction].joyact.value = value;
      return;
    }
  }

  void setGuiAction(unsigned gaction,
      unsigned type,
      bool     analog,
      int      index,
      int      value)
  { 
    if(gaction < ga_numGuiActions) {
      m_guiActions[gaction].action = gaction;
      m_guiActions[gaction].joyact.type = type;
      m_guiActions[gaction].joyact.analog = analog;
      m_guiActions[gaction].joyact.index = index;
      m_guiActions[gaction].joyact.value = value;
      return;
    }
  }

  bool checkAction(ConfigElement el, const JoystickInterface::JoystickEvent & event)
  {
    switch(el.joyact.type)
    {
      case ac_button:
        break;
      case ac_axis:
        if(el.joyact.analog) {
        } else {
        }
        break;
    }

    return false;
  }

  void debugDumpActions()
  {
    for(unsigned i=0; i < va_numActions; i++) {
      char buffer[128];
      JoystickInterface::getActionString(buffer,128,m_actions[i].joyact);
      GM_LOG("  '%s' -> %s\n",IVehicleController::getActionString(i),buffer);
    }
  }

  void updateCommands(
    const SVehicleParameters &    parameters,
    const std::vector<btVector3> & controlPoints,
    IVehicle::VehicleCommands &    commands)
  {
  }

  void joystickEvent(const JoystickInterface::JoystickEvent & event)
  {
    if(m_controllingVehicle) {
    } else { // working for gui

      // check up direction
      ConfigElement & el=m_guiActions[ga_up];
    
      checkAction(el,event);

    }
  }
};
//////////////////////////////////////////////////////////////////////////////////////////////
JoystickInterface::JoystickInterface(irr::IrrlichtDevice *, const SJoystickInfo & device)
{
  m_name = device.Name.c_str();
  m_idJoystick = device.Joystick;


  // build default configuration:
  Joystick * joy;
  joy = new Joystick(this);
  joy->setGuiAction(Joystick::ga_up,ac_axis,false,3,1);
  joy->setGuiAction(Joystick::ga_down,ac_axis,false,3,0);
  joy->setGuiAction(Joystick::ga_left,ac_axis,false,2,0);
  joy->setGuiAction(Joystick::ga_right,ac_axis,false,2,1);

  GM_LOG("************** ACTIONS *********\n");
  joy->debugDumpActions();
}

std::string  JoystickInterface::getName()
{
  return m_name;
}

unsigned JoystickInterface::getNumController()
{
  return 0;
}

IVehicleController * JoystickInterface::getController(unsigned)
{
  return 0;
}

void JoystickInterface::getActionString(char * buffer, 
    int bufferSize, 
    JoystickAction & action)
{
  switch(action.type) {
    case ac_button:
      snprintf(buffer,bufferSize,"button %d %s",
          action.index,
          action.value==1 ? "pressed" : "release");
      break;
    case ac_axis:
      if(action.analog)
        snprintf(buffer,bufferSize,"axis %d value: %d",action.index,action.value);
      else {
        if(action.value > AXIS_UP_THRESHOLD)
          snprintf(buffer,bufferSize,"axis %d pressed up",action.index);
        else if(action.value < AXIS_DOWN_THRESHOLD)
          snprintf(buffer,bufferSize,"axis %d pressed down",action.index);
        else
          snprintf(buffer,bufferSize,"axis %d released",action.index);
      }
      break;
    case ac_none:
    default:
      snprintf(buffer,bufferSize,"not set");
      break;
  }
}

bool JoystickInterface::checkAxis(const JoystickEvent & joystickEvent,
                                  unsigned axis, irr::s16 & value)
{
  assert(axis < irr::SEvent::SJoystickEvent::NUMBER_OF_AXES);

  if(joystickEvent.Axis[axis] != m_axisPrevValues[axis]) {
    m_axisPrevValues[axis]=joystickEvent.Axis[axis];
    value = joystickEvent.Axis[axis];
    return true;
  }
  return false;
}

#define ibp(_s,_b)   ((_s) & (1 << (_b)))
bool JoystickInterface::checkButton(const JoystickEvent & joystickEvent,unsigned button,bool & pressed)
{
  assert(button < irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS);

  bool s=ibp(joystickEvent.ButtonStates,button);

  if(s != m_buttonsPrevValues[button]) {
    m_buttonsPrevValues[button]=s;
    pressed=s;
    return true;
  }
  return false;
}


bool JoystickInterface::getAction(
    JoystickAction & action, 
    const JoystickEvent & event)
{
  for(unsigned i=0; i < irr::SEvent::SJoystickEvent::NUMBER_OF_AXES; i++) {
    irr::s16 value;
    if(checkAxis(event,i,value)) {
      action.type = ac_axis;
      action.index = i;
      action.analog = false;
      action.value = value;
      return true;
    }
  }

  for(unsigned i=0; i < irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; i++) {
    bool pressed;
    if(checkButton(event,i,pressed)) {
      action.type = ac_button;
      action.index = i;
      action.analog = false;
      action.value = pressed? 1:0;
      return true;
    }
  }
  return false;
}

void JoystickInterface::joystickEvent(const JoystickEvent & joystickEvent)
{
  JoystickAction action;
  if(getAction(action,joystickEvent)) {
    char buffer[128];
    getActionString(buffer,128,action);
    GM_LOG("joystick action: %s\n",buffer);
  }
}


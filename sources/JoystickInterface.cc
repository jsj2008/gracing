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

enum {
  naxes= irr::SEvent::SJoystickEvent::NUMBER_OF_AXES,
  nbuttons= irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS
};

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

    irr::s16  m_axisPrevValues[irr::SEvent::SJoystickEvent::NUMBER_OF_AXES];
    bool      m_buttonsPrevValues[irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];

    // gui actions
    enum 
    {
      ga_none=0,
      ga_up,
      ga_down,
      ga_left,
      ga_right,
      ga_enter,
      ga_esc,
      ga_numGuiActions
    };

    static const char * m_guiActionName[ga_numGuiActions];
    //ConfigElement m_guiActions[ga_numGuiActions];
    irr::s16 m_guiAxisUpAction[naxes];
    irr::s16 m_guiAxisDownAction[naxes];
    bool     m_guiAxisMustRelease[naxes];
    int      m_guiAxisMustReleaseKey[naxes];
    irr::s16 m_guiButtonAction[nbuttons];

    void startControlVehicle() {
      m_controllingVehicle=true;
    }
    void stopControlVehicle()  { 
      m_controllingVehicle=false;
    }


    Joystick(JoystickInterface * owner)
    {
      m_controllingVehicle=false;
      m_owner=owner;
      for(unsigned i=0; i<naxes; i++) 
        m_guiAxisMustRelease[i]=false;
      clearGuiActions();
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

    void clearGuiActions()
    {
      for(unsigned i=0; i<naxes; i++) {
        m_guiAxisUpAction[i]=ga_none;
        m_guiAxisDownAction[i]=ga_none;
      }

      for(unsigned i=0; i<nbuttons; i++) {
        m_guiButtonAction[i]=ga_none;
      }
    }

    void setGuiAction(unsigned gaction,
        unsigned type,
        bool     analog,
        int      index,
        int      value)
    { 

      if(type ==  ac_axis && value == 0 && index >= 0 && index < naxes) {
        m_guiAxisDownAction[index]=gaction;
      } else if(type == ac_axis && value == 1 && index >=0 && index < naxes) {
        m_guiAxisUpAction[index]=gaction;
      } else if(type == ac_button && index >0 && index < nbuttons) {
        m_guiButtonAction[index]=gaction;
      }
    }

    #define ibp(_s,_b)   ((_s) & (1 << (_b)))
    bool checkButton(const JoystickInterface::JoystickEvent & joystickEvent,unsigned button,bool & pressed)
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


    bool checkAxis(const JoystickInterface::JoystickEvent & joystickEvent,
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

    void generateAction(irr::s16 action, bool pressed)
    {
      irr::SEvent event;
      memset(&event,0,sizeof(event));
      event.EventType =  EET_KEY_INPUT_EVENT;
      event.KeyInput.Control = 0;
      event.KeyInput.Shift = 0;
      event.KeyInput.PressedDown = pressed;

      switch(action) {
        case ga_up:
          event.KeyInput.Key = irr::KEY_UP;
          break;
        case ga_down:
          event.KeyInput.Key = irr::KEY_DOWN;
          break;
        case ga_left:
          event.KeyInput.Key = irr::KEY_LEFT;
          break;
        case ga_right:
          event.KeyInput.Key = irr::KEY_RIGHT;
          break;
        case ga_enter:
          event.KeyInput.Key = irr::KEY_RETURN;
          break;
        case ga_esc:
          event.KeyInput.Key = irr::KEY_ESCAPE;
          break;
      }
      ResourceManager::getInstance()->getDevice()->postEventFromUser(event);
    }

    void joystickEventGui(const JoystickInterface::JoystickEvent & event)
    {
      for(unsigned i=0; i < naxes; i++) {
        irr::s16 value;
        if(checkAxis(event,i,value)) {
          if(value > AXIS_UP_THRESHOLD && m_guiAxisUpAction[i] !=ga_none && !m_guiAxisMustRelease[i]) {
            generateAction(m_guiAxisUpAction[i],true);
            m_guiAxisMustRelease[i]=true;
            m_guiAxisMustReleaseKey[i]=m_guiAxisUpAction[i];
          } else if(value < AXIS_DOWN_THRESHOLD && m_guiAxisDownAction[i] !=ga_none && !m_guiAxisMustRelease[i]) {
            generateAction(m_guiAxisDownAction[i],true);
            m_guiAxisMustReleaseKey[i]=m_guiAxisDownAction[i];
            m_guiAxisMustRelease[i]=true;
          } else if(value > AXIS_DOWN_THRESHOLD && value < AXIS_UP_THRESHOLD && m_guiAxisMustRelease[i]) {
            m_guiAxisMustRelease[i]=false;
            generateAction(m_guiAxisMustReleaseKey[i],false);
          }
        }
      }

      for(unsigned i=0; i < nbuttons; i++) {
        bool pressed;
        if(checkButton(event,i,pressed) && m_guiButtonAction[i] != ga_none) 
          generateAction(m_guiButtonAction[i],pressed);
      }
    }

    void joystickEvent(const JoystickInterface::JoystickEvent & event)
    {
      if(m_controllingVehicle) {
      } else {
        joystickEventGui(event);
      }
    }

};

const char * Joystick::m_guiActionName[ga_numGuiActions]={
      "up","down","left","right","enter","esc"
};
//////////////////////////////////////////////////////////////////////////////////////////////
JoystickInterface::JoystickInterface(irr::IrrlichtDevice *, const SJoystickInfo & device)
{
  m_name = device.Name.c_str();
  m_idJoystick = device.Joystick;

  // build default configuration:
  Joystick * joy;
  joy = new Joystick(this);
  joy->setGuiAction(Joystick::ga_up,ac_axis,false,3,0);
  joy->setGuiAction(Joystick::ga_down,ac_axis,false,3,1);
  joy->setGuiAction(Joystick::ga_left,ac_axis,false,2,0);
  joy->setGuiAction(Joystick::ga_right,ac_axis,false,2,1);
  joy->setGuiAction(Joystick::ga_enter,ac_button,false,2,0);
  
  ResourceManager::getInstance()->getEventReceiver()->addListener(joy);

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


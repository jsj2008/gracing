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

#ifndef ANALOG_CONTROLS
#error Digital control not implemented for joysticks
#endif

CFG_PARAM_D(glob_joystick_throttleIncrement)=0.2; // min: .2, max:
CFG_PARAM_D(glob_joystick_throttleDecrement)=0.01;
CFG_PARAM_D(glob_joystick_steeringIncrement)=0.005f;

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

    ConfigElement       m_actions[va_numActions];
    bool                m_controllingVehicle;
    bool                m_isLearning;
    unsigned            m_actionToLearn;
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
    //irr::s16 m_guiAxisUpAction[naxes];
    //irr::s16 m_guiAxisDownAction[naxes];
    //irr::s16 m_guiButtonAction[nbuttons];
    unsigned m_guiAxisUpAction[naxes];
    unsigned m_guiAxisDownAction[naxes];
    unsigned m_guiButtonAction[nbuttons];
    bool     m_guiAxisMustRelease[naxes];
    int      m_guiAxisMustReleaseKey[naxes];

    void startControlVehicle()
    {
      m_controllingVehicle=true;
    }

    void stopControlVehicle()
    { 
      m_controllingVehicle=false;
    }

    void startLearnAction(unsigned actionId) 
    { 
      m_isLearning=true;
      m_actionToLearn=actionId;
    };

    void stopLearnAction() 
    { 
      m_isLearning=false;
    };

    bool isLearningAction() { return m_isLearning; }

    virtual void init(
        const std::vector<btVector3> & controlPoints,
        const btVector3 vehicleForward,
        const btVector3 startPosition) 
    { 
      startControlVehicle();
    };

    Joystick(JoystickInterface * owner)
    {
      m_controllingVehicle=false;
      m_owner=owner;
      m_isLearning=false;
      for(unsigned i=0; i<naxes; i++) 
        m_guiAxisMustRelease[i]=false;
      clearActions();
      clearGuiActions();
      ResourceManager::getInstance()->getEventReceiver()->addListener(this);
    };

    ~Joystick()
    {
      ResourceManager::getInstance()->getEventReceiver()->removeListener(this);
    }

    void getAction(unsigned vaction,
        unsigned & type,
        bool     & analog,
        int      & index,
        int      & value)
    {
      if(vaction < va_numActions) {
        type=m_actions[vaction].joyact.type;
        analog=m_actions[vaction].joyact.analog;
        index=m_actions[vaction].joyact.index;
        value=m_actions[vaction].joyact.value;
      }
    }

    const char * getActionString(unsigned actionId) 
    { 
      if(actionId < va_numActions) {
        return IVehicleController::getActionDefaultString(actionId);
      } 
      actionId -= va_numActions;
      if(actionId < ga_numGuiActions) {
        return m_guiActionName[actionId];
      }
      return "";
    }

    void getActionSettingString(unsigned actionId, std::string & outString)
    {
      char b1[128];
      char b2[128];
      if(actionId < va_numActions) {
        JoystickInterface::getActionString(b1,128,m_actions[actionId].joyact);
        snprintf(b2,128," %s %s",IVehicleController::getActionString(actionId),b1);
        outString = b2;
        return;
      } 
      actionId -= va_numActions;
      if(actionId < ga_numGuiActions) {
        bool found=false;

        for(unsigned i=0; i < naxes; i++) {
          if(m_guiAxisDownAction[i]==actionId) {
            snprintf(b1,128,"axis %d down",i);
            found=true;
            break;
          };
          if(m_guiAxisUpAction[i]==actionId) {
            snprintf(b1,128,"axis %d up",i);
            found=true;
            break;
          }
        }

        if(!found) {
          for(unsigned i=0; i < nbuttons; i++) 
            if(m_guiButtonAction[i]==actionId) {
              snprintf(b1,127,"button %d",i);
              found=true;
              break;
            }
        }

        if(!found) 
          snprintf(b1,128,"not found");
        snprintf(b2,128,"%s %s", m_guiActionName[actionId], b1);
        outString = b2;
        return;
      }
      outString = "";
    }

    unsigned getNumActions()
    {
      return va_numActions + ga_numGuiActions;
    }

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

      vaction -= va_numActions;
      if(vaction < ga_numGuiActions) {
        if(value < 0) 
          value = 0;
        else if(value > 0)
          value = 1;
          
        setGuiAction(vaction,type,analog,index,value);
      }
    }

    void clearActions()
    {
      for(unsigned i=0; i<va_numActions; i++) {
        memset(&m_actions[i],0,sizeof(m_actions[i]));
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
      GM_LOG("SETTING GUI action: %d, type: %d,analog:%d, index:%d, value:%d\n",
          gaction,type,analog,index,value);
      if(type ==  ac_axis && value == 0 && index >= 0 && index < naxes) {
        m_guiAxisDownAction[index]=gaction;
      } else if(type == ac_axis && value == 1 && index >=0 && index < naxes) {
        m_guiAxisUpAction[index]=gaction;
      } else if(type == ac_button && index >0 && index < nbuttons) {
        m_guiButtonAction[index]=gaction;
      }
    }

    void getGuiAction(unsigned gaction,
        unsigned & type,
        bool     & analog,
        int      & index,
        int      & value)
    {
      type = 0;
      value = 0;
      index = 0;
      analog = 0;
      for(unsigned i=0; i<naxes; i++) {
        if(m_guiAxisDownAction[i] == gaction) {
          type = ac_axis;
          value = 0;
          analog = 0;
          index = i;
          return;
        }
        if(m_guiAxisUpAction[i] == gaction) {
          type = ac_axis;
          value = 1;
          analog = 0;
          index = i;
          return;
        }
        if(m_guiButtonAction[i] == gaction) {
          type = ac_button;
          value = 0;
          index = i;
          analog = 0;
          return;
        }
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
        GM_LOG("  '%s' [%d] -> %s\n",IVehicleController::getActionString(i),i,buffer);
      }
    }

    inline bool checkAction(JoystickInterface::JoystickAction & ac)
    {
      switch(ac.type) {
        case ac_axis:
          if((ac.value == 1 && m_axisPrevValues[ac.index] > AXIS_UP_THRESHOLD)  ||
              (ac.value == 0 && m_axisPrevValues[ac.index] < AXIS_DOWN_THRESHOLD)) {
            return true;
          } 
          break;
        case ac_button:
          if(m_buttonsPrevValues[ac.index]) {
            return true;
          }
          break;
      }
      return false;
    }

    inline void executeAction(JoystickInterface::JoystickAction & ac,
        double in_variable,
        double & out_variable,
        double increment)
    {
      switch(ac.type) {
        case ac_axis:
          if((ac.value == 1 && m_axisPrevValues[ac.index] > AXIS_UP_THRESHOLD)  ||
              (ac.value == 0 && m_axisPrevValues[ac.index] < AXIS_DOWN_THRESHOLD)) {
            out_variable = in_variable + increment;
            return;
          } 
          break;
        case ac_button:
          if(m_buttonsPrevValues[ac.index]) {
            out_variable = in_variable + increment;
            return;
          }
          break;
      }
#if 0
      if(in_variable > glob_steeringIncrement)
        out_variable = in_variable - increment;
      else if(in_variable < -glob_steeringIncrement) 
        out_variable = in_variable + increment;
#endif
    }

    void updateCommands(
        const SVehicleParameters &    parameters,
        const std::vector<btVector3> & controlPoints,
        IVehicle::VehicleCommands &    commands)
    {
      if(!m_controllingVehicle)
        return ;
      // accelerate

      executeAction(
        m_actions[va_accelerate].joyact,
        parameters.throttle,
        commands.throttling,
        glob_joystick_throttleIncrement);

      executeAction(
        m_actions[va_decelerate].joyact,
        parameters.throttle,
        commands.throttling,
        -glob_joystick_throttleIncrement);

      executeAction(
        m_actions[va_steerLeft].joyact,
        parameters.steering,
        commands.steering,
        -glob_joystick_steeringIncrement);

      executeAction(
        m_actions[va_steerRight].joyact,
        parameters.steering,
        commands.steering,
        glob_joystick_steeringIncrement);

      commands.changeCamera=checkAction(m_actions[va_changeCamera].joyact);

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
          if(value > AXIS_UP_THRESHOLD && 
              m_guiAxisUpAction[i] !=ga_none && 
              !m_guiAxisMustRelease[i]) {
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

    void learnAction(const JoystickInterface::JoystickEvent & event)
    {
      for(unsigned i=0; i < naxes; i++) {
        irr::s16 value;
        if(checkAxis(event,i,value)) {
          GM_LOG("learned action: axis %d value %d\n",i,value);
          m_isLearning = false;
          value = value < 0 ? 0 : 1;
          setAction(m_actionToLearn, ac_axis, false,i,value);
          return;
        }
      }

      for(unsigned i=0; i < nbuttons; i++) {
        bool pressed;
        if(checkButton(event,i,pressed) && pressed) {
          GM_LOG("learned action: button %d\n",i);
          m_isLearning = false;
          setAction(m_actionToLearn,ac_button,false,i,0);
          return;
        }
      }
    }

    void joystickEvent(const JoystickInterface::JoystickEvent & event)
    {
#if 0
      for(unsigned i=0; i < naxes; i++) {
        irr::s16 value;
        if(checkAxis(event,i,value)) {
          GM_LOG("learned action: axis %d value %d\n",i,value);
        }
      }
      for(unsigned i=0; i < nbuttons; i++) {
        bool pressed;
        if(checkButton(event,i,pressed)) {
          GM_LOG("learned action: button %d pressed: %s\n",i,pressed?"yes":"no");
        }
      }
#endif
      if(m_isLearning) {
        learnAction(event);
      } if(m_controllingVehicle) {
        bool dummyBool;
        irr::s16 dummyS16;
        for(unsigned i=0; i<nbuttons; i++) 
          checkButton(event,i,dummyBool);
        for(unsigned i=0; i<naxes; i++)
          checkAxis(event,i,dummyS16);
      } else 
        joystickEventGui(event);
    }
};

const char * Joystick::m_guiActionName[ga_numGuiActions]={
      "gui none", "gui up","gui down","gui left","gui right","gui enter","gui esc"
};
//////////////////////////////////////////////////////////////////////////////////////////////
JoystickInterface::JoystickInterface(irr::IrrlichtDevice *, const SJoystickInfo & device)
{
  m_name = device.Name.c_str();
  m_idJoystick = device.Joystick;

  // build default configuration:
  Joystick * joy;
  joy = new Joystick(this);
  m_controllers.push_back(joy);

#if 0
  joy->setGuiAction(Joystick::ga_up,ac_axis,false,3,0);
  joy->setGuiAction(Joystick::ga_down,ac_axis,false,3,1);
  joy->setGuiAction(Joystick::ga_left,ac_axis,false,2,0);
  joy->setGuiAction(Joystick::ga_right,ac_axis,false,2,1);
  joy->setGuiAction(Joystick::ga_enter,ac_button,false,2,0);

  joy->setAction(IVehicleController::va_accelerate,ac_button,false,2,0);
  joy->setAction(IVehicleController::va_decelerate,ac_button,false,1,0);
  joy->setAction(IVehicleController::va_steerLeft,ac_axis,false,2,0);
  joy->setAction(IVehicleController::va_steerRight,ac_axis,false,2,1);
  GM_LOG("************** ACTIONS *********\n");
  joy->debugDumpActions();
#endif
}

std::string  JoystickInterface::getName()
{
  return m_name;
}

unsigned JoystickInterface::getNumController()
{
  return m_controllers.size();
}

IVehicleController * JoystickInterface::getController(unsigned index)
{
  if(index < m_controllers.size())
    return m_controllers[index];
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
        if(action.value == 1)
          snprintf(buffer,bufferSize,"axis %d pressed up",action.index);
        else if(action.value == 0)
          snprintf(buffer,bufferSize,"axis %d pressed down",action.index);
        else 
          snprintf(buffer,bufferSize,"axis %d %d???? ",action.index,action.value);
#if 0
        if(action.value > AXIS_UP_THRESHOLD)
          snprintf(buffer,bufferSize,"axis %d pressed up %d",action.index,action.value);
        else if(action.value < AXIS_DOWN_THRESHOLD)
          snprintf(buffer,bufferSize,"axis %d pressed down",action.index,action.value);
        else
          snprintf(buffer,bufferSize,"axis %d rel %d",action.index,action.value);
#endif
      }
      break;
    case ac_none:
    default:
      snprintf(buffer,bufferSize,"not set");
      break;
  }
}

void JoystickInterface::setConfiguration(XmlNode * root)
{
  std::vector<XmlNode*> devices;
  unsigned type;
  bool     analog;
  int      index;
  unsigned action;
  int      value;

  for(unsigned i=0; i<m_controllers.size(); i++) {
    delete m_controllers[i];
  }
  m_controllers.clear();

  root->getChildren("device",devices);
  for(unsigned i=0; i<devices.size(); i++) {
    Joystick * joy= new Joystick(this);
    m_controllers.push_back(joy);
    std::vector<XmlNode*> actions;
    devices[i]->getChildren("action",actions);
    for(unsigned j=0; j<actions.size(); j++) {
      XmlNode * act=actions[j];
      act->get("action",action);
      act->get("type",type);
      act->get("analog",analog);
      act->get("index",index);
      act->get("value",value);
      joy->setAction(action,type,analog,index,value);
    }
    actions.clear();
    devices[i]->getChildren("gui-action",actions);
    for(unsigned j=0; j<actions.size(); j++) {
      XmlNode * act=actions[j];
      act->get("action",action);
      act->get("type",type);
      act->get("analog",analog);
      act->get("index",index);
      act->get("value",value);
      joy->setGuiAction(action,type,analog,index,value);
    }
    GM_LOG("************** ACTIONS *********\n");
    joy->debugDumpActions();
  }
}

void JoystickInterface::addController()
{
  Joystick * joy;
  joy = new Joystick(this);
  m_controllers.push_back(joy);
}

void JoystickInterface::getConfiguration(XmlNode * root)
{
  root->deleteAllChildren();
  unsigned type;
  bool     analog;
  int      index;
  int      value;

  for(unsigned i=0; i< m_controllers.size(); i++) {
    Joystick * joystick=
      static_cast<Joystick*>(m_controllers[i]);
    XmlNode * node=root->addChild("device");

    for(unsigned j=0; j < IVehicleController::va_numActions; j++) {
      XmlNode * act=node->addChild("action");
      joystick->getAction(j,type,analog,index,value);
      act->set("action",j);
      act->set("type",type);
      act->set("analog",analog);
      act->set("index",index);
      act->set("value",value);
    }

    for(unsigned j=0; j < Joystick::ga_numGuiActions; j++) {
      XmlNode * act=node->addChild("gui-action");
      joystick->getGuiAction(j,type,analog,index,value);
      act->set("action",j);
      act->set("type",type);
      act->set("analog",analog);
      act->set("index",index);
      act->set("value",value);
      //act->set("aname", Joystick::m_guiActionName[j]);
    }
  }
}


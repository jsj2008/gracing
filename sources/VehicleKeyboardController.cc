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
#include <assert.h>
#include "EventReceiver.h"
#include "VehicleKeyboardController.h"

#define  KEYBOARD_INTERFACE_CONTROLLER_NAME "keyboard"

#define  NOT_DEFINED "not defined"

static const char * keyDescr[256]={
  /* 0x00 */ NOT_DEFINED,
  /* 0x01 */ "LBUTTON",
  /* 0x02 */ "RBUTTON",
  /* 0x03 */ "CANCEL",
  /* 0x04 */ "MBUTTON",
  /* 0x05 */ "XBUTTON1",
  /* 0x06 */ "XBUTTON2",
  /* 0x07 */ NOT_DEFINED,
  /* 0x08 */ "BACK",
  /* 0x09 */ "TAB",
  /* 0x0a */ NOT_DEFINED,
  /* 0x0b */ NOT_DEFINED,
  /* 0x0c */ "CLEAR",
  /* 0x0d */ "RETURN",
  /* 0x0e */ NOT_DEFINED,
  /* 0x0f */ NOT_DEFINED,
  /* 0x10 */ "SHIFT",
  /* 0x11 */ "CONTROL",
  /* 0x12 */ "MENU",
  /* 0x13 */ "PAUSE",
  /* 0x14 */ "CAPITAL",
  /* 0x15 */ "HANGUL",
  /* 0x16 */ NOT_DEFINED,
  /* 0x17 */ "JUNJA",
  /* 0x18 */ "FINAL",
  /* 0x19 */ "KANJI",
  /* 0x1a */ NOT_DEFINED,
  /* 0x1b */ "ESCAPE",
  /* 0x1c */ "CONVERT",
  /* 0x1d */ "NONCONVERT",
  /* 0x1e */ "ACCEPT",
  /* 0x1f */ "MODECHANGE",
  /* 0x20 */ "SPACE",
  /* 0x21 */ "PRIOR",
  /* 0x22 */ "NEXT",
  /* 0x23 */ "END",
  /* 0x24 */ "HOME",
  /* 0x25 */ "LEFT",
  /* 0x26 */ "UP",
  /* 0x27 */ "RIGHT",
  /* 0x28 */ "DOWN",
  /* 0x29 */ "SELECT",
  /* 0x2a */ "PRINT",
  /* 0x2b */ "EXECUT",
  /* 0x2c */ "SNAPSHOT",
  /* 0x2d */ "INSERT",
  /* 0x2e */ "DELETE",
  /* 0x2f */ "HELP",
  /* 0x30 */ "KEY_0",
  /* 0x31 */ "KEY_1",
  /* 0x32 */ "KEY_2",
  /* 0x33 */ "KEY_3",
  /* 0x34 */ "KEY_4",
  /* 0x35 */ "KEY_5",
  /* 0x36 */ "KEY_6",
  /* 0x37 */ "KEY_7",
  /* 0x38 */ "KEY_8",
  /* 0x39 */ "KEY_9",
  /* 0x3a */ NOT_DEFINED,
  /* 0x3b */ NOT_DEFINED,
  /* 0x3c */ NOT_DEFINED,
  /* 0x3d */ NOT_DEFINED,
  /* 0x3e */ NOT_DEFINED,
  /* 0x3f */ NOT_DEFINED,
  /* 0x40 */ NOT_DEFINED,
  /* 0x41 */ "KEY_A",
  /* 0x42 */ "KEY_B",
  /* 0x43 */ "KEY_C",
  /* 0x44 */ "KEY_D",
  /* 0x45 */ "KEY_E",
  /* 0x46 */ "KEY_F",
  /* 0x47 */ "KEY_G",
  /* 0x48 */ "KEY_H",
  /* 0x49 */ "KEY_I",
  /* 0x4a */ "KEY_J",
  /* 0x4b */ "KEY_K",
  /* 0x4c */ "KEY_L",
  /* 0x4d */ "KEY_M",
  /* 0x4e */ "KEY_N",
  /* 0x4f */ "KEY_O",
  /* 0x50 */ "KEY_P",
  /* 0x51 */ "KEY_Q",
  /* 0x52 */ "KEY_R",
  /* 0x53 */ "KEY_S",
  /* 0x54 */ "KEY_T",
  /* 0x55 */ "KEY_U",
  /* 0x56 */ "KEY_V",
  /* 0x57 */ "KEY_W",
  /* 0x58 */ "KEY_X",
  /* 0x59 */ "KEY_Y",
  /* 0x5a */ "KEY_Z",
  /* 0x5b */ "LWIN",
  /* 0x5c */ "RWIN",
  /* 0x5d */ "APPS",
  /* 0x5e */ NOT_DEFINED,
  /* 0x5f */ "SLEEP",
  /* 0x60 */ "NUMPAD0",
  /* 0x61 */ "NUMPAD1",
  /* 0x62 */ "NUMPAD2",
  /* 0x63 */ "NUMPAD3",
  /* 0x64 */ "NUMPAD4",
  /* 0x65 */ "NUMPAD5",
  /* 0x66 */ "NUMPAD6",
  /* 0x67 */ "NUMPAD7",
  /* 0x68 */ "NUMPAD8",
  /* 0x69 */ "NUMPAD9",
  /* 0x6a */ "MULTIPLY",
  /* 0x6b */ "ADD",
  /* 0x6c */ "SEPARATOR",
  /* 0x6d */ "SUBTRACT",
  /* 0x6e */ "DECIMAL",
  /* 0x6f */ "DIVIDE",
  /* 0x70 */ "F1",
  /* 0x71 */ "F2",
  /* 0x72 */ "F3",
  /* 0x73 */ "F4",
  /* 0x74 */ "F5",
  /* 0x75 */ "F6",
  /* 0x76 */ "F7",
  /* 0x77 */ "F8",
  /* 0x78 */ "F9",
  /* 0x79 */ "F10",
  /* 0x7a */ "F11",
  /* 0x7b */ "F12",
  /* 0x7c */ "F13",
  /* 0x7d */ "F14",
  /* 0x7e */ "F15",
  /* 0x7f */ "F16",
  /* 0x80 */ "F17",
  /* 0x81 */ "F18",
  /* 0x82 */ "F19",
  /* 0x83 */ "F20",
  /* 0x84 */ "F21",
  /* 0x85 */ "F22",
  /* 0x86 */ "F23",
  /* 0x87 */ "F24",
  /* 0x88 */ NOT_DEFINED,
  /* 0x89 */ NOT_DEFINED,
  /* 0x8a */ NOT_DEFINED,
  /* 0x8b */ NOT_DEFINED,
  /* 0x8c */ NOT_DEFINED,
  /* 0x8d */ NOT_DEFINED,
  /* 0x8e */ NOT_DEFINED,
  /* 0x8f */ NOT_DEFINED,
  /* 0x90 */ "NUMLOCK",
  /* 0x91 */ "SCROLL",
  /* 0x92 */ NOT_DEFINED,
  /* 0x93 */ NOT_DEFINED,
  /* 0x94 */ NOT_DEFINED,
  /* 0x95 */ NOT_DEFINED,
  /* 0x96 */ NOT_DEFINED,
  /* 0x97 */ NOT_DEFINED,
  /* 0x98 */ NOT_DEFINED,
  /* 0x99 */ NOT_DEFINED,
  /* 0x9a */ NOT_DEFINED,
  /* 0x9b */ NOT_DEFINED,
  /* 0x9c */ NOT_DEFINED,
  /* 0x9d */ NOT_DEFINED,
  /* 0x9e */ NOT_DEFINED,
  /* 0x9f */ NOT_DEFINED,
  /* 0xa0 */ "LSHIFT",
  /* 0xa1 */ "RSHIFT",
  /* 0xa2 */ "LCONTROL",
  /* 0xa3 */ "RCONTROL",
  /* 0xa4 */ "LMENU",
  /* 0xa5 */ "RMENU",
  /* 0xa6 */ NOT_DEFINED,
  /* 0xa7 */ NOT_DEFINED,
  /* 0xa8 */ NOT_DEFINED,
  /* 0xa9 */ NOT_DEFINED,
  /* 0xaa */ NOT_DEFINED,
  /* 0xab */ NOT_DEFINED,
  /* 0xac */ NOT_DEFINED,
  /* 0xad */ NOT_DEFINED,
  /* 0xae */ NOT_DEFINED,
  /* 0xaf */ NOT_DEFINED,
  /* 0xb0 */ NOT_DEFINED,
  /* 0xb1 */ NOT_DEFINED,
  /* 0xb2 */ NOT_DEFINED,
  /* 0xb3 */ NOT_DEFINED,
  /* 0xb4 */ NOT_DEFINED,
  /* 0xb5 */ NOT_DEFINED,
  /* 0xb6 */ NOT_DEFINED,
  /* 0xb7 */ NOT_DEFINED,
  /* 0xb8 */ NOT_DEFINED,
  /* 0xb9 */ NOT_DEFINED,
  /* 0xba */ NOT_DEFINED,
  /* 0xbb */ "PLUS",
  /* 0xbc */ "COMMA",
  /* 0xbd */ "MINUS",
  /* 0xbe */ "PERIOD",
  /* 0xbf */ NOT_DEFINED,
  /* 0xc0 */ NOT_DEFINED,
  /* 0xc1 */ NOT_DEFINED,
  /* 0xc2 */ NOT_DEFINED,
  /* 0xc3 */ NOT_DEFINED,
  /* 0xc4 */ NOT_DEFINED,
  /* 0xc5 */ NOT_DEFINED,
  /* 0xc6 */ NOT_DEFINED,
  /* 0xc7 */ NOT_DEFINED,
  /* 0xc8 */ NOT_DEFINED,
  /* 0xc9 */ NOT_DEFINED,
  /* 0xca */ NOT_DEFINED,
  /* 0xcb */ NOT_DEFINED,
  /* 0xcc */ NOT_DEFINED,
  /* 0xcd */ NOT_DEFINED,
  /* 0xce */ NOT_DEFINED,
  /* 0xcf */ NOT_DEFINED,
  /* 0xd0 */ NOT_DEFINED,
  /* 0xd1 */ NOT_DEFINED,
  /* 0xd2 */ NOT_DEFINED,
  /* 0xd3 */ NOT_DEFINED,
  /* 0xd4 */ NOT_DEFINED,
  /* 0xd5 */ NOT_DEFINED,
  /* 0xd6 */ NOT_DEFINED,
  /* 0xd7 */ NOT_DEFINED,
  /* 0xd8 */ NOT_DEFINED,
  /* 0xd9 */ NOT_DEFINED,
  /* 0xda */ NOT_DEFINED,
  /* 0xdb */ NOT_DEFINED,
  /* 0xdc */ NOT_DEFINED,
  /* 0xdd */ NOT_DEFINED,
  /* 0xde */ NOT_DEFINED,
  /* 0xdf */ NOT_DEFINED,
  /* 0xe0 */ NOT_DEFINED,
  /* 0xe1 */ NOT_DEFINED,
  /* 0xe2 */ NOT_DEFINED,
  /* 0xe3 */ NOT_DEFINED,
  /* 0xe4 */ NOT_DEFINED,
  /* 0xe5 */ NOT_DEFINED,
  /* 0xe6 */ NOT_DEFINED,
  /* 0xe7 */ NOT_DEFINED,
  /* 0xe8 */ NOT_DEFINED,
  /* 0xe9 */ NOT_DEFINED,
  /* 0xea */ NOT_DEFINED,
  /* 0xeb */ NOT_DEFINED,
  /* 0xec */ NOT_DEFINED,
  /* 0xed */ NOT_DEFINED,
  /* 0xee */ NOT_DEFINED,
  /* 0xef */ NOT_DEFINED,
  /* 0xf0 */ NOT_DEFINED,
  /* 0xf1 */ NOT_DEFINED,
  /* 0xf2 */ NOT_DEFINED,
  /* 0xf3 */ NOT_DEFINED,
  /* 0xf4 */ NOT_DEFINED,
  /* 0xf5 */ NOT_DEFINED,
  /* 0xf6 */ "ATTN",
  /* 0xf7 */ "CRSEL",
  /* 0xf8 */ "EXSEL",
  /* 0xf9 */ "EREOF",
  /* 0xfa */ "PLAY",
  /* 0xfb */ "ZOOM",
  /* 0xfc */ NOT_DEFINED,
  /* 0xfd */ "PA1",
  /* 0xfe */ "OEM_CLEAR",
  /* 0xff */ NOT_DEFINED
};
class VehicleKeyboardController : public IVehicleController
{
  public: 
    VehicleKeyboardController(EventReceiver * receiver);
    virtual void updateCommands(
        const SVehicleParameters  &    vehicleParameters,
        const std::vector<btVector3> & controlPoints,
        IVehicle::VehicleCommands &    commands);


    void        startControlVehicle() { }
    void        stopControlVehicle()  { }

    void        setKeyForAction(unsigned action, unsigned key);
    unsigned    getKeyForAction(unsigned action);
    virtual void getActionSettingString(unsigned actionId, std::string & outString);
    unsigned    getNumActions();

  private:
    EventReceiver * m_eventReceiver;

    irr::EKEY_CODE m_keyForAction[va_numActions];
};
VehicleKeyboardController::VehicleKeyboardController(EventReceiver * receiver)
{
  m_eventReceiver=receiver;
  for(unsigned i=0; i<va_numActions; i++) m_keyForAction[i]=(irr::EKEY_CODE)0;
}

void VehicleKeyboardController::getActionSettingString(unsigned actionId, std::string & outString)
{
  const char * base=getActionString(actionId);
  unsigned keycode=getKeyForAction(actionId);
  char  buffer[128];
  const char * kk;
  if(keycode < 256)
    kk=keyDescr[keycode];
  else
    kk=NOT_DEFINED;
  snprintf(buffer,128,"%s %s",base,kk);
  outString = buffer;
}


void VehicleKeyboardController::setKeyForAction(unsigned action, unsigned key)
{
  if(action < va_numActions) {
    m_keyForAction[action]=(irr::EKEY_CODE)key;
  }
}

unsigned VehicleKeyboardController::getNumActions()
{
  return va_numActions;
}

unsigned VehicleKeyboardController::getKeyForAction(unsigned action)
{
  if(action < va_numActions)
    return m_keyForAction[action];
  return 0;
}

void VehicleKeyboardController::updateCommands(
    const SVehicleParameters &    parameters,
   const std::vector<btVector3> & controlPoints,
   IVehicle::VehicleCommands &    commands)
{
  assert(m_eventReceiver);
  if(m_eventReceiver->IsKeyDown(m_keyForAction[va_accelerate])) {
    commands.throttling=1.;
  }

  if(m_eventReceiver->IsKeyDown(m_keyForAction[va_decelerate])) {
    commands.throttling=-1.;
  }

  if(m_eventReceiver->IsKeyDown(m_keyForAction[va_steerLeft])) {
    commands.steering=IVehicle::VehicleCommands::steerLeft;
  }

  if(m_eventReceiver->IsKeyDown(m_keyForAction[va_steerRight])) {
    commands.steering=IVehicle::VehicleCommands::steerRite;
  }
}

KeyboardInterface::KeyboardInterface(EventReceiver * er)
{
  VehicleKeyboardController * controller;
  controller = new VehicleKeyboardController(er);
  controller->setKeyForAction(IVehicleController::va_accelerate,irr::KEY_UP);
  controller->setKeyForAction(IVehicleController::va_decelerate,irr::KEY_DOWN);
  controller->setKeyForAction(IVehicleController::va_steerLeft,irr::KEY_LEFT);
  controller->setKeyForAction(IVehicleController::va_steerRight,irr::KEY_RIGHT);
  m_controllers.push_back(controller);

  controller = new VehicleKeyboardController(er);
  controller->setKeyForAction(IVehicleController::va_accelerate,irr::KEY_KEY_W);
  controller->setKeyForAction(IVehicleController::va_decelerate,irr::KEY_KEY_Z);
  controller->setKeyForAction(IVehicleController::va_steerLeft,irr::KEY_KEY_S);
  controller->setKeyForAction(IVehicleController::va_steerRight,irr::KEY_KEY_A);
  m_controllers.push_back(controller);
}

KeyboardInterface::~KeyboardInterface()
{
  for(unsigned i=0; i < m_controllers.size(); i++) 
    delete m_controllers[i];
}


std::string KeyboardInterface::getName()
{
  return KEYBOARD_INTERFACE_CONTROLLER_NAME;
}

unsigned KeyboardInterface::getNumController()
{
  return m_controllers.size();
}



IVehicleController * KeyboardInterface::getController(unsigned i)
{
  if(i < m_controllers.size()) 
    return m_controllers[i];
  return 0;
}

void KeyboardInterface::setConfiguration(XmlNode * root)
{
  EventReceiver * er=ResourceManager::getInstance()->getEventReceiver();
  for(unsigned i=0; i<m_controllers.size(); i++)
    delete m_controllers[i];
  m_controllers.clear();
  std::vector<XmlNode*> nodes,actions;
  root->getChildren("device",nodes);
  for(unsigned i=0; i<nodes.size(); i++) {
    VehicleKeyboardController * controller;
    controller=new VehicleKeyboardController(er);
    m_controllers.push_back(controller);
    nodes[i]->getChildren("action",actions);
    for(unsigned j=0; j<actions.size(); j++) {
      unsigned action,code;
      XmlNode * act=actions[j];
      if(act->get("action",action) && act->get("code",code)) 
        controller->setKeyForAction(action,code);
    }
  }
}


void KeyboardInterface::getActionDescription(std::string & descr, unsigned action, unsigned keycode )
{
  char buffer[128];
  const char * kk;
  if(keycode < 256)
    kk=keyDescr[keycode];
  else
    kk=NOT_DEFINED;

  snprintf(buffer,128,"%s %s",
      IVehicleController::getActionString(action),kk);

  descr=buffer;
}


void KeyboardInterface::getConfiguration(XmlNode * root)
{
  std::vector<XmlNode*> nodes;
  root->deleteAllChildren();

  for(unsigned i=0; i<m_controllers.size(); i++) {
    VehicleKeyboardController * controller=
      static_cast<VehicleKeyboardController *>(m_controllers[i]);
    XmlNode * node = root->addChild("device");
    
    for(unsigned i=0; i < IVehicleController::va_numActions; i++) {
      XmlNode * act=node->addChild("action");

      act->set("action",i);
      act->set("code",controller->getKeyForAction(i));
      std::string descr;

      getActionDescription(descr,i,controller->getKeyForAction(i));
      act->set("descr",descr);
    }
  }
}

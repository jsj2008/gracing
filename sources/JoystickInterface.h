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
#ifndef JOYSTICKINTERFACE_H
#define JOYSTICKINTERFACE_H
#include "EventReceiver.h"
#include "IDeviceInterface.h"


class JoystickInterface : public IEventListener, public IDeviceInterface
{
  public:

    JoystickInterface(irr::IrrlichtDevice *, const irr::SJoystickInfo & device);

    //////////////////////////////
    // Event Listener
    virtual void joystickEvent(const irr::SEvent::SJoystickEvent & joystickEvent);


    //////////////////////////////
    // Device Interface
    virtual std::string          getName();
    virtual unsigned             getNumController();
    virtual IVehicleController * getController(unsigned);

  private:
    friend class Joystick;
    struct JoystickAction {
      int  type;
      bool analog;
      int  index;
      int  value;
    };

    typedef irr::SEvent::SJoystickEvent  JoystickEvent;

    irr::u8     m_idJoystick;
    std::string m_name;
    unsigned    m_numControllers;

    bool        checkAxis(const JoystickEvent & joystickEvent,unsigned axis,irr::s16 & value);
    bool        checkButton(const JoystickEvent & joystickEvent,unsigned button,bool & pressed);

    bool        getAction(JoystickAction & action, const JoystickEvent & event);
    static void getActionString(char * buffer, int bufferSize, JoystickAction & action);

    irr::s16  m_axisPrevValues[irr::SEvent::SJoystickEvent::NUMBER_OF_AXES];
    bool      m_buttonsPrevValues[irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];
};


#endif

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
#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H
#include <irrlicht.h>
#include <vector>

class IEventListener  
{
  public:
    virtual void mouseEvent(const irr::SEvent::SMouseInput & MouseInput) { }
};

// TODO: probably this should be called
//       'EventDispacther' and not 'EventReceiver'
class EventReceiver : public irr::IEventReceiver
{
  public:
    EventReceiver();

    // This is the one method that we have to implement
    virtual bool OnEvent(const irr::SEvent& event);

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(irr::EKEY_CODE keyCode) const;

    virtual bool OneShotKey(irr::EKEY_CODE keyCode);

    virtual bool IsAnyKeyDown() const;

    inline void addListener(IEventListener * lstnr)
    {
      m_listeners.push_back(lstnr);
    }

    inline bool removeListener(IEventListener * lstnr)
    {
      std::vector<IEventListener*>::iterator it;
      for(it=m_listeners.begin(); it < m_listeners.end(); it++) 
        if( *it == lstnr ) {
          m_listeners.erase(it);
          return true;
        }
      return false;
    }

  private:
    // We use this array to store the current state of each key
    bool KeyIsDown[irr::KEY_KEY_CODES_COUNT];
    bool OneShotKeyIsDown[irr::KEY_KEY_CODES_COUNT];
    int  KeysPressed;

    std::vector<IEventListener*>  m_listeners;
};
#endif

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
#ifndef IPHASEHANDLER_H
#define IPHASEHANDLER_H

#include <irrlicht.h>

#include "PhyWorld.h"

// a game phase handler
class IPhaseHandler /*: public irr::IEventReceiver*/
{
  public:
    IPhaseHandler(irr::IrrlichtDevice * device,
        PhyWorld * world)
    {
      m_device = device;
      m_sceneManager = device->getSceneManager();
      m_world = world;
      m_driver = device->getVideoDriver();
      m_guiEnv = device->getGUIEnvironment();
    }

    virtual bool step()
    {
      m_sceneManager->drawAll();
      return false;
    }

    virtual void prepare() { }

  protected:
    irr::IrrlichtDevice  *      m_device;
    irr::scene::ISceneManager * m_sceneManager;
    irr::gui::IGUIEnvironment * m_guiEnv;
    PhyWorld *                  m_world;
    irr::video::IVideoDriver*   m_driver;
 
};
#endif

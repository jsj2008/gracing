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

#include "Race.h"


Race::Race(irr::IrrlichtDevice * device, PhyWorld * world)
  : IPhaseHandler(device,world)
{
  m_readySetGo = new GuiReadySetGo(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(0,0,200,100));
}

void Race::step()
{
  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  m_sceneManager->drawAll();
  m_world->step();
  m_guiEnv->drawAll();
  m_world->debugDrawWorld();
  m_driver->endScene();
}


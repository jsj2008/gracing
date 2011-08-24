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
#include <string>
#include "EmptyPhaseHandler.h"
#include "ResourceManager.h"


EmptyPhaseHandler::EmptyPhaseHandler(irr::IrrlichtDevice * device,
        PhyWorld * world)
  : IPhaseHandler(device,world)
{
  ResourceManager * resmanager=
    ResourceManager::getInstance();
  // TODO: choose the best image size 
  //       depending on the screen resolution
  std::string bgpath = resmanager->getResourcePath() + "/background.png";

  unsigned w,h;
  resmanager->getScreenWidth(w);
  resmanager->getScreenHeight(h);

  m_background = resmanager->getVideoDriver()->getTexture(bgpath.c_str());

  if(m_background) {
    m_background->grab();
    m_dstRect.UpperLeftCorner.X = 0;
    m_dstRect.UpperLeftCorner.Y = 0;
    m_dstRect.LowerRightCorner.X = w;
    m_dstRect.LowerRightCorner.Y = h;

    irr::core::dimension2d<irr::u32> dim=m_background->getSize();

    m_srcRect.UpperLeftCorner.X = 0;
    m_srcRect.UpperLeftCorner.Y = 0;
    m_srcRect.LowerRightCorner.X = dim.Width;
    m_srcRect.LowerRightCorner.Y = dim.Height;
  }
}

bool EmptyPhaseHandler::step()
{
  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  if(m_background)
    m_driver->draw2DImage (
        m_background,
        m_dstRect,
        m_srcRect,
        0,
        0,
        true);
  m_guiEnv->drawAll();
  m_driver->endScene();
  return false;
}

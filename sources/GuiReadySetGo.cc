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
#include <assert.h>

#include "gmlog.h"
#include "GuiReadySetGo.h"
#include "ResourceManager.h"


// measures
// ready: 0,0 220,180 
// set: 0,188 170,343
// go: 0,344 106,150

GuiReadySetGo::GuiReadySetGo(irr::gui::IGUIEnvironment * environment,
      irr::gui::IGUIElement * parent, irr::s32 id,
      const irr::core::rect<irr::s32> rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{

  ResourceManager * resmanager = ResourceManager::getInstance();

  irr::video::IVideoDriver* driver = environment->getVideoDriver();
  std::string path = resmanager->getResourcePath() + "/readysetgo.png";

  m_image = driver->getTexture(path.c_str());
  m_image->grab();
  m_driver=driver;

  SPart part;

  part.rect=irr::core::rect<irr::s32>(0,0,220,180);
  part.width=part.rect.LowerRightCorner.X-part.rect.UpperLeftCorner.X;
  part.height=part.rect.LowerRightCorner.Y-part.rect.UpperLeftCorner.Y;
  m_parts.push_back(part);

  part.rect=irr::core::rect<irr::s32>(0,188,170,343);
  part.width=part.rect.LowerRightCorner.X-part.rect.UpperLeftCorner.X;
  part.height=part.rect.LowerRightCorner.Y-part.rect.UpperLeftCorner.Y;
  m_parts.push_back(part);

  part.rect=irr::core::rect<irr::s32>(0,344,106,495);
  part.width=part.rect.LowerRightCorner.X-part.rect.UpperLeftCorner.X;
  part.height=part.rect.LowerRightCorner.Y-part.rect.UpperLeftCorner.Y;
  m_parts.push_back(part);

  m_partIndex=0;
  m_fraction=0.;
  m_delta=0.05;
  m_isStill=false;
}

void GuiReadySetGo::evolve()
{
  if(!IsVisible)
    return;
  static int framesStill=0;

  if(m_isStill) {
    framesStill++;
    if(framesStill >= 80) {
      m_partIndex++;
      if(m_partIndex >= m_parts.size()) {
        m_partIndex=0;
        IsVisible=false;
      }
      m_fraction=0.;
      m_isStill=false;
    }
  } else {
    m_fraction+=m_delta;
    if(m_fraction > 1.+m_delta) {
      m_isStill=true;
      framesStill=0;
      m_isEnded=m_partIndex == m_parts.size()-1;
    }
  }
}

void GuiReadySetGo::restart()
{
  m_partIndex=0;
  m_fraction=0.;
  m_delta=0.05;
  m_isStill=false;
  IsVisible=true;
  m_isEnded=false;
}

void GuiReadySetGo::draw()
{
  if(!IsVisible)
    return;
  if(!m_image)
    return;

  evolve();

  irr::core::rect<irr::s32>  sourceRect=m_parts[m_partIndex].rect;
  irr::core::rect<irr::s32> destRect(0,0,0,0);

  irr::core::rect<irr::s32> centeredRect;

  irr::u32 screenWidth;
  irr::u32 screenHeight;
  irr::u32 destWidth=m_parts[m_partIndex].width*m_fraction;
  irr::u32 destHeight=m_parts[m_partIndex].height*m_fraction;

  ResourceManager * resmanager = ResourceManager::getInstance();
  resmanager->getScreenHeight(screenHeight);
  resmanager->getScreenWidth(screenWidth);

  screenWidth-=destWidth;
  screenHeight-=destHeight;

  destRect.UpperLeftCorner.X=screenWidth / 2;
  destRect.UpperLeftCorner.Y=screenHeight / 2;

  destRect.LowerRightCorner.X=
    destRect.UpperLeftCorner.X + destWidth;
  destRect.LowerRightCorner.Y=
    destRect.UpperLeftCorner.Y + destHeight;

  m_driver->draw2DImage (
      m_image,
      destRect,
      sourceRect,
      0,
      0, //irr::video::SColor(255,255,255,255),
      true);
}

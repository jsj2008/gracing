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
#include "ResourceManager.h"
#include "GUISpeedometer.h"
#include "gmlog.h"

GUISpeedometer::GUISpeedometer(bool border, 
    irr::gui::IGUIEnvironment* environment,
		irr::gui::IGUIElement* parent, irr::s32 id, const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  m_guiEnv=environment;

  m_font=ResourceManager::getInstance()->getSystemFontSmall();

  if(!m_font) {
    GM_LOG("Cannot load font \n");
  }
}

void GUISpeedometer::draw() 
{   
  if(!IsVisible)
    return;

  irr::gui::IGUISkin * skin;

  skin=m_guiEnv->getSkin();
  if(!skin)
    return;

  irr::gui::IGUIFont *  font;

  if(m_font)
    font=m_font;
  else
    font=skin->getFont();


  //if (Background)
  {
  }

  if(font) {
    char b[32];
    irr::core::rect<irr::s32> frameRect(AbsoluteRect);

    irr::video::IVideoDriver* driver = Environment->getVideoDriver();
    irr::video::SColor BGColor = skin->getColor(irr::gui::EGDC_3D_FACE);
    driver->draw2DRectangle(BGColor, frameRect, &AbsoluteClippingRect);

    snprintf(b,32,"%3.0f",rint(getValue()));

    irr::video::SColor FGColor(255,255,255,255);
    font->draw(b, frameRect,FGColor,true, true, &AbsoluteClippingRect);
        //skin->getColor(irr::gui::EGDC_BUTTON_TEXT)
  }
      
  
  IGUIElement::draw(); 
} 


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
#ifndef GUI_CRONOMETER_H
#define GUI_CRONOMETER_H

#include "ResourceManager.h"
#include "GuiCronometer.h"

GuiCronometer::GuiCronometer(irr::gui::IGUIEnvironment * environment,
      irr::gui::IGUIElement * parent, irr::s32 id,
      const irr::core::rect<irr::s32> rectangle, irr::ITimer * timer)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  m_started=false;
  m_timer=timer;

  m_font=ResourceManager::getInstance()->getSystemFontSmall();
}

void GuiCronometer::stop()
{
  m_started=false;
}

void GuiCronometer::start()
{
  m_started=true;
  m_startTime=m_timer->getRealTime();
}

void GuiCronometer::draw()
{
  unsigned time;
  if(m_started) 
    time=m_timer->getRealTime()-m_startTime;
  else
    time=0;
  
  unsigned cs,s,m;

  cs=(time / 10);
  s=cs / 100;
  m=s / 60;
  cs=cs % 100;
  s=s % 60;

  char buffer[64];
  if(m == 0)
    snprintf(buffer,64,"0' %02d''.%02d",s,cs);
  else
    snprintf(buffer,64,"%02d' %02d''.%02d",m,s,cs);

  irr::video::SColor FGColor(255,255,255,255);
  irr::core::rect<irr::s32> frameRect(AbsoluteRect);
  m_font->draw(buffer, frameRect,FGColor,false, true, &AbsoluteClippingRect);
}
#endif

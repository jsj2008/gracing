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
#include <stdarg.h>
#include "GuiCommunicator.h"
#include "ResourceManager.h"
#include "gmlog.h"

GuiCommunicator::GuiCommunicator(irr::gui::IGUIEnvironment * environment,
      irr::gui::IGUIElement * parent, irr::s32 id,
      const irr::core::rect<irr::s32> rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  m_showingMessage=false;
  m_buffer[0]=0;
  m_font=ResourceManager::getInstance()->getSystemFont();
}

void GuiCommunicator::unshow()
{
  m_showingMessage=false;
}

void GuiCommunicator::show(const char * fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(m_buffer, bufferSize, fmt, ap);
  va_end(ap);

  m_showingMessage=true;
  m_framesStillToShow=numberOfFrames;
}

void GuiCommunicator::draw()
{
  if(!IsVisible)
    return;
  if(!m_showingMessage)
    return;

  m_framesStillToShow--;
  if(m_framesStillToShow == 0)
  {
    m_showingMessage=false;
  }

  irr::video::SColor FGColor(255,255,255,255);
  irr::core::rect<irr::s32> frameRect(AbsoluteRect);
  m_font->draw(m_buffer, frameRect,FGColor,true, true, &AbsoluteClippingRect);

}

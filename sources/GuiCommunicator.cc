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
  m_nMessages=0;
  m_font=ResourceManager::getInstance()->getSystemFont();

  m_frame=new GuiFrame(rectangle);
}

void GuiCommunicator::unshow()
{
  m_showingMessage=false;
}

void GuiCommunicator::adjustSizeWithLastInsert()
{
  assert(m_nMessages);
  unsigned li=m_nMessages - 1;
  irr::core::dimension2d<irr::u32>  dim=
     m_font->getDimension(irr::core::stringw(m_buffers[li].text).c_str());

  m_buffersHeights[li] = dim.Height;
  m_totalHeight += dim.Height;
  if(dim.Width > m_totalWidth) {
    m_totalWidth = dim.Width;
  }

  unsigned sheight,swidth;
  ResourceManager::getInstance()->getScreenHeight(sheight);
  ResourceManager::getInstance()->getScreenWidth(swidth);
  
  AbsoluteRect.UpperLeftCorner.Y = (sheight - m_totalHeight) >> 1;
  AbsoluteRect.LowerRightCorner.Y = 
    AbsoluteRect.UpperLeftCorner.Y + m_totalHeight;

  AbsoluteRect.UpperLeftCorner.X = (swidth - m_totalWidth) >> 1;
  AbsoluteRect.LowerRightCorner.X =
    AbsoluteRect.UpperLeftCorner.X + m_totalWidth;
  m_frame->setSize(AbsoluteRect);
}

void GuiCommunicator::add(bool center,const char * fmt, ...)
{
  va_list ap;
  if(m_nMessages < maxMessages) {
    va_start(ap, fmt);
    vsnprintf(m_buffers[m_nMessages].text, bufferSize, fmt, ap);
    va_end(ap);
    m_centers[m_nMessages]=center;
    irr::core::dimension2d<irr::u32>  dim=
       m_font->getDimension(irr::core::stringw(m_buffers[0].text).c_str());
    m_nMessages++;
    adjustSizeWithLastInsert();
  }
}

void GuiCommunicator::show(const char * fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(m_buffers[0].text, bufferSize, fmt, ap);
  va_end(ap);
  m_centers[0]=true;
  m_nMessages=1;
  m_totalHeight=0;
  m_totalWidth=0;
  m_showingMessage=true;
  m_framesStillToShow=160;
  adjustSizeWithLastInsert();
}

void GuiCommunicator::draw()
{
  if(!IsVisible)
    return;
  if(!m_showingMessage)
    return;

  if(m_frame)
    m_frame->draw();

  m_framesStillToShow--;
  if(m_framesStillToShow == 0)
  {
    m_showingMessage=false;
  }

  irr::video::SColor FGColor(255,255,255,255);
  irr::core::rect<irr::s32> frameRect(AbsoluteRect);
  frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + m_buffersHeights[0];
  for(unsigned i=0; i<m_nMessages; i++) {
    m_font->draw(m_buffers[i].text, frameRect,FGColor,m_centers[i], true, &AbsoluteClippingRect);
    frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y;
    frameRect.LowerRightCorner.Y = m_buffersHeights[i] + frameRect.UpperLeftCorner.Y;
  }
}


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
#ifndef GUI_COMMUNICATOR_H
#define GUI_COMMUNICATOR_H

#include <irrlicht.h>
#include "GuiFrame.h"

class GuiCommunicator : public irr::gui::IGUIElement
{
  public:
  GuiCommunicator(irr::gui::IGUIEnvironment * environment,
      irr::gui::IGUIElement * parent, irr::s32 id,
      const irr::core::rect<irr::s32> rectangle);

  void show(const char * fmt,...);
  void add(bool center,const char * fmt,...);
  void unshow();

  virtual void draw();

  private:
  enum                  { bufferSize=64 };
  enum                  { numberOfFrames=160 }; // TODO: calc this dinamically 
  enum                  { maxMessages=7 };
  //char                  m_buffer[maxMessages][bufferSize];
  struct message {
    char text[bufferSize];
  };
  message               m_buffers[maxMessages];
  bool                  m_centers[maxMessages];
  unsigned              m_buffersHeights[maxMessages];
  bool                  m_showingMessage;
  unsigned              m_framesStillToShow;
  unsigned              m_nMessages;
  unsigned              m_totalHeight;
  unsigned              m_totalWidth;
  irr::gui::IGUIFont *  m_font;
  GuiFrame *            m_frame;

  void adjustSizeWithLastInsert();
  
  public:
  inline void refreshTime() {  m_framesStillToShow=numberOfFrames; }

};
#endif

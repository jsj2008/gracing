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
#ifndef ITRACKCHOOSER_H
#define ITRACKCHOOSER_H

#include "IPhaseHandler.h"

class ITrackChooser : public IPhaseHandler
{
  public:
    ITrackChooser(irr::IrrlichtDevice * device,
        PhyWorld * world)
      : IPhaseHandler(device,world) { };

    virtual bool step()=0;
    virtual void prepare()=0;
    virtual void unprepare()=0;
};

class DefaultTrackChooser : public ITrackChooser 
{
  public:
    DefaultTrackChooser(irr::IrrlichtDevice * device,
        PhyWorld * world);


   virtual bool step();
   virtual void prepare();
   virtual void unprepare();

  private:
    irr::video::ITexture  * m_background;
    irr::video::ITexture  * m_trackShot;
    irr::video::ITexture  * m_defaultTrackShot;
    irr::core::rect<irr::s32>  m_dstRect;
    irr::core::rect<irr::s32>  m_srcRect;

    unsigned                   m_currentTrackIndex;

    void setTrack();
};
#endif

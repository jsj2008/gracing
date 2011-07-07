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
#ifndef COCKPIT_H
#define COCKPIT_H
#include <irrlicht.h>
#include "INumberOutput.h"

class GuiCockpit : public  INumberOutput, public irr::gui::IGUIElement 
{
  public:
    GuiCockpit(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle,
        irr::ITimer * timer);

    ~GuiCockpit();

    void draw();

    void stop();
    void start();
    void pause();
    void unpause();

    inline void setLap(unsigned lap, unsigned totalLaps) { m_lap=lap; m_totalLaps=totalLaps; }
    inline void setRank(unsigned rank, unsigned totRank) { m_rank=rank; m_totRank=totRank; }

  private:

    void drawString(const char *, const irr::core::vector2d<irr::s32> &);

    irr::gui::IGUIEnvironment *         m_guiEnv;
    std::vector<irr::video::ITexture*>  m_textures;

    irr::core::rect<irr::s32>           m_bodyRect;
    unsigned                            m_bodyTidx;

    unsigned                            m_handTidx;
    irr::core::rect<irr::s32>           m_handRect ;
    irr::core::rect<irr::s32>           m_handDstRect ;
    irr::core::vector2d<irr::s32>       m_handCenter;
    irr::core::vector2d<irr::s32>       m_handPosition;

    irr::core::rect<irr::s32>           m_digitsRect[11];
    irr::s32                            m_digitsTexture[11];
    bool                                m_digitsPresence[11];

    irr::core::vector2d<irr::s32>       m_timerPosition;
    irr::core::vector2d<irr::s32>       m_lapPosition;
    irr::core::vector2d<irr::s32>       m_rankPosition;

    irr::ITimer *                       m_timer;
    unsigned                            m_startTime;
    bool                                m_started;
    unsigned                            m_pausedTime;
    unsigned                            m_showedTime;
    bool                                m_paused;

    unsigned                            m_totalLaps;
    unsigned                            m_lap;

    unsigned                            m_rank;
    unsigned                            m_totRank;

};

#endif

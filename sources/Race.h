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
#ifndef RACE_H
#define RACE_H
#include "IPhaseHandler.h"
#include "IVehicle.h"
#include "Track.hh"

#include "GuiReadySetGo.h"
#include "GuiCronometer.h"


class Race : public  IPhaseHandler
{
  public:
    Race(irr::IrrlichtDevice * device, PhyWorld * world);

    virtual void step();

    bool addVehicle(IVehicle * vehicle);

    inline void setTrack(Track * track) { m_track=track; }

    inline void restart() { gotoState(rs_readySetGo); }

    enum {
      rs_readySetGo,
      rs_started,
      rs_paused
    };

  private:
  
    bool gotoState(unsigned state);

    enum { max_vehicles=3 };

    struct VehicleInfo {
      IVehicle * vehicle;
      irr::core::vector3df startPosition;
      double         startRotation;
    };

    Track     *  m_track;


    struct VehicleInfo m_vehicles[max_vehicles];
    int                m_nVehicles;

    GuiReadySetGo * m_readySetGo;
    GuiCronometer * m_cronometer;

    unsigned             m_status;
};


#endif

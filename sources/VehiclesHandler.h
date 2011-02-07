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
#ifndef VEHICLESHANDLER_H
#define VEHICLESHANDLER_H

#include <irrlicht.h>
#include "IPhaseHandler.h"
#include "IVehicle.h"

class VehiclesHandler : public IPhaseHandler
{
  public:
    VehiclesHandler(
      irr::IrrlichtDevice * device, PhyWorld * world);

    ~VehiclesHandler();

    virtual void step();

    virtual bool OnEvent(const irr::SEvent& event);
  private:
    irr::core::array<IVehicle*>  
              m_vehicles;

    irr::scene::ISceneNodeAnimator* m_rotator;

    void startVehicle(unsigned index=0);
    void prevVehicle();
    void nextVehicle();

    unsigned m_currentVehicleIndex;


    int      m_status;
    enum {
      ST_SHOWING,
      ST_EXITING,
      ST_ENTERING
    };
};
#endif

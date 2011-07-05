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
#ifndef VEHICLECHOOSER_H
#define VEHICLECHOOSER_H

#include <vector>
#include <irrlicht.h>

/* implemented interfaces */
#include "IPhaseHandler.h"

/* used interfaces */
#include "IVehicle.h"

class VehicleChooser :  public IPhaseHandler
{
  public:
    VehicleChooser(irr::IrrlichtDevice * device,
        PhyWorld * world);

    bool step();
    void prepare();

    struct iTransiction 
    {
      virtual void init(double t, const irr::core::vector3df & startPos, const irr::core::vector3df & endPos)=0;
      virtual bool doit(double t, irr::core::vector3df & position, double  & rotation)=0;
    };

  private:

    enum {
      status_uninited,
      status_vehicle_staying,
      status_vehicle_entering,
      status_vehicle_exiting,
    } m_status,m_nextStatus;

    enum {
      trans_stay,
      trans_exit,
      trans_enter
    };


    iTransiction * m_transictions[3];

    void changeVehicle(int direction=1);

    unsigned m_currentVehicle;
    unsigned m_maxVehicles;

    double   m_transictionTime;
    double   m_timeStep;

    irr::core::vector3df   m_vPosition;
    double                 m_vRotation;

    IVehicle * m_vehicle;
    irr::scene::ICameraSceneNode * m_camera;
    irr::scene::ILightSceneNode *  m_sun;

    irr::core::vector3df  m_pos0;
    irr::core::vector3df  m_pos1;
    irr::core::vector3df  m_pos2;

    int                   m_vdir;
};

#endif

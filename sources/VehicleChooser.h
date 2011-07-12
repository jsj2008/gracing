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

#include "Tweening.h"
typedef ITween<irr::core::vector3df>         ITween3df;
typedef TweenVoid<irr::core::vector3df>      TweenVoid3df;
typedef TweenQuadIn<irr::core::vector3df>    TweenQuadIn3df;
typedef TweenBounceOut<irr::core::vector3df> TweenBounceOut3df;

typedef ITween<double>         ITween1d;
typedef TweenVoid<double>      TweenVoid1d;
typedef TweenQuadIn<double>    TweenQuadIn1d;
typedef TweenBounceOut<double> TweenBounceOut1d;

class VehicleChooser :  public IPhaseHandler
{
  public:
    VehicleChooser(irr::IrrlichtDevice * device,
        PhyWorld * world);

    bool step();
    void prepare(unsigned nHumanVehicles, unsigned nTotVehicles, unsigned * choosenVehicles);
    void unprepare();

#if 0
    struct iTransiction 
    {
      virtual void init(double t, const irr::core::vector3df & startPos, const irr::core::vector3df & endPos)=0;
      virtual bool doit(double t, irr::core::vector3df & position, double  & rotation)=0;
    };
#endif

  private:

    enum {
      status_uninited,
      status_rotating,
      status_still
    } m_status;

    /// !!! /// !!!
    unsigned * m_choosenVehicles;
    unsigned   m_totChooseableVehicles;
    unsigned   m_humanVehicles;
    /// !!! /// !!!

    double   m_transictionTime;
    double   m_timeStep;


    ITween1d *             m_transiction;
    double                 m_angle;


    enum                   { m_shownVehicles=4 };

    struct vinfo {
      IVehicle *             vehicle;
      unsigned               index;
      double                 angleOffset;
      double                 rotation;
      double                 height;
    } m_infos[m_shownVehicles];
    unsigned                       m_vehicleIndex;
    
    irr::scene::ICameraSceneNode * m_camera;
    irr::scene::ILightSceneNode *  m_sun;

    // options
    double m_radius;
    double m_angleSpan;
    double m_vehiclesHeight;
};

#endif

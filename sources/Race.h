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
/* implemented interfaces */
#include "IPhaseHandler.h"


/* used interfaces */
#include "IVehicle.h"
#include "IVehicleController.h"

#include "Track.hh"
#include "VehicleCameraAnimator.h"

/* gui elements */
#include "GuiReadySetGo.h"
#include "GuiCronometer.h"
#include "GuiCommunicator.h"


class Race : public  IPhaseHandler
{
  public:
    Race(irr::IrrlichtDevice * device, PhyWorld * world);

    virtual void step();

    bool addVehicle(IVehicle * vehicle, IVehicleController * controller, bool followed=false);

    inline void setTrack(Track * track) { m_track=track; }

    inline void restart() { gotoState(rs_readySetGo); }

    enum {
      rs_readySetGo,
      rs_started,
      rs_paused
    };

    /* callback for lap trigger */
    void lapTriggered(void * userdata);

  private:

    unsigned m_debugFlags;

    void recalcVehicleVehiclesStartPositions();
  
    bool gotoState(unsigned state);
    void updateVehiclesInfo();
    void updateKeyboard();

    inline void setLapNumber(unsigned n) { m_totalLaps=n; }

    enum { max_vehicles=3 };

    struct VehicleInfo {
      IVehicle *           vehicle;

      /* start position of this vehicle */
      irr::core::vector3df startPosition;

      /* start rotation (aroux Y axis) of this vehicle */
      double               startRotation;

      /* the curret control point                   */
      /* each vehicle is between two control points */
      /* this is the first of the two               */
      unsigned             controlPointIndex;

      /* distatnce to the next control point        */
      double               ctrlPntDistance;

      /* if true the vehicle is goind in the wrong way */
      bool                 wrongWay;

      /* if true the vehicle is passed on last control */
      /* point and is waiting for lap end              */
      bool                 waitingForLapTrigger;

      /* this is the class responsible to control      */
      /* this vehicle (throttle, steer,...             */
      IVehicleController * controller;


      /* lap number of this vehicle */
      unsigned             lapNumber;

      /* frame in which the vehicle has overturn */
      unsigned             overturnCountDown;

    };

    Track     *  m_track;

    // util
    void restoreVehicle(VehicleInfo &);
    void followNextVehicle();


    struct VehicleInfo m_vehicles[max_vehicles];
    unsigned           m_nVehicles;

    GuiReadySetGo *   m_readySetGo;
    GuiCronometer *   m_cronometer;
    GuiCommunicator * m_communicator;

    unsigned          m_status;

    unsigned          m_totalLaps;

    VehicleCameraAnimator *        m_cameraAnim;
    irr::scene::ICameraSceneNode * m_camera;
    unsigned                       m_followedVehicleIndex;
    enum { invalidVehicleIndex = 0xffff };

    irr::IrrlichtDevice *          m_device;
};


#endif

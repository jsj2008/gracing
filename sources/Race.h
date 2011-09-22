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

#include <string>

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
#include "GuiCockpit.h"

class Race : public  IPhaseHandler
{
  public:
    Race(irr::IrrlichtDevice * device, PhyWorld * world);

    virtual bool step();

    bool addVehicle(IVehicle * vehicle, 
        IVehicleController * controller, 
        const char * name=0,
        bool followed=false);

    //inline void setTrack(Track * track) { m_track=track; }
    void setTrack(Track * track);

    void unprepare();

    inline void restart() { gotoState(rs_readySetGo); }

    enum {
      rs_readySetGo,
      rs_started,
      rs_finished,
      rs_paused,
      rs_notRunning
    };

    /* callback for lap trigger */
    void lapTriggered(void * userdata);
    void togglePause();
    void setSplitModality(int modality);

  private:

    unsigned m_debugFlags;

    void recalcVehicleVehiclesStartPositions();
  

    inline void setLapNumber(unsigned n) { m_totalLaps=n; }

    struct VehicleInfo;
    struct CameraData {
      VehicleCameraAnimator *        cameraAnim;
      irr::scene::ICameraSceneNode * camera;
      GuiCockpit *                   cockpit;
      irr::core::rect<irr::s32>      viewport;

      CameraData(const VehicleInfo &, irr::gui::IGUIEnvironment *, irr::IrrlichtDevice *);
      ~CameraData();
      void setViewPort(const irr::core::rect<irr::s32> & viewport);
      void setViewPort(irr::s32 x1, irr::s32 y1, irr::s32 x2, irr::s32 y2);
    };

    

    enum { max_vehicles=4 };

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

      /* whenever or not the vehicle has finished the race */
      bool                 raceFinished;

      unsigned             index;
      std::string          name;

      unsigned             rank;

      CameraData *         cameraData;
      bool                 waitChangeCamera;
      bool                 waitcameraUp;
      bool                 waitcameraDown;

    };

    Track     *  m_track;

    // vehicle stuff
    void restoreVehicle(VehicleInfo &);
    void vehicleFinished(VehicleInfo &);
    void updateVehiclesInfo();
    int  vehicleInfoCmp(const VehicleInfo &, const VehicleInfo &);

    // camera handling
    void followNextVehicle();

    // util
    bool updateKeyboard();
    void updateRanking();
    void updateCamerasViewPort();
    void drawScene();

    // status handling
    bool gotoState(unsigned state);

    struct VehicleInfo m_vehicles[max_vehicles];
    unsigned           m_rank[max_vehicles];
    unsigned           m_nVehicles;

    enum { max_cameras=4 };
    CameraData *      m_cameraData[max_cameras];
    unsigned          m_nCameras;
    unsigned          m_splitType;

    GuiReadySetGo *   m_readySetGo;

    GuiCommunicator * m_communicator;

    unsigned          m_status;

    unsigned          m_totalLaps;

    unsigned          m_nFinishedVehicles;

    unsigned          m_firstControlPoint;

    irr::IrrlichtDevice *          m_device;

    irr::core::rect<irr::s32> m_wholeScreenViewPort;
};


#endif

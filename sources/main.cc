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
#include <irrlicht.h>
#include <typeinfo>

#include "gException.h"
#include "ResourceManager.h"
#include "Track.hh"
#include "PhyWorld.h"
#include "gmlog.h"
#include "Vehicle.h"
#include "TestVehicle.h"
#include "IrrDebugDrawer.h"
#include "DataRecorder.h"
#include "IPhaseHandler.h"
#include "Race.h"
#include "GUISpeedometer.h"
#include "EventReceiver.h"
#include "VehicleChooser.h"

// vehicle controllers
#include "VehicleKeyboardController.h"
#include "VehicleNullController.h"
#include "VehicleAutoController.h"


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

extern double glob_frameRate;


IrrDebugDrawer * debugDrawer= 0;

static int dumpNode(irr::scene::ISceneNode * node,int level=0) 
{
  int tot=1;
  char spacer[128]="                                        ";
  if(level==0) 
    GM_LOG("------------------------------\n");
  if(node == 0) {
    GM_LOG("No node!!\n");
    return 0;
  }
  if(level<32) {
    spacer[level]=0;
    GM_LOG("%s",spacer);
    spacer[level]=' ';
  }

  irr::core::vector3df pos;
  irr::core::vector3df absPos;

  absPos=node->getAbsolutePosition();
  pos=node->getPosition();

  GM_LOG("[%03d] Node '%s' @%p of type %c%c%c%c, id: %04X, pos: %f,%f,%f"
      ", abs pos: %f,%f,%f, rotation: %f,%f,%f\n",level,
      node->getName(),
      node,
      (node->getType())&0xff,
      (node->getType()>>8)&0xff,
      (node->getType()>>16)&0xff,
      (node->getType()>>24)&0xff,
      node->getID(),
      pos.X,pos.Y,pos.Z,
      absPos.X,absPos.Y,absPos.Z,
       node->getRotation().X, 
       node->getRotation().Y,
       node->getRotation().Z);

  const core::list<irr::scene::ISceneNode*>&  list=node->getChildren();

  irr::scene::ISceneNodeList::ConstIterator it = list.begin();
  for (; it!=list.end(); ++it)
  {
    tot+=dumpNode(*it,level+1);
  }

  if(level==0) {
    GM_LOG("-----> tot nodes: %d\n",tot);
    GM_LOG("------------------------------\n");
  }

  return tot;
}

bool stepMode=false;
static bool doneStep=false;


CFG_PARAM_BOOL(glob_enableDebug)=false;
CFG_PARAM_BOOL(glob_enableShadows)=false;
CFG_PARAM_BOOL(glob_enableFullScreen)=false;
CFG_PARAM_BOOL(glob_enableVSync)=false;


#ifdef __WIN32__
#include <Windows.h>
int WINAPI WinMain(   HINSTANCE   hInstance,HINSTANCE   hPrevInstance,LPSTR   lpCmdLine,int nCmdShow)
{
   char *apppath = new char [MAX_PATH];
   strncpy(apppath,  GetCommandLine(), MAX_PATH);
   if (apppath[0] == '\"') {
      apppath = (apppath+1);
      char *lastdit = strchr(apppath, '\"');
      *lastdit = '\x0';
   }

   char **argv = NULL;
   int argc = 1;

   if ( *lpCmdLine != '\x0' ) {
      char *cmdLineCopy = new char [strlen(lpCmdLine)+1];
      strcpy ( cmdLineCopy, lpCmdLine );

      char *c = cmdLineCopy;
      while(c) {
         ++argc;
         c = strchr ( (c+1),' ');
      }

      argv = new char *[argc];
      argv[0] = apppath;

      if(argc > 1) {
         argv [1] = cmdLineCopy;
         char *c = strchr(cmdLineCopy, ' ');
         int n = 2;
         while(c) {
            *c = '\x0';
            argv [n] = (c+1);
            ++n;
            c = strchr((c+1), ' ');
         }
      }
   } else {
      argv = new char *[1];
      argv[0] = apppath;
   }
#else
int main(int argc, char ** av)
{
#endif

  CFG_INIT_BOOL(glob_enableDebug,false);
 
  GM_LOG("--------- starting gracing\n");
 
  ResourceManager * resmanager=ResourceManager::getInstance();

  video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

  u32 screenWidth;
  u32 screenHeight;

  resmanager->getScreenHeight(screenHeight);
  resmanager->getScreenWidth(screenWidth);

  IrrlichtDevice *device =
    createDevice( driverType, dimension2d<u32>(screenWidth, screenHeight), 24,
        glob_enableFullScreen,
        glob_enableShadows, 
        glob_enableVSync,
        resmanager->getEventReceiver()); 
  if (!device)
    return 1;

  resmanager->setDevice(device);


  device->setWindowCaption(L"gracing - rosco-p");

  IVideoDriver* driver = device->getVideoDriver();
  PhyWorld * world = ResourceManager::getInstance()->getPhyWorld(); 

  if(glob_enableDebug) {
    debugDrawer= new IrrDebugDrawer(driver);
    world->setDebugDrawer(debugDrawer);
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);
  }


  Track * thetrack=0;

  // temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code
  const XmlNode * defaultTrack;
  ResourceManager::getInstance()->cfgGet("default-track",defaultTrack);

  if(defaultTrack) {
    std::string name;
    defaultTrack->get("name",name);
    thetrack=new Track(device,world,name.c_str());
    GM_LOG("got the default track %s!!!!\n",name.c_str());
  }

  //thetrack=new Track(device,world,"farm.zip");
  //thetrack=new Track(device,world,"devtrack.zip");
  //thetrack=new Track(device,world,"tuxtollway.zip");
  //thetrack=new Track(device,world,"jungle.zip");
  //thetrack=new Track(device,world,"beach.zip");


  if(!thetrack) 
    thetrack=new Track(device,world,"farm.zip");


  bool autoplayer;
  ResourceManager::getInstance()->cfgGet("start-auto-player",autoplayer);

  // temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code temp code


  bool done=false;
  int lastFPS=-1;

  unsigned long startFrameTime;
  unsigned long endFrameTime;
  unsigned long frameDuration = 1000 / glob_frameRate;

  // phase handlers
  VehicleChooser * vehicleChooser;
  Race *           race;
  IPhaseHandler *  currentPhaseHandler;

  vehicleChooser=new VehicleChooser(device,world);
  race = new Race(device,world);

  currentPhaseHandler = 0;


#define START_CHOOSER 1
#ifdef START_CHOOSER
  currentPhaseHandler = new VehicleChooser(device,world);
#else
  const std::vector<IVehicle*> & vehicles=
    resmanager->getVehiclesList();
  assert(vehicles.size() >= 4);
  race->setTrack(thetrack);
  race->addVehicle(vehicles[1],
      new VehicleKeyboardController(resmanager->getEventReceiver()), 
      //new VehicleAutoController(), 
      "gonorra",true);
  race->addVehicle(vehicles[0], 
      new VehicleAutoController(), 
      "ccaaspeedstar");
  race->addVehicle(vehicles[3], new VehicleAutoController(),
      "speedstar");
  race->restart();
  currentPhaseHandler = race;
#endif


  unsigned runningVehicles[3];

  // temp init !!!!!
  vehicleChooser->prepare(1,3,runningVehicles);
  currentPhaseHandler= vehicleChooser;

  bool donePhase;

  while(device->run() && !done) {
    if(device->isWindowActive()) {

      startFrameTime=device->getTimer()->getRealTime();
      if(!stepMode || !doneStep) {
        doneStep=true;
        donePhase=currentPhaseHandler->step();
      }

      /* temp keyboard handling part */
      if(resmanager->getEventReceiver()->IsKeyDown(irr::KEY_ESCAPE))
        done=true;

      if (driver->getFPS() != lastFPS)
      {
        lastFPS = driver->getFPS();
        core::stringw tmp = L"gracing (FPS: ";
        tmp += lastFPS;
        tmp += ")";
        device->setWindowCaption(tmp.c_str());
      }

      if(donePhase) {
        if(currentPhaseHandler == race) {
          race->unprepare();
          vehicleChooser->prepare(1,3,runningVehicles);
          currentPhaseHandler= vehicleChooser;
        } else if(currentPhaseHandler == vehicleChooser) {
          vehicleChooser->unprepare();
          // start the race
          const std::vector<IVehicle*> & vehicles=
            resmanager->getVehiclesList();
          assert(vehicles.size() >= 4);

          race->setTrack(thetrack);

          if(autoplayer) 
            race->addVehicle(vehicles[runningVehicles[0]],
                new VehicleAutoController(),
                vehicles[runningVehicles[0]]->getName().c_str(),true);
          else
            race->addVehicle(vehicles[runningVehicles[0]],
                new VehicleKeyboardController(resmanager->getEventReceiver()), 
                vehicles[runningVehicles[0]]->getName().c_str(),true);

          race->restart();

          currentPhaseHandler = race;
        }
      }
      endFrameTime=device->getTimer()->getRealTime();
      unsigned long dt = (endFrameTime - startFrameTime);
      if(dt < frameDuration) {
        device->sleep(frameDuration - dt);
      }
    } else {
      device->yield();
    }
  }
}


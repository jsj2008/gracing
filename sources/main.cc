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

#include "gException.h"
#include "ResourceManager.h"
#include "CCrisMeshFileLoader.h"
#include "Track.hh"
#include "PhyWorld.h"
#include "gmlog.h"
#include "Vehicle.h"
#include "TestVehicle.h"
#include "VehicleCameraAnimator.h"
#include "IrrDebugDrawer.h"
#include "DataRecorder.h"
#include "IPhaseHandler.h"
#include "Race.h"
#include "GUISpeedometer.h"
#include "EventReceiver.h"

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

#define CAMERA_STEP 0.05

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

  EventReceiver receiver;

  video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

  u32 screenWidth;
  u32 screenHeight;

  resmanager->getScreenHeight(screenHeight);
  resmanager->getScreenWidth(screenWidth);

  IrrlichtDevice *device =
    createDevice( driverType, dimension2d<u32>(screenWidth, screenHeight), 16,
        false, false, false, &receiver);
  if (!device)
    return 1;

  resmanager->setDevice(device);


  device->setWindowCaption(L"gracing - rosco-p");

  ISceneManager* smgr = device->getSceneManager();
  IVideoDriver* driver = device->getVideoDriver();
  IGUIEnvironment* guienv = device->getGUIEnvironment();
  PhyWorld * world = PhyWorld::buildMe();
  CCrisMeshFileLoader * mloader=new CCrisMeshFileLoader(smgr,device->getFileSystem());

  if(glob_enableDebug) {
    debugDrawer= new IrrDebugDrawer(driver);
    world->setDebugDrawer(debugDrawer);
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);
  }

  smgr->addExternalMeshLoader(mloader);

  // prepare the track
  Track * thetrack;
  thetrack=new Track(device,world,"farm.zip");
  thetrack->load();

  // prepare the vehicle
  std::string vehpath;
  resmanager->getVehicleCompletePath("turing_machine.zip",vehpath);
  IVehicle * vehicle=new Vehicle(
        0, /* smgr->getRootSceneNode(),*/
        device,
        world,
        vehpath.c_str(),0xcafe);
  if(glob_enableDebug)
    ((Vehicle*)vehicle)->setDebugDrawFlags(Vehicle::db_forwardImpulse | Vehicle::db_sideImpulse | Vehicle::db_suspensions);
  vehicle->load();
  vehicle->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);

  resmanager->getVehicleCompletePath("sprinter.zip",vehpath);
  IVehicle * vehicle2 = new Vehicle(
        0, /* smgr->getRootSceneNode(),*/
        device,
        world,
        vehpath.c_str(),0xcafe);
  if(glob_enableDebug)
    ((Vehicle*)vehicle2)->setDebugDrawFlags(Vehicle::db_forwardImpulse | Vehicle::db_sideImpulse | Vehicle::db_suspensions);
  vehicle2->load();
  //vehicle2->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);

#if 0
  resmanager->getVehicleCompletePath("sprinter.zip",vehpath);
  IVehicle * vehicle3 = new Vehicle(
        0, /* smgr->getRootSceneNode(),*/
        device,
        world,
        vehpath.c_str(),0xcafe);
  if(glob_enableDebug)
    ((Vehicle*)vehicle3)->setDebugDrawFlags(Vehicle::db_forwardImpulse | Vehicle::db_sideImpulse | Vehicle::db_suspensions);
  vehicle3->load();
  vehicle3->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);
#endif


  // gui
  GUISpeedometer * smeter=new GUISpeedometer(true,guienv,guienv->getRootGUIElement(),1,
      core::rect<s32>(0,0,200,100));
  vehicle->setSpeedOMeter(smeter);
  std::string fontPath = resmanager->getResourcePath() + "/title_font.xml";
	gui::IGUIFont* font_big = guienv->getFont(fontPath.c_str());
	if (font_big) 
		guienv->getSkin()->setFont(font_big);


  // camera
  irr::scene::ICameraSceneNode * camera;
  camera = smgr->addCameraSceneNode();
  VehicleCameraAnimator * camanim=new
      VehicleCameraAnimator(vehicle);
  camera->addAnimator(camanim);

  bool flagC, flagD;
  bool flagL, flagU;
  bool flagI;
  bool done=false;
  int lastFPS=-1;

  flagL=true;
  flagU=true;
  flagC=true;
  flagD=true;
  flagI=true;

  unsigned long startFrameTime;
  unsigned long endFrameTime;
  unsigned long frameDuration = 1000 / 80;

  Race *          race;
  race = new Race(device,world);

  IVehicleController * controller= new VehicleKeyboardController(&receiver);

  race->setTrack(thetrack);
  //race->addVehicle(vehicle,new VehicleAutoController());
  race->addVehicle(vehicle,controller);
  //race->addVehicle(vehicle2,new VehicleAutoController());
  //race->addVehicle(vehicle3,new VehicleAutoController());

  IPhaseHandler * currentPhaseHandler;
  currentPhaseHandler = race;

  race->restart();

  while(device->run() && !done) {
    if(device->isWindowActive()) {

      startFrameTime=device->getTimer()->getRealTime();
      if(!stepMode || !doneStep) {
        doneStep=true;
        currentPhaseHandler->step();
      }

      /* temp keyboard handling part */
      if(receiver.IsKeyDown(irr::KEY_ESCAPE) || 
          receiver.IsKeyDown(irr::KEY_KEY_Q) )
        done=true;

      if(receiver.IsKeyDown(irr::KEY_KEY_L)) {
        thetrack->load();
        if(flagL) {
          dumpNode(smgr->getRootSceneNode());
          flagL=false;
        } 
      } else {
        flagL=true;
      }

      if(receiver.IsKeyDown(irr::KEY_KEY_U)) {
        thetrack->unload();
        if(flagU) {
          dumpNode(smgr->getRootSceneNode());
          flagU=false;
        }
      } else {
        flagU=true;
      }

      if(receiver.IsKeyDown(irr::KEY_KEY_C)) {
        if(flagC)  {
          race->restart();
          GM_LOG("start position: %f,%f,%f\n",
              thetrack->getStartPosition().X,
              thetrack->getStartPosition().Y,
              thetrack->getStartPosition().Z);
          flagC=false;
        }
      } else {
        flagC=true;
      }


      if(receiver.IsKeyDown(irr::KEY_KEY_W))
        camanim->moveXY(0.,CAMERA_STEP);

      if(receiver.IsKeyDown(irr::KEY_KEY_Z)) 
        camanim->moveXY(0.,-CAMERA_STEP);

      if(receiver.IsKeyDown(irr::KEY_KEY_A))
        camanim->moveXY(CAMERA_STEP,0.);

      if(receiver.IsKeyDown(irr::KEY_KEY_S)) 
        camanim->moveXY(-CAMERA_STEP,0.);

      if (driver->getFPS() != lastFPS)
      {
        lastFPS = driver->getFPS();
        core::stringw tmp = L"gracing (FPS: ";
        tmp += lastFPS;
        tmp += ")";
        device->setWindowCaption(tmp.c_str());
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

#if 0
#endif

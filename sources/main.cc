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
#include "VehicleCameraAnimator.h"
#include "IrrDebugDrawer.h"
#include "DataRecorder.h"
#include "GUISpeedometer.h"


//#include <cegui.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#define CAMERA_STEP 0.05

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

class MyEventReceiver : public IEventReceiver
{
  public:
    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
      // Remember whether each key is down or up
      if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
        KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
      }

      return false;
    }

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(EKEY_CODE keyCode) const
    {
      return KeyIsDown[keyCode];
    }

    MyEventReceiver()
    {
      for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
        KeyIsDown[i] = false;
    }

  private:
    // We use this array to store the current state of each key
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
};


/////////// GUI temp code

#if 0
void initGUI()
{
  CEGUI::IrrlichtRenderer* myRenderer = 
    new CEGUI::IrrlichtRenderer( myIrrlichtDevice, true );
}
#endif

/////////// GUI temp code

CFG_PARAM_BOOL(glob_enableDebug)=true;

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

  MyEventReceiver receiver;

  video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

  u32 screenWidth;
  u32 screenHeight;

  resmanager->getScreenHeight(screenHeight);
  resmanager->getScreenWidth(screenWidth);

  GM_LOG("Dimensione: %d,%d\n",screenWidth,screenHeight);

  IrrlichtDevice *device =
    createDevice( driverType, dimension2d<u32>(screenWidth, screenHeight), 16,
        false, false, false, &receiver);

  resmanager->setDevice(device);

  if (!device)
    return 1;

  device->setWindowCaption(L"gracing - rosco-p");

  ISceneManager* smgr = device->getSceneManager();
  IVideoDriver* driver = device->getVideoDriver();
  IGUIEnvironment* guienv = device->getGUIEnvironment();
  PhyWorld * world = PhyWorld::buildMe();
  CCrisMeshFileLoader * mloader=new CCrisMeshFileLoader(smgr,device->getFileSystem());

  IrrDebugDrawer * debugDrawer= 0;
  if(glob_enableDebug) {
    debugDrawer= new IrrDebugDrawer(driver);
    world->setDebugDrawer(debugDrawer);
    debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);
  }

  smgr->addExternalMeshLoader(mloader);

  Track * thetrack;
  thetrack=new Track(device,world,"farm.zip");
  thetrack->load();

  std::string vehpath;
  resmanager->getVehicleCompletePath("sprinter.zip",vehpath);
  Vehicle * vehicle=new Vehicle(
        0, /* smgr->getRootSceneNode(),*/
        device,
        world,
        vehpath.c_str(),0xcafe);

  vehicle->setDebugDrawFlags(Vehicle::db_forwardImpulse | Vehicle::db_sideImpulse);

  GUISpeedometer * smeter=new GUISpeedometer(true,guienv,guienv->getRootGUIElement(),1,
      core::rect<s32>(0,0,200,100));
  vehicle->setSpeedOMeter(smeter);

  vehicle->load();
  vehicle->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);
  vehicle->reset(thetrack->getStartPosition());
  //vehicle->reset(irr::core::vector3df(0.,5.,0.));
  smgr->getRootSceneNode()->addChild(vehicle);


  //std::string fontPath = resmanager->getResourcePath() + "/font.png";
  std::string fontPath = resmanager->getResourcePath() + "/title_font.xml";
	gui::IGUIFont* font_big = guienv->getFont(fontPath.c_str());
	if (font_big) {
		guienv->getSkin()->setFont(font_big);
  }
#if 0
  irr::gui::IGUIStaticText * text=guienv->addStaticText(L"gracing",
      core::rect<s32>(0,0,700,120));
  text->
    enableOverrideColor(true);
  text=guienv->addStaticText(L"demo",
      core::rect<s32>(0,130,700,250));
  text->
    enableOverrideColor(true);
#endif


  irr::scene::ICameraSceneNode * camera;
  camera = smgr->addCameraSceneNode();

  VehicleCameraAnimator * camanim=new
      VehicleCameraAnimator(vehicle);

  camera->addAnimator(camanim);

#if 0
  irr::scene::ILightSceneNode *light;
  light = smgr->addLightSceneNode(0, 
      irr::core::vector3df(10.f,40.f,-5.f),
      irr::video::SColorf(0.2f, 0.2f, 0.2f), 90.f,0xbeda);
#endif

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


  ///////////
#if 0
  btCollisionShape * x_shape= new btBoxShape(btVector3(1.,1.,1.));
  btTransform tr=btTransform::getIdentity();
  tr.setOrigin(btVector3(0.,2.,5.));
  btRigidBody * x_body;
  x_body=world->createRigidBody(0, 1., tr,x_shape);
#endif
  ///////////

  while(device->run() && !done) {
    if(device->isWindowActive()) {

      driver->beginScene(true, true, SColor(255,100,101,140));
      smgr->drawAll();
      world->step();
      guienv->drawAll();
      world->debugDrawWorld();
      driver->endScene();

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
          vehicle->reset(thetrack->getStartPosition());
          GM_LOG("start position: %f,%f,%f\n",
              thetrack->getStartPosition().X,
              thetrack->getStartPosition().Y,
              thetrack->getStartPosition().Z);
          flagC=false;
        }
      } else {
        flagC=true;
      }

      if(receiver.IsKeyDown(irr::KEY_UP)) {
        if(vehicle)
          vehicle->throttleUp();
      }

      if(receiver.IsKeyDown(irr::KEY_KEY_W))
        camanim->moveXY(0.,CAMERA_STEP);

      if(receiver.IsKeyDown(irr::KEY_KEY_Z)) 
        camanim->moveXY(0.,-CAMERA_STEP);

      if(receiver.IsKeyDown(irr::KEY_KEY_A))
        camanim->moveXY(CAMERA_STEP,0.);

      if(receiver.IsKeyDown(irr::KEY_KEY_S)) 
        camanim->moveXY(-CAMERA_STEP,0.);

      if(receiver.IsKeyDown(irr::KEY_DOWN)) {
        if(vehicle)
          vehicle->throttleDown();
      }

      if(receiver.IsKeyDown(irr::KEY_LEFT)) {
        if(vehicle)
          vehicle->steerLeft();
      }

      if(receiver.IsKeyDown(irr::KEY_RIGHT)) {
        if(vehicle) {
          vehicle->steerRight();
        }
      }

      if(receiver.IsKeyDown(irr::KEY_KEY_I)) {
        if(flagI) {
          if(vehicle) {
            GM_LOG("Vehicle position: %f,%f,%f\n",
                vehicle->getPosition().X,
                vehicle->getPosition().Y,
                vehicle->getPosition().Z);
            dumpNode(smgr->getRootSceneNode());
            world->dumpBodyPositions();
            if(vehicle)
              vehicle->dumpDebugInfo();
          }
          flagI=false;
        } 
      } else flagI=true;


      if(receiver.IsKeyDown(irr::KEY_KEY_D)) {
        if(flagD) {
          vehicle->applyTorque(1000.,1000.,0.);
          GM_LOG("sssssssss\n");
          flagD=false;
        }
      } else {
        flagD=true;
      }

      if (driver->getFPS() != lastFPS)
      {
        lastFPS = driver->getFPS();
        core::stringw tmp = L"Irrlicht SplitScreen-Example (FPS: ";
        tmp += lastFPS;
        tmp += ")";
        device->setWindowCaption(tmp.c_str());
      }
    } else {
      device->yield();
    }
  }
  delete debugDrawer;
}


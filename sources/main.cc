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
#include "Track.hh"
#include "Vehicle.h"
#include "CCrisMeshFileLoader.h"
#include "PhyWorld.h"

#include "gmlog.h"

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifndef BASE_DIR
#define BASE_DIR "."
#endif

#ifdef _WIN32_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
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
  MyEventReceiver receiver;
  video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

  GM_LOG("--------- starting gracing\n");

  IrrlichtDevice *device =
    createDevice( driverType, dimension2d<u32>(800, 600), 16,
        false, false, false, &receiver);

  if (!device)
    return 1;

  device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

  IVideoDriver* driver = device->getVideoDriver();
  ISceneManager* smgr = device->getSceneManager();
  IGUIEnvironment* guienv = device->getGUIEnvironment();
  PhyWorld * world=new PhyWorld();

  CCrisMeshFileLoader * mloader=new CCrisMeshFileLoader(smgr,device->getFileSystem());

  smgr->addExternalMeshLoader(mloader);

  guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!",
      rect<s32>(10,10,260,22), true);

  Track * track = new Track(device,world,BASE_DIR "/track-1.zip");
  IVehicle * vehicle=0; // = new Vehicle(0,device,world,BASE_DIR "/car_ab.zip");


  bool done=false;
  while(device->run() && !done)
  {
    world->step();
    driver->beginScene(true, true, SColor(255,100,101,140));

    smgr->drawAll();
    guienv->drawAll();

    driver->endScene();

    if(receiver.IsKeyDown(irr::KEY_KEY_L) && vehicle==0) {
      vehicle = new Vehicle(smgr->getRootSceneNode(),device,world,BASE_DIR "/car_ab.zip");
      vehicle->load();
      vehicle->use(IVehicle::USE_GRAPHICS);

      scene::ISceneNodeAnimator* anim =
        smgr->createRotationAnimator(core::vector3df(0.8f, 0, 0.8f));

      if(anim)
      {
        vehicle->addAnimator(anim);

        /*
           I'm done referring to anim, so must
           irr::IReferenceCounted::drop() this reference now because it
           was produced by a createFoo() function. As I shouldn't refer to
           it again, ensure that I can't by setting to 0.
           */
        anim->drop();
        anim = 0;
      }



      //scene::ISceneNodeAnimator* anim =
      //  smgr->createRotationAnimator(core::vector3df(0.8f, 0, 0.8f));

      //smgr->addSceneNode(vehicle);
    }

    if(receiver.IsKeyDown(irr::KEY_KEY_U) && vehicle) {
      vehicle->drop();
      vehicle=0;
    }

    if(receiver.IsKeyDown(irr::KEY_ESCAPE)) 
      done=true;
  }

  delete track;

  device->drop();

  return 0;
}

/*
   That's it. Compile and run.
 **/

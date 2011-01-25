#include <irrlicht.h>
#include "Track.hh"
#include "CCrisMeshFileLoader.h"

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

#ifdef _IRR_WINDOWS_
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
        printf("key pressed '%d'\n",event.KeyInput.Key);
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

int main()
{
  MyEventReceiver receiver;
  video::E_DRIVER_TYPE driverType=video::EDT_OPENGL;

  GM_LOG("--------- starting gracing\n");

  IrrlichtDevice *device =
    createDevice( driverType, dimension2d<u32>(640, 480), 16,
        false, false, false, &receiver);

  if (!device)
    return 1;

  device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

  IVideoDriver* driver = device->getVideoDriver();
  ISceneManager* smgr = device->getSceneManager();
  IGUIEnvironment* guienv = device->getGUIEnvironment();


  CCrisMeshFileLoader * mloader=new CCrisMeshFileLoader(smgr,device->getFileSystem());

  smgr->addExternalMeshLoader(mloader);

  guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!",
      rect<s32>(10,10,260,22), true);

  //smgr->addCameraSceneNodeFPS();


  Track * track = new Track(device,BASE_DIR "/track-1.zip");

  GM_LOG("---------------------------------------\n");

  bool done=false;
  while(device->run() && !done)
  {
    driver->beginScene(true, true, SColor(255,100,101,140));

    smgr->drawAll();
    guienv->drawAll();

    driver->endScene();
    if(receiver.IsKeyDown(irr::KEY_ESCAPE)) {
      done=true;
    }
  }

  device->drop();

  return 0;
}

/*
   That's it. Compile and run.
 **/

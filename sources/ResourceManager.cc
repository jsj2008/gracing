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

#include <string>
#include <dirent.h>


#include "ResourceManager.h"
#include "XmlNode.h"
#include "config.h"
#include "util.hh"
#include "gmlog.h"
#include "EventReceiver.h"
#include "CCrisMeshFileLoader.h"
#include "GuiMenu.h"
#include "JoystickInterface.h"

/* phase handlers */
#include "IPhaseHandler.h"
#include "Race.h"
#include "VehicleChooser.h"

// vehicle controllers
#include "VehicleKeyboardController.h"
#include "VehicleNullController.h"
#include "VehicleAutoController.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
} 


extern bool globalDone;
static EventReceiver g_eventReceiver;

int log(lua_State * L)
{
  const char * str;
  if((str=luaL_checklstring(L,1,0))) {
    GM_LOG("%s",str);
  }
  return 0;
}

int startRace(lua_State * L)
{
  unsigned hum,tot;
  hum=luaL_checknumber(L,1);
  tot=luaL_checknumber(L,2);
  ResourceManager::getInstance()->startRace(hum,tot);
  return 0;
}

int showMenu(lua_State * L)
{
  const char * str;
  if((str=luaL_checklstring(L,1,0))) {
    ResourceManager::getInstance()->showMenu(str,true);
  }
  return 0;
}

int endRace(lua_State * L)
{
  ResourceManager::getInstance()->endRace();
  return 0;
}

int hideMenu(lua_State * L)
{
  ResourceManager::getInstance()->hideMenu();
  return 0;
}

int quit(lua_State * L)
{
  globalDone = true;
  return 0;
}

struct embFunctions_s {
  const char * n;
  int (*f)(lua_State*);
} embFunctions[] =
{
  { "log",log },
  { "showMenu", showMenu },
  { "hideMenu", hideMenu },
  { "startRace", startRace },
  { "endRace", endRace },
  { "quit", quit },
  { 0,0 }
};

static int registerLua(lua_State * L)
{
  lua_newtable(L);
  int table=lua_gettop(L);

  lua_pushstring(L, "GR");
  lua_pushvalue(L, table);
  lua_settable(L, LUA_GLOBALSINDEX);


  for(int i=0; embFunctions[i].n; i++) {
    lua_pushstring(L, embFunctions[i].n);
    lua_pushcfunction(L, embFunctions[i].f);
    lua_settable(L,table);
  }

  lua_pop(L,1);

  return 0;
}



/////////////////////////////////////////////////////////////////////////////////


#define  DEFAULT_FONT       "droid-serif-32.xml"
#define  DEFAULT_FONT_SMALL "droid-serif-24.xml"
#define  DEFAULT_FONT_BIG    "droid-serif-64.xml"


#ifdef __APPLE__
#  include <CoreFoundation/CoreFoundation.h>


CFG_PARAM_UINT(glob_screenWidth)=800;
CFG_PARAM_UINT(glob_screenHeight)=600;
//CFG_PARAM_UINT(glob_screenWidth)=1280;
//CFG_PARAM_UINT(glob_screenHeight)=800;

static unsigned listDirectory(const char* path, 
    std::vector<std::string> &return_files, 
    //const std::vector<std::string> *allowedExtensions, 
    int type) 
{ 
  unsigned fileCount = 0; 
  DIR *searchDir = opendir(path); 
  if(searchDir != NULL) { 
    struct dirent *entry; 
    while((entry=readdir(searchDir))!=NULL) {
      if(entry->d_type == type) 
        if(std::string(entry->d_name)!="." && std::string(entry->d_name)!="..") { 
            return_files.push_back(entry->d_name); 
            fileCount++; 
        } 
    }
    closedir(searchDir);
  } 
  return fileCount; 
}; 

static bool macGetBundlePath(std::string& data_dir)
{
    // the following code will enable STK to find its data when placed in 
    // an app bundle on mac OS X.
    // returns true if path is set, returns false if path was not set
    char path[1024];
    CFBundleRef main_bundle = CFBundleGetMainBundle(); assert(main_bundle);
    CFURLRef main_bundle_URL = CFBundleCopyBundleURL(main_bundle); assert(main_bundle_URL);
    CFStringRef cf_string_ref = CFURLCopyFileSystemPath( main_bundle_URL, kCFURLPOSIXPathStyle); assert(cf_string_ref);
    CFStringGetCString(cf_string_ref, path, 1024, kCFStringEncodingASCII);
    CFRelease(main_bundle_URL);
    CFRelease(cf_string_ref);

    std::string contents = std::string(path) + std::string("/Contents");
    if(contents.find(".app") != std::string::npos)
    {
        // executable is inside an app bundle, use app bundle-relative paths
        data_dir = contents + std::string("/Resources/");
        return true;
    }
    else
    {
        return false;
    }
}
#endif

using namespace irr;

ResourceManager * ResourceManager::s_instance;


irr::io::path ResourceManager::createAbsoluteFilename(const std::string & fileName)
{
    io::path abs_path=m_fileSystem->getAbsolutePath(fileName.c_str());
    abs_path=m_fileSystem->flattenFilename(abs_path);
    return abs_path;
}

ResourceManager::ResourceManager() 
{

#ifdef __APPLE__
  char buffer[1024];
  getcwd(buffer,1024);
#endif

  m_device = createDevice(video::EDT_NULL);

#ifdef __APPLE__
  chdir(buffer);
#endif

  m_fileSystem=m_device->getFileSystem();

#ifdef __APPLE__
  assert(macGetBundlePath(m_rootDir));
#else
#error "Still not implemented"
#endif

  m_device->grab();

  GM_LOG("** creating lua interpreter\n");
  m_lua = lua_open();


  luaopen_base(m_lua);
  luaopen_string(m_lua);
  luaopen_table(m_lua);
  luaopen_math(m_lua);
  //luaopen_io(m_lua); // dont know why it is not working
  luaopen_debug(m_lua);

  registerLua(m_lua);

  //Lunar<LuaBridge>::Register(m_lua);

  std::string configFilename;
  getConfigCompletePath("config.xml",configFilename);
  loadConfig(configFilename);

  ///////////////////////////////////////
  m_screenWidth=glob_screenWidth;
  m_screenHeight=glob_screenHeight;

  m_humanVehicles=1;
  m_totVehicles=4;
  m_mustStartRace=false;
  //{ m_max_vehicles=4 };
  //unsigned   m_choosenVehicles[m_max_vehicles];

}

void ResourceManager::loadConfig(const std::string & filename)
{
  m_configRoot=new XmlNode(filename,this);

  if(!m_configRoot)
    return;

  ConfigInit::initGlobVariables(this);
}

void ResourceManager::getTrackCompletePath(const char * trackName, std::string & path)
{
  path=m_trackDir + std::string(trackName);
}

void ResourceManager::getVehicleCompletePath(const char * vehicleName, std::string & path)
{    
  path=m_vehicleDir + std::string(vehicleName);
}

void ResourceManager::getTexturesCompletePath(const char * texturesName, std::string & path)
{    
  path=m_texturesDir + std::string(texturesName);
}


void ResourceManager::getConfigCompletePath(const char * filename, std::string & path)
{
  path=m_rootDir + std::string(filename);
}

void ResourceManager::getResourceCompletePath(const char * filename, std::string & path)
{
  path=m_rootDir + std::string(filename);
}

void ResourceManager::setDevice(irr::IrrlichtDevice *device)
{
  if(m_device)
    m_device->drop();

  m_device=device;
  m_fileSystem=m_device->getFileSystem();
  m_world = PhyWorld::buildMe();

  m_trackDir = m_rootDir + std::string("/Tracks/");
  m_vehicleDir = m_rootDir + std::string("/Vehicles/");
  m_texturesDir = m_rootDir + std::string("/Textures/");

  
  ///////////////////
  // input devices //
  ///////////////////

  // - joystick(s)
  core::array<SJoystickInfo> devices;
  JoystickInterface *    joystickInterface;
  if(device->activateJoysticks(devices)) 
    for(u32 joystick = 0; joystick < devices.size(); ++joystick) {
      joystickInterface=new JoystickInterface(m_device,devices[joystick]);
      g_eventReceiver.addListener(joystickInterface); // <- ??
      m_inputDevices.push_back(joystickInterface);
      
    }



  /////////////////////////
  // custom mesh loaders // 
  /////////////////////////
  irr::scene::ISceneManager* smgr = device->getSceneManager();
  CCrisMeshFileLoader * mloader=new CCrisMeshFileLoader(smgr,device->getFileSystem());
  smgr->addExternalMeshLoader(mloader);

  ////////////////////
  // load resources //
  ////////////////////

  // system font
  std::string fontName; 
  if(!cfgGet("system-font",fontName)) 
    fontName = DEFAULT_FONT;
  irr::gui::IGUIEnvironment* guienv = device->getGUIEnvironment();
  std::string fontPath = getResourcePath() + "/" + fontName;
  m_font = guienv->getFont(fontPath.c_str());
  assert(m_font);
  GM_LOG("loading done font\n");

  GM_LOG("loading big font\n");
  if(!cfgGet("system-font",fontName)) 
    fontName = DEFAULT_FONT_BIG;
  fontPath = getResourcePath() + "/" + fontName;
  m_fontBig = guienv->getFont(fontPath.c_str());
  assert(m_font);
  GM_LOG("loading done font\n");

  // system font small
  GM_LOG("loading small daf font\n");
  if(!cfgGet("system-font-small",fontName)) 
    fontName = DEFAULT_FONT_SMALL;

  fontPath = getResourcePath() + "/" + fontName;
  m_fontSmall = guienv->getFont(fontPath.c_str());
  assert(m_fontSmall);
  GM_LOG("loading done font\n");

  /////////////////////////////////
  // load menus                  //
  /////////////////////////////////
  irr::core::rect<irr::s32> rect;
  m_menu = new GuiMenu(device->getGUIEnvironment(),
      device->getGUIEnvironment()->getRootGUIElement(),0,rect);
  m_menu->setVisible(true);
  m_menu->setHasFrame(true);
  m_menu->load("menu.xml");
  m_menu->setGroup(L"main");
  m_menu->centerOnTheScreen();
  getEventReceiver()->addListener(m_menu);

  /////////////////////////////////
  // load menus                  //
  /////////////////////////////////
  loadVehicles();

  /////////////////////////////////
  // phase handlers menus       //
  /////////////////////////////////
  m_phaseHandlers[pa_vehicleChooser]=new VehicleChooser(device,m_world);
  m_phaseHandlers[pa_race] = new Race(device,m_world);
  m_phaseHandlers[pa_empty] = new EmptyPhaseHandler(device,m_world);
  m_currentPhaseHandler = m_phaseHandlers[pa_empty];

  const XmlNode * defaultTrack;
  cfgGet("default-track",defaultTrack);

  if(defaultTrack) {
    std::string name;
    defaultTrack->get("name",name);
    m_track=new Track(m_device,m_world,name.c_str());
    GM_LOG("got the default track %s!!!!\n",name.c_str());
  }

  //thetrack=new Track(device,world,"farm.zip");
  //thetrack=new Track(device,world,"devtrack.zip");
  //thetrack=new Track(device,world,"tuxtollway.zip");
  //thetrack=new Track(device,world,"jungle.zip");
  //thetrack=new Track(device,world,"beach.zip");

  if(!m_track) 
    m_track=new Track(m_device,m_world,"farm.zip");
}

bool ResourceManager::cfgGet(const char * name, bool & value)
{
  if(!m_configRoot)
    return false;

  const XmlNode * node=m_configRoot->getChild(name);

  if(!node) 
    return false;

  std::string text=node->getText();


  value=false;
  if(text == "yes" ||
     text == "true")
    value=true;

  return true;
}

bool ResourceManager::cfgGet(const char * nodeName, const XmlNode * & node)
{
  if(!m_configRoot)
    return false;

  node = m_configRoot->getChild(nodeName);

  return (node);
}

bool ResourceManager::cfgGet(const char * name, std::string & value)
{
  if(!m_configRoot)
    return false;

  const XmlNode * node=m_configRoot->getChild(name);

  if(!node) 
    return false;

  std::string text=node->getText();

  value=text;

  return true;
}

bool ResourceManager::cfgGet(const char * name, unsigned & value)
{
  if(!m_configRoot)
    return false;

  const XmlNode * node=m_configRoot->getChild(name);

  if(!node) 
    return false;

  std::string text=node->getText();

  value=Util::parseUnsigned(text.c_str());
  return true;
}

bool ResourceManager::cfgGet(const char * name, double & value)
{
  if(!m_configRoot)
    return false;

  const XmlNode * node=m_configRoot->getChild(name);

  if(!node) 
    return false;

  std::string text=node->getText();

  value=Util::parseFloat(text.c_str());
  return true;
}

bool ResourceManager::cfgGet(const char * name, double value[3])
{
  if(!m_configRoot)
    return false;

  const XmlNode * node=m_configRoot->getChild(name);

  if(!node) 
    return false;

  std::string text=node->getText();

  Util::parseVector(text.c_str(),value);
  return true;
}

EventReceiver * ResourceManager::getEventReceiver()
{
  return &g_eventReceiver;
}
    
void ResourceManager::loadVehicles()
{
  // first retrieve the list of files
  std::vector<std::string> files;
  int type=DT_REG;
  unsigned nFiles;

  nFiles=listDirectory(m_vehicleDir.c_str(),files,type);
  std::string vpath;
  IVehicle * vehicle;

  for(unsigned i=0; i < nFiles; i++) {
    getVehicleCompletePath(files[i].c_str(), vpath);
    // TODO: should be use a factory like loader??
    vehicle=new Vehicle(
        0, 
        m_device,
        m_world,
        vpath.c_str(),0xcafe);
    vehicle->load();
    GM_LOG("loaded '%s'\n",vpath.c_str());
    vehicle->grab();
    m_vehicles.push_back(vehicle);
  }
}

void ResourceManager::showMenu(const std::wstring & name, bool centerOnTheScreen) 
{
  m_menu->setGroup(name);
  if(centerOnTheScreen)
    m_menu->centerOnTheScreen();
  g_eventReceiver.resetOneShotKey();
  m_menu->setVisible(true);
}

void ResourceManager::showMenu(const std::string & name,bool centerOnTheScreen) 
{
  std::wstring wname(name.begin(),name.end());
  m_menu->setGroup(wname);
  if(centerOnTheScreen)
    m_menu->centerOnTheScreen();
  g_eventReceiver.resetOneShotKey();
  m_menu->setVisible(true);
}

void ResourceManager::hideMenu()
{
  g_eventReceiver.resetOneShotKey();
  m_menu->setVisible(false);
}

////////////////////////////////////////////////////////////////////////////////////// LUA STUFF
void ResourceManager::lua_doFile(const char * filename)
{
  std::string fn = filename;
  std::string completePath = getResourcePath() + "/" + fn;

  GM_LOG("executing file: '%s'\n",completePath.c_str());
  if(luaL_dofile(m_lua,completePath.c_str()))
    GM_LOG("%s\n", lua_tostring(m_lua, -1));
}


void ResourceManager::lua_doString(const char * script)
{
  if(luaL_dostring(m_lua,script))
    GM_LOG("%s\n", lua_tostring(m_lua, -1));
}

void ResourceManager::startRace(unsigned humanVehicles, unsigned totVehicles)
{
  m_mustStartRace=true;
  m_humanVehicles=humanVehicles;
  m_totVehicles=totVehicles;
}

void ResourceManager::stepPhaseHandler() { 
  // TODO: this function is very very very very ugly!
  //       dont like any thing in it!
  bool done;

  if(m_mustStartRace) {
    m_mustStartRace = false;
    m_humanVehicles=1;
    hideMenu();
    static_cast<VehicleChooser*>(m_phaseHandlers[pa_vehicleChooser])->prepare(
                                 m_humanVehicles,m_totVehicles,m_choosenVehicles);
    m_currentPhaseHandler= m_phaseHandlers[pa_vehicleChooser];
    return;
  } 

  
  
  done=m_currentPhaseHandler->step();

  if(done || m_mustEndRace) {
    if(m_currentPhaseHandler == m_phaseHandlers[pa_vehicleChooser]) {

      m_currentPhaseHandler->unprepare();

      static_cast<Race*>(m_phaseHandlers[pa_race])->setTrack(m_track);

      const std::vector<IVehicle*> & vehicles=getVehiclesList();
      assert(vehicles.size() >= 4);

      for(unsigned i=0; i < m_totVehicles; i++) {
        GM_LOG("adding vehicle '%s'\n",
              vehicles[m_choosenVehicles[i]]->getName().c_str());
        if(i < m_humanVehicles) 
          static_cast<Race*>(m_phaseHandlers[pa_race])->addVehicle(vehicles[m_choosenVehicles[i]],
              new VehicleKeyboardController(getEventReceiver()), 
              vehicles[m_choosenVehicles[i]]->getName().c_str(),true);
        else
          static_cast<Race*>(m_phaseHandlers[pa_race])->addVehicle(
              vehicles[m_choosenVehicles[i]],
              new VehicleAutoController(),
              vehicles[m_choosenVehicles[i]]->getName().c_str(),false);
      }
      static_cast<Race*>(m_phaseHandlers[pa_race])->restart();
      m_currentPhaseHandler = m_phaseHandlers[pa_race];
      GM_LOG("staring race\n");
    } else if(m_currentPhaseHandler == m_phaseHandlers[pa_race]) {
      m_mustEndRace = false;
      m_phaseHandlers[pa_race]->unprepare();
      m_currentPhaseHandler = m_phaseHandlers[pa_empty];
      showMenu("main");
    }

  }
}

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

struct ControllerChooser {
  unsigned deviceId;
  unsigned controllerId;
  std::vector<IDeviceInterface*> & deviceList;
  ControllerChooser(std::vector<IDeviceInterface*> & deviceList) 
    : deviceList(deviceList)
  { 
    deviceId=0; 
    controllerId=0;
  }

  IVehicleController * getController()
  {
    if(deviceId == deviceList.size())
      return 0;
    if(controllerId == deviceList[deviceId]->getNumController()) {
      if(++deviceId == deviceList.size())
        return 0;
      controllerId=0;
    }
    return deviceList[deviceId]->getController(controllerId++);
  }
}; 

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
  GM_LOG("starting race %d uhuman\n",hum);
  ResourceManager::getInstance()->startRace(hum,tot);
  return 0;
}

int addController(lua_State * L)
{
  unsigned dev;
  dev=luaL_checknumber(L,1);
  ResourceManager::getInstance()->addControllerToDevice(dev);
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

int resumeRace(lua_State * L)
{
  ResourceManager::getInstance()->hideMenu();
  ResourceManager::getInstance()->resumeRace();
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

int getInputDevices(lua_State * L) 
{
  lua_newtable(L);

  int tb=lua_gettop(L);

  std::vector<std::string> list;

  ResourceManager::getInstance()->getInputDeviceList(list);

  for(unsigned i=0; i < list.size(); i++) {
    lua_pushnumber(L, i+1);
    lua_pushstring(L, list[i].c_str()) ; //el[i]);
    lua_settable(L,tb);
  }
  return 1;
}

int getControllerActionName(lua_State * L)
{
  unsigned deviceId;
  unsigned controllerId;
  unsigned actionId;

  deviceId=luaL_checkinteger(L,1);
  controllerId=luaL_checkinteger(L,2);
  actionId=luaL_checkinteger(L,3);

  const char * name;
  name=ResourceManager::getInstance()->getControllerActionName(deviceId,controllerId,actionId);
  lua_pushstring(L,name);
  return 1;
}

int getActionsList(lua_State * L)
{

  unsigned deviceId;
  unsigned controllerId;

  deviceId=luaL_checkinteger(L,1);
  controllerId=luaL_checkinteger(L,2);

  lua_newtable(L);

  int tb=lua_gettop(L);

  std::vector<std::string> list;

  ResourceManager::getInstance()->getControllerActions(deviceId,controllerId,list);

  for(unsigned i=0; i < list.size(); i++) {
    lua_pushnumber(L, i+1);
    lua_pushstring(L, list[i].c_str()) ; 
    lua_settable(L,tb);
  }
  return 1;
}

int startLearnAction(lua_State * L)
{
  unsigned dev,contr,act;
  const char * callback;
  dev=luaL_checkinteger(L,1);
  contr=luaL_checkinteger(L,2);
  act=luaL_checkinteger(L,3);
  callback=luaL_checkstring(L,4);
  ResourceManager::getInstance()->startControllerLearning(dev,contr,act,callback);
  return 0;
}

int getInputDeviceNumControllers(lua_State * L)
{
  int deviceId;
  deviceId=luaL_checkinteger(L,1);
  unsigned numControllers=
      ResourceManager::getInstance()->getInputDeviceNumControllers(deviceId);
  lua_pushnumber(L,numControllers);
  return 1;
}

int setConfig(lua_State * L) {
  const char * key;
  const char * value;
  bool  res=false;
  if((key=luaL_checklstring(L,1,0)) && (value=luaL_checklstring(L,2,0))) {
    res=ResourceManager::getInstance()->cfgSet(key,value);
  }
  lua_pushboolean(L,res);
  return 1;
}

int setSplitScreenModality(lua_State * L) {
  int l;
  l=luaL_checknumber(L,1);
  ResourceManager::getInstance()->setSplitScreenModality(l);
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
  { "resumeRace", resumeRace },
  { "quit", quit },
  { "setConfig", setConfig },
  { "setSplitScreenMode", setSplitScreenModality },
  { "getInputDevices", getInputDevices },
  { "getInputDeviceNumControllers", getInputDeviceNumControllers },
  { "getActionsForController",getActionsList },
  { "getControllerActionName", getControllerActionName },
  { "startLearnAction", startLearnAction },
  { "addController", addController },
  { 0,0 }
};

// TODO: please decide where to put the following funciton
static XmlNode * runPath(const char * path, XmlNode * root,bool create=false)
{
  char buffer[1024];
  char * ptr;
  char * namePtr;
  strncpy(buffer,path,1024);
  ptr=buffer;

  //GM_LOG("getting node from path '%s'\n",path);

  XmlNode * father=root;
  XmlNode * node;

  while(*ptr) {
    namePtr=ptr;
    for( ; *ptr && *ptr != '/'; ptr++) 
      ;
    *ptr=0;
    //GM_LOG("  getting node '%s'\n",namePtr);
    node = father->getChild(namePtr);
    if(!node) {
      if(create)
        node=father->addChild(namePtr);
      else
        return 0;
    }
    assert(node);
    father=node;
    ptr++;
  }

  return node;
}

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

  GM_LOG("loading config--------------------------------------------------\n");
  std::string configFilename;
  getConfigCompletePath("config.xml",configFilename);
  loadConfig(configFilename);

  ///////////////////////////////////////
  m_screenWidth=glob_screenWidth;
  m_screenHeight=glob_screenHeight;

  unsigned screenResolution=0;

  if(cfgGet("video/resolution",screenResolution)) {
    switch(screenResolution) {
      case 0:
        m_screenWidth=800;
        m_screenHeight=600;
      break;
      case 1:
        m_screenWidth=1024;
        m_screenHeight=600;
      break;
      case 2:
        m_screenWidth=1024;
        m_screenHeight=768;
      break;
      case 3:
        m_screenWidth=1152;
        m_screenHeight=720;
      break;
      case 4:
        m_screenWidth=1280;
        m_screenHeight=800;
      break;
    }
  }
  
  m_humanVehicles=1;
  m_totVehicles=4;
  m_mustStartRace=false;
  m_mustResumeRace=false;
  m_controllerLearning=0;
  m_mustEndRace=false;
  //{ m_max_vehicles=4 };
  //unsigned   m_choosenVehicles[m_max_vehicles];

}

void ResourceManager::setSplitScreenModality(int l)
{
  static_cast<Race*>(m_phaseHandlers[pa_race])->setSplitModality(l);
}

void ResourceManager::loadConfig(const std::string & filename)
{
  m_configRoot=new XmlNode(filename,this);

  if(!m_configRoot)
    return;

  ConfigInit::initGlobVariables(this);
}

void ResourceManager::saveConfig(const std::string & filename)
{
  // get configuration from device interfaces
  XmlNode * devicesRoot;

  devicesRoot = m_configRoot->getChild("input-devices");
  if(!devicesRoot) {
    devicesRoot = m_configRoot->addChild("input-devices");
  }

  for(unsigned i=0; i<m_inputDevices.size(); i++) {
    IDeviceInterface * device=m_inputDevices[i];
    XmlNode * node;
    std::string name;
    name=device->getName();
    node=devicesRoot->getChildByAttr("name",name);
    if(!node) {
      node=devicesRoot->addChild("device");
      node->set("name",name);
    }

    device->getConfiguration(node);
  }

  m_configRoot->save(filename);
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

void ResourceManager::startControllerLearning(unsigned deviceId, 
    unsigned controllerId, unsigned action, const char * callback)
{
  if(deviceId < m_inputDevices.size() &&
     controllerId < m_inputDevices[deviceId]->getNumController()) {
    m_controllerLearning = 
       m_inputDevices[deviceId]->getController(controllerId);
    m_controllerLearning->startLearnAction(action);
    if(callback) 
      m_genericCallbackCode=callback;
    GM_LOG("start learning controller\n");
  }
}

void ResourceManager::stopControllerLearning()
{
 if(m_controllerLearning) {
   m_controllerLearning=0;
   lua_doString(m_genericCallbackCode.c_str());
 }
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

  // - keyboard
  KeyboardInterface * keyboardInterface;
  keyboardInterface = new KeyboardInterface(&g_eventReceiver);

  // - joystick(s)
  core::array<SJoystickInfo> devices;
  JoystickInterface *    joystickInterface;
  if(device->activateJoysticks(devices)) 
    for(u32 joystick = 0; joystick < devices.size(); ++joystick) {
      joystickInterface=new JoystickInterface(m_device,devices[joystick]);
      m_inputDevices.push_back(joystickInterface);
    }

  // NB: keyboard is pushed last 
  //     because the order in which the devices
  //     appear in the m_inputDevices vector
  //     is the order they are choosen in the race
  //     (and keyboard must be used only when
  //      there is no joystick)
  m_inputDevices.push_back(keyboardInterface);


  // set the configuration for the input devices
  XmlNode * inputDevicesNode;
  const char * id="input-devices";
  inputDevicesNode=m_configRoot->getChild(id);
  if(inputDevicesNode) {
    for(unsigned i=0; i < m_inputDevices.size(); i++) {
      IDeviceInterface * di=m_inputDevices[i];
      GM_LOG("looking for configuration for '%s':",di->getName().c_str());
      XmlNode * node=inputDevicesNode->getChildByAttr("name",di->getName());
      if(node) {
        di->setConfiguration(node);
        GM_LOG("found\n");
      } else {
        GM_LOG("not found\n");
      }
    }
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
  //m_menu->setGroup(L"racesetup");
  m_menu->centerOnTheScreen();
  getEventReceiver()->addListener(m_menu);

  /////////////////////////////////
  // load menus                  //
  /////////////////////////////////
  loadVehicles();

  /////////////////////////////////
  // phase handlers menus        //
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
    m_track=new Track(m_device,m_world,"toylevel.zip");
}


void ResourceManager::saveConfig()
{
  std::string configFilename;
  getConfigCompletePath("config.xml",configFilename);
  saveConfig(configFilename);
}


bool ResourceManager::cfgGet(const char * name, bool & value)
{
  if(!m_configRoot)
    return false;

  //const XmlNode * node=m_configRoot->getChild(name);
  const XmlNode * node=runPath(name,m_configRoot);

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

  //const XmlNode * node=m_configRoot->getChild(name);
  const XmlNode * node=runPath(name,m_configRoot);

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

  //const XmlNode * node=m_configRoot->getChild(name);
  const XmlNode * node=runPath(name,m_configRoot);

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

  //const XmlNode * node=m_configRoot->getChild(name);
  const XmlNode * node=runPath(name,m_configRoot);

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

  //const XmlNode * node=m_configRoot->getChild(name);
  const XmlNode * node=runPath(name,m_configRoot);

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
  m_menu->getParent()->bringToFront(m_menu);
  m_menu->setVisible(true);

  // DEBUG DEBUG

  irr::gui::IGUIElement * e=m_menu->getParent();

  const irr::core::list<irr::gui::IGUIElement *> & list= e->getChildren();

  irr::core::list<irr::gui::IGUIElement *>::ConstIterator it=list.begin();

  for(it=list.begin(); it != list.end(); it++) {
    GM_LOG("--->%p\n",*it);
  }



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
  //      
  //       for example, the general policy used
  //       to serve 'lua' requests (startRace, endRace,....)
  //       is to set a flag related to the operation to do.
  //       this is __VERY__ ugly.
  bool done;

  if(m_mustStartRace) {
    m_mustStartRace = false;
    hideMenu();
    GM_LOG("must start race: %d\n",m_humanVehicles);
    m_currentPhaseHandler= m_phaseHandlers[pa_vehicleChooser];
    static_cast<VehicleChooser*>(m_phaseHandlers[pa_vehicleChooser])->prepare(
          m_humanVehicles,m_totVehicles,m_choosenVehicles);
    return;
  } 

  if(m_controllerLearning) {
    if(!m_controllerLearning->isLearningAction() || 
        getEventReceiver()->OneShotKey(irr::KEY_ESCAPE)) {
      stopControllerLearning();
    }
  }

  if(m_mustResumeRace && m_currentPhaseHandler == m_phaseHandlers[pa_race]) {
    static_cast<Race*>(m_currentPhaseHandler)->togglePause();
    m_mustResumeRace=false;
  }

  
  if(!done)
    done=m_currentPhaseHandler->step();


  if(done || m_mustEndRace) {
    if(m_currentPhaseHandler == m_phaseHandlers[pa_vehicleChooser]) {
      ControllerChooser controllerChooser(m_inputDevices);

      m_currentPhaseHandler->unprepare();

      static_cast<Race*>(m_phaseHandlers[pa_race])->setTrack(m_track);

      const std::vector<IVehicle*> & vehicles=getVehiclesList();
      assert(vehicles.size() >= 4);

      for(unsigned i=0; i < m_totVehicles; i++) {
        IVehicleController * controller;
        bool followed;
        if(i < m_humanVehicles) {
          controller=controllerChooser.getController();
          followed=true;
        } else {
          controller=new VehicleAutoController();
          followed=false;
        }

        static_cast<Race*>(m_phaseHandlers[pa_race])->addVehicle(
            vehicles[m_choosenVehicles[i]],
            controller,
            vehicles[m_choosenVehicles[i]]->getName().c_str(),followed);
      }
      static_cast<Race*>(m_phaseHandlers[pa_race])->restart();
      m_currentPhaseHandler = m_phaseHandlers[pa_race];
      GM_LOG("starting race\n");
    } else if(m_currentPhaseHandler == m_phaseHandlers[pa_race]) {
      m_mustEndRace = false;
      m_phaseHandlers[pa_race]->unprepare();
      m_currentPhaseHandler = m_phaseHandlers[pa_empty];
      showMenu("main");
    }
  }
}


bool ResourceManager::cfgSet(const char * name, unsigned value)
{
  char buffer[64];
  snprintf(buffer,64,"%d",value);
  return cfgSet(name,buffer);
}


bool ResourceManager::cfgSet(const char * name, const char * value)
{
  XmlNode * node;
  node=runPath(name,m_configRoot,true);
  assert(node);
  node->setText(value);

  return true;
}


unsigned ResourceManager::getInputDeviceNumControllers(unsigned deviceIdx)
{
#if 0
  GM_LOG("------------------------------\n");
  for(unsigned i=0; i<m_inputDevices.size(); i++) {
    GM_LOG("[% 2d] '%s', controllers: %d\n",i,m_inputDevices[i]->getName().c_str(),
        m_inputDevices[i]->getNumController());
  }
  GM_LOG("------------------------------\n");
#endif

  if(deviceIdx < m_inputDevices.size()) {
    unsigned num;
    num = m_inputDevices[deviceIdx]->getNumController();
    return num;
  }
  return 0;
}


void ResourceManager::getInputDeviceList(std::vector<std::string> & list)
{
  list.clear();
  for(unsigned i=0; i < m_inputDevices.size(); i++) {
    list.push_back(m_inputDevices[i]->getName());
  }
}

const char * ResourceManager::getControllerActionName(unsigned deviceId,
    unsigned controllerId,unsigned actionId)
{
  if(deviceId < m_inputDevices.size() && 
     controllerId < m_inputDevices[deviceId]->getNumController()) {
    return m_inputDevices[deviceId]->getController(controllerId)->getActionString(actionId);
  }
  return "";
}

void ResourceManager::addControllerToDevice(unsigned deviceId)
{
  if(deviceId < m_inputDevices.size()) 
    m_inputDevices[deviceId]->addController();
}
void ResourceManager::getControllerActions(unsigned deviceId, unsigned controllerId,
    std::vector<std::string> & list)
{
  if(deviceId < m_inputDevices.size() && 
     controllerId < m_inputDevices[deviceId]->getNumController())
  {
    IVehicleController * controller=
      m_inputDevices[deviceId]->getController(controllerId);
    list.clear();
    unsigned numActions=controller->getNumActions();
    for(unsigned i=0; i<numActions; i++) {
      std::string actName;
      controller->getActionSettingString(i,actName);
      //GM_LOG("[%d] '%s'\n",i,actName.c_str());
      list.push_back(actName);
    }
  }
}

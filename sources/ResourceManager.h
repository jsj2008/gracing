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

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#include <string>
#include <vector>
#include <stdlib.h>
#include <irrlicht.h>
#include <lunar.h>

#include "PhyWorld.h"
#include "Vehicle.h"
#include "EventReceiver.h"
#include "EmptyPhaseHandler.h"
#include "Track.hh"

class XmlNode;
class GuiMenu;
class Track;
class IDeviceInterface;
class IVehicleController;
class AudioLayer;


void stupid_store(void * mem, unsigned size);
void stupid_assert(void * mem);

class ResourceManager 
{
  public:

    static inline ResourceManager * getInstance() { if(s_instance==0) s_instance=new ResourceManager(); return s_instance; }

    void setDevice(irr::IrrlichtDevice *device);

    void  stepPhaseHandler(); 

    /* path handling */
    irr::io::path createAbsoluteFilename(const std::string & fileName);
    void getVehicleCompletePath(const char * vehicleName, std::string & path);
    void getTrackCompletePath(const char * trackName, std::string & path);
    void getConfigCompletePath(const char * filename, std::string & path);
    void getResourceCompletePath(const char * filename, std::string & path);
    void getTexturesCompletePath(const char * texturesName, std::string & path);
    void getAudioCompletePath(const char * texturesName, std::string & path);
    inline const std::string & getResourcePath() { return m_rootDir; }
    inline irr::io::IXMLReaderUTF8 * createXMLReaderUTF8(const std::string & filename)
    {
      return m_fileSystem->createXMLReaderUTF8(filename.c_str());
    }
    inline irr::io::IXMLWriter * createXMLWriter(const std::string & filename)
    {
      return m_fileSystem->createXMLWriter(filename.c_str());
    }


    /* system font */
    inline irr::gui::IGUIFont *  getSystemFontSmall() { return m_fontSmall; }
    inline irr::gui::IGUIFont *  getSystemFont() { return m_font; }
    inline irr::gui::IGUIFont *  getSystemFontBig() { return m_fontBig; }

    /* config access */
    bool cfgGet(const char * name, bool & value);
    bool cfgGet(const char * name, double & value);
    bool cfgGet(const char * name, double value[3]);
    bool cfgGet(const char * name, unsigned & value);
    bool cfgGet(const char * name, std::string & value);
    bool cfgGet(const char * nodeName, const XmlNode * & node);

    bool cfgSet(const char * name, const char * value);
    bool cfgSet(const char * name, unsigned value);

    void getInputDeviceList(std::vector<std::string> & list);
    unsigned getInputDeviceNumControllers(unsigned);
    void getControllerActions(unsigned deviceId, unsigned controllerId,
        std::vector<std::string> & list);
    const char * getControllerActionName(unsigned deviceId,
        unsigned controllerId,unsigned actionId);

    void saveConfig();



    /* singleton class access */
    inline irr::io::IFileSystem *     getFileSystem()  { return m_fileSystem; }
    inline irr::video::IVideoDriver * getVideoDriver() { return m_device->getVideoDriver(); }
    inline PhyWorld *                 getPhyWorld()    { return m_world; }
    EventReceiver *                   getEventReceiver(); // TODO: dont remenber why this is not inlined
    inline lua_State *                getLuaState()    { return m_lua; }
    inline irr::IrrlichtDevice *      getDevice()      { return m_device; }
    inline irr::gui::IGUIEnvironment *getGuiEnv()      { return m_device->getGUIEnvironment(); }
    inline AudioLayer *               getAudioLayer()  { return m_audioLayer; }

    
    /* game status handling */
    inline const std::vector<IVehicle*> & getVehiclesList() { return m_vehicles; }
    inline const std::vector<Track*> &    getTracksList() { return m_tracks; }
    inline void                           setUsedTrack(Track * track) { m_track=track; }

    inline void getScreenHeight(unsigned & height) { height=m_screenHeight; }
    inline void getScreenWidth(unsigned & width) { width=m_screenWidth; }

    /* menu handling */
    void hideMenu();
    void showMenu(const std::wstring & name,bool centerOnTheScreen=false);
    void showMenu(const std::string & name,bool centerOnTheScreen=false);

    void startRace(unsigned humanVehicles, unsigned totVehicles);
    inline void endRace()  { m_mustEndRace=true; }
    inline void resumeRace() { m_mustResumeRace=true; }

    /* controllers configuration */
    void startControllerLearning(unsigned deviceId, 
                unsigned controllerId,unsigned action,const char * callbackCode);
    void stopControllerLearning();
    void addControllerToDevice(unsigned deviceId);


    void setSplitScreenModality(int l);

    /* lua stuff */
    void lua_doFile(const char * filename);
    void lua_doString(const char * script);

  private:
    static ResourceManager * s_instance;
    ResourceManager();

    std::vector<IDeviceInterface *> m_inputDevices;

    void loadConfig(const std::string & filename);
    void saveConfig(const std::string & filename);

    void loadVehicles();
    void loadTracks();

    irr::IrrlichtDevice *  m_device;
    irr::io::IFileSystem * m_fileSystem;
    PhyWorld *             m_world;// = PhyWorld::buildMe();

    std::string m_rootDir;
    std::string m_trackDir;
    std::string m_vehicleDir;
    std::string m_texturesDir;
    std::string m_audioDir;

    XmlNode *   m_configRoot;

    GuiMenu *   m_menu;

    lua_State * m_lua;

    unsigned m_screenHeight;
    unsigned m_screenWidth;

    // resources
    irr::gui::IGUIFont *  m_font;
    irr::gui::IGUIFont *  m_fontSmall;
    irr::gui::IGUIFont *  m_fontBig;


    enum {
      pa_race=0,
      pa_vehicleChooser=1,
      pa_trackChooser=2,
      pa_empty=3,

      pa_numPhaseHandlers
    };

    IPhaseHandler *  m_phaseHandlers[pa_numPhaseHandlers];
    IPhaseHandler *  m_currentPhaseHandler;

    //////////////////////
    // game description //
    //////////////////////
    unsigned   m_humanVehicles;
    unsigned   m_totVehicles;
    enum       { m_max_vehicles=4 };
    unsigned   m_choosenVehicles[m_max_vehicles];
    Track *    m_track; 

    std::vector<IVehicle*> m_vehicles;
    std::vector<Track*>    m_tracks;


    IVehicleController * m_controllerLearning;
    std::string          m_genericCallbackCode;

    AudioLayer *         m_audioLayer;
    

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   phase handlers status 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    bool       m_mustStartRace;
    bool       m_mustEndRace;
    bool       m_mustResumeRace;
};

#endif

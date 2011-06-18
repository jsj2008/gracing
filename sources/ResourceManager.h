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
#include <stdlib.h>
#include <irrlicht.h>

#include "EventReceiver.h"

class XmlNode;

class ResourceManager 
{
  public:

    static inline ResourceManager * getInstance() { if(s_instance==0) s_instance=new ResourceManager(); return s_instance; }

    irr::io::path createAbsoluteFilename(const std::string & fileName);

    void setDevice(irr::IrrlichtDevice *device);

    void getVehicleCompletePath(const char * vehicleName, std::string & path);
    void getTrackCompletePath(const char * trackName, std::string & path);
    void getConfigCompletePath(const char * filename, std::string & path);
    void getResourceCompletePath(const char * filename, std::string & path);

    EventReceiver * getEventReceiver();

    /* system font */
    inline irr::gui::IGUIFont *  getSystemFontSmall() { return m_fontSmall; }
    inline irr::gui::IGUIFont *  getSystemFont() { return m_font; }

    /* config access */
    bool cfgGet(const char * name, bool & value);
    bool cfgGet(const char * name, double & value);
    bool cfgGet(const char * name, double value[3]);
    bool cfgGet(const char * name, unsigned & value);
    bool cfgGet(const char * name, std::string & value);

    inline const std::string & getResourcePath() { return m_rootDir; }


    inline irr::io::IXMLReaderUTF8 * createXMLReaderUTF8(const std::string & filename)
    {
      return m_fileSystem->createXMLReaderUTF8(filename.c_str());
    }

    inline irr::io::IFileSystem * getFileSystem() { return m_fileSystem; }
    inline irr::video::IVideoDriver * getVideoDriver() { return m_device->getVideoDriver(); }

    inline void getScreenHeight(unsigned & height) { height=m_screenHeight; }
    inline void getScreenWidth(unsigned & width) { width=m_screenWidth; }

  private:
    static ResourceManager * s_instance;
    ResourceManager();

    void loadConfig(const std::string & filename);


    irr::IrrlichtDevice * m_device;
    irr::io::IFileSystem * m_fileSystem;

    std::string m_rootDir;
    std::string m_trackDir;
    std::string m_vehicleDir;

    XmlNode *   m_configRoot;

    unsigned m_screenHeight;
    unsigned m_screenWidth;

    // resources
    irr::gui::IGUIFont *  m_font;
    irr::gui::IGUIFont *  m_fontSmall;
};

#endif

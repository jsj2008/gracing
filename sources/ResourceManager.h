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
#include <stdlib.h>
#include <irrlicht.h>

#ifndef VEHICLES_DIR
#error Please define VEHICLES_DIR preprocessor macro
#endif

#ifndef TRACKS_DIR
#error Please define TRACKS_DIR preprocessor macro
#endif

class ResourceManager 
{
  public:

    static inline ResourceManager * getInstance() { if(s_instance==0) s_instance=new ResourceManager(); return s_instance; }

    irr::io::path createAbsoluteFilename(const std::string & fileName);

    void setDevice(irr::IrrlichtDevice *device);

    inline void getVehiclesCompletePath(char * buffer, int buffer_len)
    {
      const char * ddir=VEHICLES_DIR;
      int i;
      --buffer_len;
      for(i=0; i<buffer_len && *ddir; ++i, ++ddir)
        buffer[i]=*ddir;
    }

    void getTrackCompletePath(const char * trackName, std::string & path);

    inline const std::string & getResourcePath() { return m_rootDir; }

    static inline void getVehicleCompletePath(const char * vehicleName, char * buffer, int buffer_len)
    {
#ifdef __APPLE__
      //CFUrlRef url=CFBundleCopyResourceURL(bundle, CFSTR(vehicleName), CFSTR("zip"), CFSTR("Resources"));
      const char * ddir=VEHICLES_DIR;
      int i;
      --buffer_len;
      for(i=0; i<buffer_len && *ddir; ++i, ++ddir)
        buffer[i]=*ddir;
      for( ; i<buffer_len && *vehicleName; ++i, ++vehicleName)
        buffer[i]=*vehicleName;
      buffer[i]=0;
#else
      int i;
      for(i=0; i<buffer_len && *vehicleName; ++i)
        buffer[i]=vehicleName[i];
#endif
    }

  private:
    static ResourceManager * s_instance;
    ResourceManager();


    irr::IrrlichtDevice * m_device;
    irr::io::IFileSystem * m_fileSystem;

    std::string m_rootDir;
    std::string m_trackDir;

};


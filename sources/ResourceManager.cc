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

#include "ResourceManager.h"
#include "XmlNode.h"
#include "config.h"
#include "util.hh"
#include "gmlog.h"


#ifdef __APPLE__
#  include <CoreFoundation/CoreFoundation.h>


CFG_PARAM_UINT(glob_screenWidth)=1024;
CFG_PARAM_UINT(glob_screenHeight)=768;

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



  std::string configFilename;
  getConfigCompletePath("config.xml",configFilename);
  loadConfig(configFilename);

  ///////////////////////////////////////
  m_screenWidth=glob_screenWidth;
  m_screenHeight=glob_screenHeight;
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

void ResourceManager::getConfigCompletePath(const char * filename, std::string & path)
{
  path=m_rootDir + std::string(filename);
}

void ResourceManager::setDevice(irr::IrrlichtDevice *device)
{
  if(m_device)
    m_device->drop();

  m_device=device;
  m_fileSystem=m_device->getFileSystem();

  m_trackDir = m_rootDir + std::string("/Tracks/");
  m_vehicleDir = m_rootDir + std::string("/Vehicles/");
}

bool ResourceManager::cfgGet(const char * name, bool & value)
{
  GM_LOG("Getting var name '%s'\n",name);
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
    

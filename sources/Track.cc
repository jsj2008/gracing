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
#include <assert.h>

#include "gmlog.h"
#include "gException.h"

#include "Track.hh"
#include "Util.hh"
#include "PhyWorld.h"
#include "ResourceManager.h"

#ifndef BASE_DIR
#define BASE_DIR "."
#endif
#define MANIFEST_NAME "TRACK"


Track::Track(
    irr::IrrlichtDevice * device,
    PhyWorld * world,
    const char * filename
    )
{
  m_device=device;
  m_filesystem=device->getFileSystem();
  enum {
    buffer_len=1024
  };
  //char buffer[buffer_len];
  ResourceManager::getInstance()->getTrackCompletePath(filename, m_filename);
  //m_filename=strdup(buffer);
  m_loaded=false;
  m_world=world;
  m_filesystem->grab();
  //m_device->grab();
  m_cammgr=0;
}

void Track::load()
{
  if(m_loaded) return;

  irr::u32 cnt=m_filesystem->getFileArchiveCount();

  irr::io::path mypath(m_filename.c_str());
  
  bool res=m_filesystem->addFileArchive(mypath);

  GM_LOG(" %d sukinae %d\n",__LINE__,cnt);

  if(!res) {
    GM_LOG("cannot load: '%s'\n",mypath.c_str());
    return ;
  }

  GM_LOG("laoding index '%u' from '%s'\n",cnt,m_filename.c_str());

  m_archiveIndex=cnt;


  irr::io::IFileArchive* archive=m_filesystem->
    getFileArchive(m_archiveIndex);
  if(!archive) {
    throw Exception_cannotLoadFile(m_filename.c_str());
  }


  const irr::io::IFileList * fileList=archive->getFileList();
  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);

  //GM_LOG("uno %d\n",manifestIndex);
  
  if(manifestIndex<0) {
    GM_LOG("Not a valid track file\n");
    throw Exception_cannotLoadFile(m_filename.c_str());
  }

  irr::io::IReadFile *  manifestFile=
    archive->createAndOpenFile(manifestIndex);
  
  if(!manifestFile) {
    throw Exception_cannotLoadFile(m_filename.c_str());
  }

  assert(manifestFile);

  irr::io::IXMLReaderUTF8 * xmlReader=
    m_filesystem->createXMLReaderUTF8 (manifestFile);

  enum { MAX_DEPTH=128 };

  int ot;
  int nodeStack[MAX_DEPTH];

  irr::scene::ISceneManager* smgr = m_device->getSceneManager();

  irr::io::EXML_NODE nodeType;
  enum {
    ot_none,
    ot_mesh,
    ot_camera,
    ot_lamp,
    ot_start_pos,
    ot_start_rot,
    ot_unknown
  };
  irr::io::IReadFile * rfile;
  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_NONE:
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("mesh",xmlReader->getNodeName())==0) {
          ot=ot_mesh;
        } else if(strcmp("camera",xmlReader->getNodeName())==0) {
          ot=ot_camera;
        } else if(strcmp("lamp",xmlReader->getNodeName())==0) {
          ot=ot_lamp;
        } else if(strcmp("track_start_pos",xmlReader->getNodeName())==0) {
          ot=ot_start_pos;
        } else {
          ot=ot_unknown;
        }
        nodeStack[++nodeStackPtr]=ot;
        inElement=true;
      break;
      case irr::io::EXN_ELEMENT_END:
        inElement=false;
        nodeStackPtr --;
      break;
      case irr::io::EXN_TEXT:
        if(inElement) {
          switch(nodeStack[nodeStackPtr]) {
            case ot_start_pos:
              Util::parseVector(xmlReader->getNodeName(), m_startPosition);
              break;

            case ot_lamp:
              GM_LOG("Loading lights: %s\n",xmlReader->getNodeName());
              rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                loadLights(rfile,smgr);
                rfile->drop();
              } else {
                GM_LOG("  --> cannot find file\n");
              }
              break;
            case ot_mesh:
              GM_LOG("Loading mesh: %s\n",xmlReader->getNodeName());
              rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                irr::scene::IAnimatedMesh* mesh = smgr->getMesh(rfile);
                irr::scene::IAnimatedMeshSceneNode* node=0;
                node=smgr->addAnimatedMeshSceneNode( mesh );
                if(!node) {
                  GM_LOG("cannot load mesh\n");
                } else {
                  m_sceneNodes.push_back(node);
                  node->grab();
                }
                rfile->drop();
              } else {
                GM_LOG("  cannot find file\n");
              }
              break;
            case ot_camera:
              GM_LOG("Loading camera data from: '%s'\n",xmlReader->getNodeName());
              irr::io::IReadFile * rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                if(m_cammgr)
                  delete m_cammgr;
                m_cammgr=new CameraDataManager(rfile);
                irr::scene::ICameraSceneNode * camera=smgr->addCameraSceneNodeFPS();
                irr::core::vector3df p,r;
                m_cammgr->getPositionAndRotation(p,r);
                camera->setPosition(p);
                camera->setRotation(r);
                rfile->drop();
                
              } else {
                GM_LOG("Cannot load camera data, using default camera\n");
                smgr->addCameraSceneNodeFPS();
              }
            break;
          }
        }
      break;
      case irr::io::EXN_COMMENT:
        GM_LOG("%d Comment '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_CDATA:
        GM_LOG("%d Cdata '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_UNKNOWN:
        GM_LOG("%d Unknown '%s'\n",i,
            xmlReader->getNodeName());
      break;
      default:
      GM_LOG("%d '%s', type: %d\n",i,
          xmlReader->getNodeName(),
          xmlReader->getNodeType());
      break;
    }
  }
  //xmlReader->drop();
  manifestFile->drop();
  archive->drop();
  m_filesystem->removeFileArchive(m_archiveIndex);
	for (irr::u32 i=0; i < m_sceneNodes.size(); ++i ) {
    m_world->addStaticMesh(m_sceneNodes[i]);
  }
  m_loaded=true;
}

void Track::unload()
{
  if(!m_loaded) return;

	for (irr::u32 i=0; i < m_lights.size(); ++i ) {
    m_lights[i]->remove();
    m_lights[i]->drop();
  }
  m_lights.erase(0,m_lights.size());

	for (irr::u32 i=0; i < m_sceneNodes.size(); ++i ) {
    m_sceneNodes[i]->remove();
    m_sceneNodes[i]->drop();
  }
  m_sceneNodes.erase(0,m_sceneNodes.size());

  //if(m_cammgr)
  //  delete m_cammgr;

  m_loaded=false;
}

Track::~Track()
{
  GM_LOG("Track destruct\n");
  unload();
  //m_filesystem->drop();
  //m_device->drop();
  GM_LOG("Track destruct done\n");
}


void Track::loadLights( irr::io::IReadFile * file ,
  irr::scene::ISceneManager* smgr )
{
  irr::u16 mark;
  int cnt=0;
  double pos[3],dif[3],spe[3],radius;

  smgr->setAmbientLight(
      irr::video::SColorf(.6,.6,.6));

  mark=Util::readMark(file);

  Util::readTriple(file,pos);
  Util::readTriple(file,dif);
  Util::readTriple(file,spe);
  radius=Util::readDouble(file);

  cnt++;

  GM_LOG(" - %02d light\n   - position %f,%f,%f\n   - diffuse %f,%f,%f\n   - specular: %f,%f,%f\n   - radius: %f\n",
      cnt,
      pos[0], pos[1], pos[2],
      dif[0], dif[1], dif[2],
      spe[0], spe[1], spe[2],radius);

  irr::core::vector3df position = irr::core::vector3df(pos[0], pos[1], pos[2]);
  irr::video::SColorf specularColor = 
    irr::video::SColorf(spe[0],spe[1],spe[2]);
  irr::video::SColorf diffuseColor = 
    irr::video::SColorf(dif[0],dif[1],dif[2]);


  irr::scene::ILightSceneNode* light = 
    smgr->addLightSceneNode( 0, position, diffuseColor, radius);

  //light->setLightType(irr::video::ELT_DIRECTIONAL);
  //light->setRotation( irr::core::vector3df(180, 45, 45) );
  light->getLightData().SpecularColor = specularColor;
  light->grab();

}

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
#include "Race.h"

#define MANIFEST_NAME "TRACK"

/* trigger type */
enum
{
  tt_lap=33,

  tt_maxNumber=128
};

bool cb_ContactAddedCallback(
    btManifoldPoint& cp,
    const btCollisionObject* colObj0,
    int partId0,
    int index0,
    const btCollisionObject* colObj1,
    int partId1,
    int index1)
{
  Track::TriggerInfo * triggerInfo=0;

  if(colObj0->getUserPointer() == (void*)tt_lap && colObj1->getUserPointer()!=0) 
    triggerInfo=(Track::TriggerInfo*)colObj1->getUserPointer();
  else if(colObj1->getUserPointer() == (void*)tt_lap && colObj0->getUserPointer()!=0) 
    triggerInfo=(Track::TriggerInfo*)colObj0->getUserPointer();


  if(!triggerInfo) 
    return false;

  switch(triggerInfo->type) 
  {
    case tt_lap:
      if(triggerInfo->race)
        triggerInfo->race->lapTriggered(triggerInfo->userData);
      break;
    default:
      break;
  }

  return false;
}


Track::Track(
    irr::IrrlichtDevice * device,
    PhyWorld * world,
    const char * filename)
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

  m_rootNode=loadXml(m_filename.c_str());

  gContactAddedCallback=cb_ContactAddedCallback;
}

XmlNode * Track::loadXml(const char * filename)
{
  XmlNode * node=0;

  irr::io::path mypath(filename);
  bool res=m_filesystem->addFileArchive(mypath);

  if(!res) 
    return 0;

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  if(!xml)
    return 0;

  node=new XmlNode(xml);
  assert(node && node->getName() == "track"); // TODO: this must not be an assert!!!

  xml->drop();

  res=m_filesystem->removeFileArchive(m_filesystem->getAbsolutePath(mypath));
  assert(res);

  return node;
}

void Track::loadTriggers(XmlNode * root)
{
  std::vector<XmlNode*> nodes;
  XmlNode * node;
  std::string type;
  btVector3 dim;
  btVector3 pos;
  btQuaternion rot;

  root->getChildren(nodes);

  for(unsigned i=0; i<nodes.size(); i++) {
    node=nodes[i];
    if(node->getName() != "trigger")
      continue;
    node->get("type",type);
    node->get("halfDim",dim);
    node->get("pos",pos);
    node->get("rot",rot);
    GM_LOG("Trigger: pos: %f,%f,%f\n",pos.getX(),pos.getY(),pos.getZ());
    GM_LOG("         dim: %f,%f,%f\n",dim.getX(),dim.getY(),dim.getZ());
    if(type=="lap") {
      btCollisionShape* shape = new btBoxShape(dim);

      btTransform startTransform;
      startTransform.setIdentity();

      btScalar	btMass(0.);

      startTransform.setOrigin(pos);
      //startTransform.setRotation(rot);

      btVector3 localInertia(0,0,0);
      btDefaultMotionState* motionState = 
        new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo 
        rbInfo(btMass,motionState,shape,localInertia);
      btRigidBody* body = new btRigidBody(rbInfo);

      body->setCollisionFlags(body->getCollisionFlags() |
          btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK |
          btCollisionObject::CF_NO_CONTACT_RESPONSE);

      m_world->addRigidBody(body);

      body->setUserPointer((void*)tt_lap);
    }
  }
}

void Track::loadControlPoints(XmlNode * root)
{
  std::vector<XmlNode*> nodes;
  XmlNode * node;
  double v;
  btVector3 point;

  root->getChildren(nodes);

  for(unsigned i=0; i<nodes.size(); i++) {
    node=nodes[i];
    if(node->getName() != "point")
      continue;
    node->get("X",v); point.setX(v);
    node->get("Y",v); point.setY(v);
    node->get("Z",v); point.setZ(v);

    m_controlPoints.push_back(point);
  }
}

void Track::load()
{ 
  std::vector<XmlNode*> nodes;
  XmlNode * node;

  irr::io::path archivepath(m_filename.c_str());
  bool res=m_filesystem->addFileArchive(archivepath);
  assert(res);

  m_rootNode->getChildren(nodes);
  irr::scene::ISceneManager* smgr = m_device->getSceneManager();
  
  m_skydome=0;

  for(unsigned int i=0; i<nodes.size(); i++) {
    node=nodes[i];
    if(node->getName() == "control-points") {
      loadControlPoints(node);
    } else if(node->getName() == "triggers") {
      loadTriggers(node);
    } else if(node->getName() == "track_start_pos") {
      Util::parseVector(node->getText().c_str(), m_startPosition);
      GM_LOG("control points: %f,%f,%f,\n",
          m_startPosition.X,
          m_startPosition.Y,
          m_startPosition.Z);
    } else if(node->getName() == "track_start_rot") {
      m_startRotation=Util::parseFloat(node->getText().c_str());
    } else if(node->getName() == "skydome") {
      std::string textureName;
      std::string type;
      node->get("type",type);
      if(type == "box") {
        std::vector<XmlNode*> ts;
        irr::video::ITexture  * skydomeTexture[6];
        node->getChildren(ts);
        for(unsigned i=0; i<6; i++) skydomeTexture[i]=0;
        for(unsigned i=0; i<ts.size(); i++) {
          unsigned idx;
          XmlNode * t=ts[i];
          t->get("idx",idx);
          t->get("src",textureName);
          idx--;
          if(idx >= 0 && idx < 6) {
            // TODO: check that at index i there is no texture yet
            std::string path;
            ResourceManager::getInstance()->getTexturesCompletePath(textureName.c_str(),path);
            skydomeTexture[idx] = smgr->getVideoDriver()->getTexture(path.c_str());
            if(!skydomeTexture[idx]) {
              GM_LOG("Cannot load idx: %d, texture name: %s, path: %s\n",idx,textureName.c_str(),path.c_str());
            }
          }
          m_skydome = smgr->addSkyBoxSceneNode(
              skydomeTexture[0],
              skydomeTexture[1],
              skydomeTexture[2],
              skydomeTexture[3],
              skydomeTexture[4],
              skydomeTexture[5]);
        }
      } else if(type == "sphere") {
        std::vector<XmlNode*> ts;
        irr::video::ITexture  * skydomeTexture;
        node->getChildren(ts);
        XmlNode * t=ts[0];
        t->get("src",textureName);
        std::string path;
        ResourceManager::getInstance()->getTexturesCompletePath(textureName.c_str(),path);
        skydomeTexture = smgr->getVideoDriver()->getTexture(path.c_str());
        if(!skydomeTexture) {
          GM_LOG("Cannot load texture name: %s, path: %s\n",textureName.c_str(),path.c_str());
        } else {
          m_skydome = smgr->addSkyDomeSceneNode(skydomeTexture);
        }
      } else {
        GM_LOG("Uknown type of skydome '%s'\n",type.c_str());
      }

    } else if(node->getName() == "mesh") {
      irr::io::IReadFile * rfile=m_filesystem->
        createAndOpenFile (node->getText().c_str());
      if(rfile) {
        irr::scene::IAnimatedMesh* mesh = smgr->getMesh(rfile);
        irr::scene::IMeshSceneNode* inode=0;
        inode=smgr->addOctreeSceneNode( mesh,0,0xBADD );
        if(!inode) {
          GM_LOG("cannot load mesh '%s'\n",node->getText().c_str());
        } else {
          m_sceneNodes.push_back(inode);
          inode->grab();
        }
        rfile->drop();
      } else {
        GM_LOG("  cannot find file\n");
      }
    } else if(node->getName() == "camera") {
      // not handled 
#if 0
      irr::io::IReadFile * rfile=m_filesystem->
        createAndOpenFile (node->getText().c_str());
      if(rfile) {
        if(m_cammgr)
          delete m_cammgr;
        if(m_camera) {
          m_camera->remove();
          m_camera=0;
        }
        m_cammgr=new CameraDataManager(rfile);
        irr::core::vector3df p,r;
        rfile->drop();
      }
#endif
    } else if(node->getName() == "lamp") {
      irr::io::IReadFile * rfile=m_filesystem->
        createAndOpenFile (node->getText().c_str());
      if(rfile) {
        loadLights(rfile,smgr);
        rfile->drop();
      } else {
        GM_LOG("  --> cannot find file\n");
      }
    }
  }

  const XmlNode * triggers=m_rootNode->getChild("triggers");

  assert(triggers); // TODO: this should not be an assert

  nodes.clear();
  triggers->getChildren(nodes);
  // ?????????????????
  for(unsigned int i=0; i<nodes.size(); i++) {
    btVector3 pos;
    btVector3 dim;
    btQuaternion rot;
    node=nodes[i];
  }

  res=m_filesystem->removeFileArchive(m_filesystem->getAbsolutePath(archivepath));
  assert(res);

  // add track meshes to the physic world
  for (irr::u32 i=0; i < m_sceneNodes.size(); ++i ) {
    m_world->addStaticMesh(m_sceneNodes[i]);
  }

  // skydome
  if(m_skydome == 0) {
    irr::video::ITexture  * skydomeTexture;
    std::string textureName;

    ResourceManager::getInstance()->getTexturesCompletePath("clouds.png", textureName);
    skydomeTexture = smgr->getVideoDriver()->getTexture(textureName.c_str());
    if(skydomeTexture)
      m_skydome = smgr->addSkyDomeSceneNode(skydomeTexture);
  }

  //  handle lighting 
  irr::video::SColor   ambient_color = irr::video::SColor(255, 120, 120, 120);
  irr::video::SColor   sun_specular_color = irr::video::SColor(255, 255, 255, 255);
  irr::video::SColor   sun_diffuse_color = irr::video::SColor(255, 255, 255, 255); 
  irr::core::vector3df sun_position  = irr::core::vector3df(0, 0, 0);

  smgr->setAmbientLight(ambient_color);

  const XmlNode * sun = m_rootNode->getChild("sun");

  if(sun) {
    // .....
  }

  m_sun = smgr->addLightSceneNode(NULL, 
      sun_position,
      sun_diffuse_color);
  m_sun->setLightType(irr::video::ELT_DIRECTIONAL);
  m_sun->setRotation( irr::core::vector3df(180, 45, 45) );
  m_sun->getLightData().SpecularColor = sun_specular_color;

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
  //m_sceneNodes.erase(0,m_sceneNodes.size());
  m_sceneNodes.clear();

  if(m_sun) {
    m_sun->remove();
    m_sun=0;
  }

  if(m_skydome) {
    m_skydome->remove();
    m_skydome=0;
  }

  m_loaded=false;
}

Track::~Track()
{
  unload();
}


void Track::loadLights( irr::io::IReadFile * file ,
  irr::scene::ISceneManager* smgr )
{
  irr::u16 mark;
  double pos[3],dif[3],spe[3],radius;
  assert(0);


  mark=Util::readMark(file);

  Util::readTriple(file,pos);
  Util::readTriple(file,dif);
  Util::readTriple(file,spe);
  radius=Util::readDouble(file);


  irr::core::vector3df position = irr::core::vector3df(pos[0], pos[1], pos[2]);
  irr::video::SColorf specularColor = 
    irr::video::SColorf(spe[0],spe[1],spe[2]);
  irr::video::SColorf diffuseColor = 
    irr::video::SColorf(dif[0],dif[1],dif[2]);


  irr::scene::ILightSceneNode* light = 
    smgr->addLightSceneNode( 0, position, diffuseColor, radius,0xBADD);

  light->setLightType(irr::video::ELT_DIRECTIONAL);
  light->setRotation( irr::core::vector3df(180, 45, 45) );
  m_lights.push_back(light);
  light->getLightData().SpecularColor = specularColor;
  light->grab();
}

void Track::registerLapCallback(Race * race, IVehicle * vehicle, void * userdata)
{
  // TODO: must use a vector of TriggerInfo
  //       (to support multiple registration)

  TriggerInfo * triggerInfo=new TriggerInfo;
  triggerInfo->type=tt_lap;
  triggerInfo->race=race;
  triggerInfo->self=vehicle->getRigidBody();
  vehicle->getRigidBody()->setUserPointer(triggerInfo);
  triggerInfo->userData=userdata;

  m_triggersLap.push_back(triggerInfo);
}

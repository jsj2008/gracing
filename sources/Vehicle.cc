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

#include <string.h>
#include <assert.h>

#include "Vehicle.h"
#include "gmlog.h"

#define NOT_A_VALID_VEHICLE_IF_NOT(cond) assert(cond)
#define NOT_A_VALID_VEHICLE_IF(cond) assert(!(cond))

#define WARNING_IF(cond,fmt,...) do {\
  if(cond) {\
    GM_LOG(fmt, ## __VA_ARGS__);\
  }\
} while(0)

#define WARNING(fmt,...) do {\
  GM_LOG(fmt,## __VA_ARGS__);\
  GM_LOG("\n");\
} while(0)
  


#define MANIFEST_NAME "VEHICLE"

CFG_PARAM_D(glob_wheelsDefaultMass)=1.;
CFG_PARAM_D(glob_chassisDefaultMass)=1.;

CFG_PARAM_D(glob_gVehicleSteering)=0.f;
CFG_PARAM_D(glob_steeringIncrement)=0.04f;
CFG_PARAM_D(glob_steeringClamp)=0.3f;
CFG_PARAM_D(glob_wheelRadius)=0.5f;
CFG_PARAM_D(glob_wheelWidth)=0.4f;
CFG_PARAM_D(glob_wheelFriction)=1000;//BT_LARGE_FLOAT;
CFG_PARAM_D(glob_suspensionStiffness)=20.f;
CFG_PARAM_D(glob_suspensionDamping)=2.3f;
CFG_PARAM_D(glob_suspensionCompression)=4.4f;
CFG_PARAM_D(glob_rollInfluence)=0.1f;//1.0f;


/////////////////////////////////////////////////////////////////////////

Vehicle::Vehicle(
    irr::scene::ISceneNode * parent,
    irr::IrrlichtDevice * device, 
    PhyWorld * world,
    const char * source,
    irr::s32 id)
  : IVehicle(parent, device->getSceneManager(),id)
{
  m_device=device;
  m_world=world;
  m_filesystem=device->getFileSystem();
  m_sourceName=strdup(source);
  m_loaded=false;

  m_gVehicleSteering = glob_gVehicleSteering;
  m_steeringIncrement = glob_steeringIncrement;
  m_steeringClamp = glob_steeringClamp;
  m_wheelRadius = glob_wheelRadius;
  m_wheelWidth = glob_wheelWidth;
  m_wheelFriction = glob_wheelFriction;
  m_suspensionStiffness = glob_suspensionStiffness;
  m_suspensionDamping = glob_suspensionDamping;
  m_suspensionCompression = glob_suspensionCompression;
  m_rollInfluence = glob_rollInfluence;

  for(int i=0; i<W_NUM_OF_WHEELS; i++) 
    m_wheels[i]=0;

  m_compoundShape=0;
  m_chassisShape=0;
	m_vehicleRayCaster=0;
	m_raycastVehicle=0;

  m_using=0;

}

void Vehicle::initPhysics()
{
  if(!m_loaded) {
    WARNING("cannot init physics, coz vehicle still not loaded");
    return;
  }

	//btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f,0.5f,2.f));
}

void Vehicle::initGraphics()
{
  if(!m_loaded) {
    WARNING("cannot init graphics, coz vehicle still not loaded");
    return;
  }

  if(m_using & USE_GRAPHICS) {
    WARNING("alread have graphics in plaze");
    return;
  }

  irr::u32 n;
  irr::u32 i;
  irr::scene::ISceneNode * node;
  irr::scene::ISceneManager *     smgr=
    m_device->getSceneManager();

  GM_LOG("creating vehicle scene nodes:\n");

  n=m_chassis.size();
  GM_LOG("- chassis %d nodes\n",n);
  // actually build nodes
  for(i=0; i<n; ++i) {
    node=smgr->addAnimatedMeshSceneNode(m_chassis[i],this);
  }

  GM_LOG("----->%d\n",m_irrNodes.size());

  m_using|=USE_GRAPHICS;
}

void Vehicle::deinitGraphics()
{
  if(!(m_using & USE_GRAPHICS)) {
    WARNING("cannot init graphics, already deinited\n");
    return;
  }
  irr::u32 i,n;
  irr::scene::ISceneNode * node;
  n=m_irrNodes.size();
  for(i=0; i<n; ++i) {
    GM_LOG("fuke %d\n",i);
    node=m_irrNodes[i];
    GM_LOG("-->%p\n",node);
    node->remove();
    GM_LOG("2fuke %d\n",i);
  }
  GM_LOG("suka\n");
  m_irrNodes.erase(0,n);
  m_using &= ~USE_GRAPHICS;
}

void Vehicle::load()
{
  if(m_loaded)
    return;
  irr::u32 cnt=m_filesystem->getFileArchiveCount();

  bool res=m_filesystem->addFileArchive(m_sourceName);

  NOT_A_VALID_VEHICLE_IF(!res);

  irr::u32 archiveIndex=cnt;

  irr::io::IFileArchive* archive=m_filesystem->
    getFileArchive(archiveIndex);

  NOT_A_VALID_VEHICLE_IF_NOT(archive);

  const irr::io::IFileList * fileList=archive->getFileList();
  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);

  NOT_A_VALID_VEHICLE_IF(manifestIndex<0);

  irr::io::IReadFile *  manifestFile=
    archive->createAndOpenFile(manifestIndex);

  assert(manifestFile);

  irr::io::IXMLReaderUTF8 * xmlReader=
    m_filesystem->createXMLReaderUTF8 (manifestFile);

  enum { MAX_DEPTH=128 };

  int ot;
  int nodeStack[MAX_DEPTH];
  enum {
    ot_none,
    ot_chassis,
    ot_wfr,
    ot_wfl,
    ot_wrr,
    ot_wrl
  };

  irr::io::EXML_NODE nodeType;
  irr::scene::IAnimatedMesh* mesh;
  irr::scene::ISceneManager * smgr=m_device->getSceneManager();

  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_UNKNOWN:
      case irr::io::EXN_CDATA:
      case irr::io::EXN_COMMENT:
      case irr::io::EXN_NONE:
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("chassis",xmlReader->getNodeName())==0) {
          ot=ot_chassis;
        } else if(strcmp("wfr",xmlReader->getNodeName())==0) {
          ot=ot_wfr;
        } else if(strcmp("wfl",xmlReader->getNodeName())==0) {
          ot=ot_wfl;
        } else if(strcmp("wrr",xmlReader->getNodeName())==0) {
          ot=ot_wrr;
        } else if(strcmp("wrl",xmlReader->getNodeName())==0) {
          ot=ot_wrl;
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
            case ot_chassis:
              GM_LOG("loading chassis part from '%s'\n",
                  xmlReader->getNodeName());
              mesh=smgr->getMesh(xmlReader->getNodeName());
              WARNING_IF(mesh==0," - cannot load file '%s'\n",
                  xmlReader->getNodeName());
              if(mesh) {
                mesh->grab();
                m_chassis.push_back(mesh);
              }
              break;

            case ot_wfl:
              GM_LOG("loading front left wheel part from '%s'\n",
                  xmlReader->getNodeName());
              if(m_wheels[W_FRONT_LEFT]) {
                 WARNING("Double definizion of wheel: front left");
                 break;
              } 
              mesh=smgr->getMesh(xmlReader->getNodeName());
              WARNING_IF(mesh==0," - cannot load file '%s'\n",
                  xmlReader->getNodeName());
              if(mesh) {
                m_wheels[W_FRONT_LEFT]=mesh;
                mesh->grab();
              }
              break;

            case ot_wfr:
              GM_LOG("loading front right wheel part from '%s'\n",
                  xmlReader->getNodeName());
              mesh=smgr->getMesh(xmlReader->getNodeName());
              WARNING_IF(mesh==0," - cannot load file '%s'\n",
                  xmlReader->getNodeName());
              if(m_wheels[W_FRONT_RIGHT]) {
                WARNING("Double definition of wheel: front right");
                break;
              }
              if(mesh) {
                m_wheels[W_FRONT_RIGHT]=mesh;
                mesh->grab();
              }
              break;

            case ot_wrl:
              GM_LOG("loading rear left wheel part from '%s'\n",
                  xmlReader->getNodeName());
              mesh=smgr->getMesh(xmlReader->getNodeName());
              WARNING_IF(mesh==0," - cannot load file '%s'\n",
                  xmlReader->getNodeName());
              if(m_wheels[W_REAR_LEFT]) {
                WARNING("Double definition of wheel: rear left");
                break;
              }
              if(mesh) {
                m_wheels[W_REAR_LEFT]=mesh;
                mesh->grab();
              }
              break;

            case ot_wrr:
              GM_LOG("loading rear right wheel part from '%s'\n",
                  xmlReader->getNodeName());
              mesh=smgr->getMesh(xmlReader->getNodeName());
              WARNING_IF(mesh==0," - cannot load file '%s'\n",
                  xmlReader->getNodeName());
              if(m_wheels[W_REAR_RIGHT]) {
                WARNING("Double definition of wheel: rear right");
                break;
              }
              if(mesh) {
                m_wheels[W_REAR_RIGHT]=mesh;
                mesh->grab();
              }
              break;
          }
        }
    }
  }
  // TODO: check presence of all parts
  xmlReader->drop();
  m_loaded=true;
}


void Vehicle::unload()
{
}

void Vehicle::use(unsigned int useFlags)
{
  if(useFlags & USE_GRAPHICS) 
    initGraphics();
}

void Vehicle::unuse(unsigned int useFlags)
{
  if(useFlags & USE_GRAPHICS)
    deinitGraphics();
}


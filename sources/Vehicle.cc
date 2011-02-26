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
#include "util.hh"
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
  enum {
    ot_none,
    ot_chassis,
    ot_wfr,
    ot_wfl,
    ot_wrr,
    ot_wrl,
    ot_wrl_position,
    ot_wrr_position,
    ot_wfl_position,
    ot_wfr_position,

    ot_wrl_radius,
    ot_wrr_radius,
    ot_wfl_radius,
    ot_wfr_radius,

    ot_wrl_width,
    ot_wrr_width,
    ot_wfl_width,
    ot_wfr_width
  };

  enum {
    ot_first_position=ot_wrl_position,
    ot_first_radius=ot_wrl_radius,
    ot_first_width=ot_wrl_width
  };


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

  for(int i=0; i<W_NUM_OF_WHEELS; i++)  {
    m_wheels[i]=0;
    m_wheelShapes[i]=0;
  }

  m_chassisShape=0;
	m_vehicleRayCaster=0;
	m_raycastVehicle=0;

  m_using=0;

}

Vehicle::~Vehicle()
{
  GM_LOG("vehicle destructor\n");
  this->unload();
}

void Vehicle::initPhysics()
{
  if(!m_loaded) {
    WARNING("cannot init physics, coz vehicle still not loaded");
    return;
  }

  if(!(m_using & USE_GRAPHICS)) {
    WARNING("cannot init phisics, must first init graphics");
    return;
  }

  if(m_using & USE_PHYSICS) {
    return;
  }

  // the chassis shape
  // TODO: use the mesh to create the shape

  m_vehicleShape = new btCompoundShape();

  const irr::core::aabbox3d<float> & bb=getBoundingBox();

  GM_LOG(" - VEHICLE BOUNDING bounding box: min %2.3f,%2.3f,%2.3f, max: %2.3f,%2.3f.%2.3f\n",
      bb.MinEdge.X,bb.MinEdge.Y,bb.MinEdge.Z,
      bb.MaxEdge.X,bb.MaxEdge.Y,bb.MaxEdge.Z);

  m_chassisShape = new btBoxShape(btVector3(0.2f,0.2f,.2f));

  btTransform localTrans;
  localTrans.setIdentity();

  m_vehicleShape->addChildShape(localTrans,m_chassisShape);

  btTransform tr(btTransform::getIdentity());
	tr.setOrigin(btVector3(0,0.f,0));

  m_carBody=m_world->createRigidBody(this, glob_chassisDefaultMass, tr,m_vehicleShape);

  for(int i=0; i<4; i++) {
    m_wheelShapes[i] = 
      new btCylinderShape(btVector3(
            m_wheelRadiuses[i],
            m_wheelWidths[i],
            m_wheelRadiuses[i]));
  }

  m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_world);
	btRaycastVehicle::btVehicleTuning	m_tuning;
  
  m_raycastVehicle = new btRaycastVehicle(m_tuning,m_carBody,m_vehicleRayCaster);

  btVector3 wheelDirection(0,-1,0);
	btVector3 wheelAxleCS(-1,0,0);
  btScalar suspensionRestLength(0.6);
  
  for(int i=0; i<4; i++) {
		btVector3 conn(m_wheelPositions[i].X,
        m_wheelPositions[i].Y,
        m_wheelPositions[i].Z);
		m_raycastVehicle->addWheel(
        conn,
        wheelDirection,
        wheelAxleCS,
        suspensionRestLength,
        m_wheelRadiuses[i],
        m_tuning,
        isFrontWheel(i));
  }

  ///never deactivate the vehicle
  m_carBody->setActivationState(DISABLE_DEACTIVATION);

  m_world->addVehicle(m_raycastVehicle);

#if 0
  float connectionHeight = 1.2f;
  bool isFrontWheel=true;
#endif

  //choose coordinate system
  int rightIndex = 0;
  int upIndex = 1;
  int forwardIndex = 2;
  
  m_raycastVehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

  m_using|=USE_PHYSICS;
}

void Vehicle::initGraphics()
{
  if(!m_loaded) {
    WARNING("cannot init graphics, coz vehicle still not loaded");
    return;
  }

  if(m_using & USE_GRAPHICS) {
    return;
  }

  irr::u32 n;
  irr::u32 i;
  irr::scene::ISceneNode * node;
  irr::scene::ISceneManager * smgr=
    m_device->getSceneManager();

  GM_LOG("creating vehicle scene nodes:\n");

  n=m_chassis.size();
  GM_LOG("- chassis %d nodes\n",n);
  // actually build nodes
  for(i=0; i<n; ++i) {
    node=smgr->addAnimatedMeshSceneNode(m_chassis[i],this,0xcafe);
    m_irrNodes.push_back(node);
  }

  for(i=0; i<4; ++i) {
    node=smgr->addAnimatedMeshSceneNode(m_wheels[i],this,0xcafe);
    GM_LOG("pushing %p,%p\n",node,m_wheels[i]);
    m_irrNodes.push_back(node);
  }
  recalculateBoundingBox();
  
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
    node=m_irrNodes[i];
    GM_LOG("-->%d,%p\n",i,node);
    node->remove();
  }
  GM_LOG("done done\n");
  m_irrNodes.clear();
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
  const char * wheel_names[4]={
    "rear left",
    "rear right",
    "front left",
    "front right"
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
        } else if(strcmp("wfr_width",xmlReader->getNodeName())==0) {
          ot=ot_wfr_width;
        } else if(strcmp("wfl_width",xmlReader->getNodeName())==0) {
          ot=ot_wfl_width;
        } else if(strcmp("wrl_width",xmlReader->getNodeName())==0) {
          ot=ot_wrl_width;
        } else if(strcmp("wrr_width",xmlReader->getNodeName())==0) {
          ot=ot_wrr_width;
        } else if(strcmp("wfr_radius",xmlReader->getNodeName())==0) {
          ot=ot_wfr_radius;
        } else if(strcmp("wfl_radius",xmlReader->getNodeName())==0) {
          ot=ot_wfl_radius;
        } else if(strcmp("wrl_radius",xmlReader->getNodeName())==0) {
          ot=ot_wrl_radius;
        } else if(strcmp("wrr_radius",xmlReader->getNodeName())==0) {
          ot=ot_wrr_radius;
        } else if(strcmp("wfr_position",xmlReader->getNodeName())==0) {
          ot=ot_wfr_position;
        } else if(strcmp("wfl_position",xmlReader->getNodeName())==0) {
          ot=ot_wfl_position;
        } else if(strcmp("wrl_position",xmlReader->getNodeName())==0) {
          ot=ot_wrl_position;
        } else if(strcmp("wrr_position",xmlReader->getNodeName())==0) {
          ot=ot_wrr_position;
        } else if(strcmp("wfr",xmlReader->getNodeName())==0) {
          ot=ot_wfr;
        } else if(strcmp("wfl",xmlReader->getNodeName())==0) {
          ot=ot_wfl;
        } else if(strcmp("wrr",xmlReader->getNodeName())==0) {
          ot=ot_wrr;
        } else if(strcmp("wrl",xmlReader->getNodeName())==0) {
          ot=ot_wrl;
        } else {
          ot=ot_none;
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

            case ot_wrl_position:
            case ot_wrr_position:
            case ot_wfl_position:
            case ot_wfr_position:
              {
                int widx=nodeStack[nodeStackPtr]-ot_first_position;
                assert(widx>-1 && widx<4);
                Util::parseVector(xmlReader->getNodeName(),m_wheelPositions[widx]);
                GM_LOG("Position of '%s' is '%s' -> %2.3f,%2.3f,%2.3f\n",
                    wheel_names[widx],
                    xmlReader->getNodeName(),
                    m_wheelPositions[widx].X,
                    m_wheelPositions[widx].Y,
                    m_wheelPositions[widx].Z);
              }
              break;

            case ot_wrl_width:
            case ot_wrr_width:
            case ot_wfl_width:
            case ot_wfr_width:
              {
                int widx=nodeStack[nodeStackPtr]-ot_first_width;
                assert(widx>-1 && widx<4);
                double width=Util::parseFloat(xmlReader->getNodeName());
                m_wheelWidths[widx]=width;

                GM_LOG("Width of '%s' is '%s'\n",
                    wheel_names[widx],
                    xmlReader->getNodeName());
              }
              break;

            case ot_wrl_radius:
            case ot_wrr_radius:
            case ot_wfl_radius:
            case ot_wfr_radius:
              {
                int widx=nodeStack[nodeStackPtr]-ot_first_radius;
                assert(widx>-1 && widx<4);
                double radius=Util::parseFloat(xmlReader->getNodeName());
                m_wheelRadiuses[widx]=radius;
                GM_LOG("Radius of '%s' is '%s'\n",
                    wheel_names[widx],
                    xmlReader->getNodeName());
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

  manifestFile->drop();

  m_filesystem->removeFileArchive(archiveIndex);

  xmlReader->drop();

  // TODO: check presence of all parts
  m_loaded=true;

}

void Vehicle::reset(const irr::core::vector3d<float>&pos)
{
  //m_world->setBodyPosition(this,pos.X,pos.Y,pos.Z);
  ISceneNode::setPosition(pos);

  // reset position
  btTransform trans=btTransform::getIdentity();
  trans.setOrigin(btVector3(pos.X,pos.Y,pos.Z));
  m_carBody->setCenterOfMassTransform(trans);


  // reset velocity
	m_carBody->setLinearVelocity(btVector3(0,0,0));
	m_carBody->setAngularVelocity(btVector3(0,0,0));

  // reset collosion
  m_world->getBroadphase()->getOverlappingPairCache()->
   cleanProxyFromPairs(m_carBody->getBroadphaseHandle(),m_world->getDispatcher()); 
       //getDynamicsWorld()->getDispatcher()); 
  
}


void Vehicle::unload()
{
}

void Vehicle::use(unsigned int useFlags)
{
  if(useFlags & USE_GRAPHICS) 
    initGraphics();
  if(useFlags & USE_PHYSICS)
    initPhysics();
}

void Vehicle::unuse(unsigned int useFlags)
{
  if(useFlags & USE_GRAPHICS)
    deinitGraphics();
}


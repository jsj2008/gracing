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
    ot_wfr_width,

    ot_wheelFriction,
    ot_suspensionStiffness,
    ot_suspensionDamping,
    ot_suspensionCompression,
    ot_rollInfluence,
    ot_suspensionRestLength
  };

  enum {
    ot_first_position=ot_wrl_position,
    ot_first_radius=ot_wrl_radius,
    ot_first_width=ot_wrl_width
  };


CFG_PARAM_D(glob_chassisDefaultMass)=.001;


//CFG_PARAM_D(glob_gVehicleSteering)=0.f;
CFG_PARAM_D(glob_steeringIncrement)=0.04f;
CFG_PARAM_D(glob_steeringClamp)=0.3f;
CFG_PARAM_D(glob_wheelRadius)=0.5f;
CFG_PARAM_D(glob_wheelWidth)=0.4f;

CFG_PARAM_D(glob_wheelFriction)=1000;//BT_LARGE_FLOAT;
CFG_PARAM_D(glob_suspensionStiffness)=20.f;
CFG_PARAM_D(glob_suspensionDamping)=2.3f;
CFG_PARAM_D(glob_suspensionCompression)=4.4f;
CFG_PARAM_D(glob_rollInfluence)=0.1f;//1.0f;
CFG_PARAM_D(glob_suspensionRestLength)=.6;

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


  // accell dynamic info
  m_throttle=0.;
  m_throttleIncrement=100.;

  // steering dynamic info
  m_steering = 0.;
  m_steeringIncrement = glob_steeringIncrement;
  m_steeringClamp = glob_steeringClamp;

  m_wheelFriction = glob_wheelFriction;
  m_suspensionStiffness = glob_suspensionStiffness;
  m_suspensionDamping = glob_suspensionDamping;
  m_suspensionCompression = glob_suspensionCompression;
  m_rollInfluence = glob_rollInfluence;
  m_suspensionRestLength= glob_suspensionRestLength;

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

  float dX= (bb.MaxEdge.X - bb.MinEdge.X)/2.;
  float dY= (bb.MaxEdge.Y - bb.MinEdge.Y)/2.;
  float dZ= (bb.MaxEdge.Z - bb.MinEdge.Z)/2.;

  // create shapes
  m_chassisShape = new btBoxShape(btVector3(dX,dY,dZ));

  btTransform localTrans(btTransform::getIdentity());
  m_vehicleShape->addChildShape(localTrans,m_chassisShape);

  btTransform tr(btTransform::getIdentity());

  // create ray cast vehicle
  m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_world);
	btRaycastVehicle::btVehicleTuning	m_tuning;

  m_carBody=m_world->createRigidBody(this, glob_chassisDefaultMass, tr,m_vehicleShape);
  
  //m_raycastVehicle = new btRaycastVehicle(m_tuning,m_carBody,m_vehicleRayCaster);

#if 0
  int rightIndex = 2;
  int upIndex = 1;
  int forwardIndex = 0;
#endif

  // direction of raycast
  btVector3 wheelDirection(0,-1,0);
  // wheel rotation axis
	btVector3 wheelAxleCS(0,0,-1);
  
#if 0
  for(int i=0; i<4; i++) {
    btVector3 conn(m_wheelPositions[i].X,
        m_wheelPositions[i].Y,
        m_wheelPositions[i].Z);

    m_raycastVehicle->addWheel(
        conn,
        wheelDirection,
        wheelAxleCS,
        m_suspensionRestLength,
        m_wheelRadiuses[i],
        m_tuning,
        isFrontWheel(i));
    if(isFrontWheel(i)) {
      GM_LOG("front wheel has x: %f,%f\n",m_wheelPositions[i].X,m_suspensionRestLength);
    }

    btWheelInfo& wheel = m_raycastVehicle->getWheelInfo(i);
    wheel.m_frictionSlip = m_wheelFriction;
    wheel.m_suspensionStiffness = m_suspensionStiffness;
    wheel.m_wheelsDampingRelaxation = m_suspensionDamping;
    wheel.m_wheelsDampingCompression = m_suspensionCompression;
    wheel.m_rollInfluence = m_rollInfluence;

    m_raycastVehicle->resetSuspension();

 	  m_raycastVehicle->updateWheelTransform(i,true);

  }
#endif

  ///never deactivate the vehicle
  m_carBody->setActivationState(DISABLE_DEACTIVATION);

#if 0
  m_world->addVehicle(m_raycastVehicle);
#endif

  //choose coordinate system
  //m_raycastVehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

  dumpDebugInfo();

  m_using|=USE_PHYSICS;
}

#if 0
static void btTransformToIrrlichtMatrix(const btTransform& worldTrans, irr::core::matrix4 &matr) 
{ 
  worldTrans.getOpenGLMatrix(matr.pointer()); 
} 

static void btTransformFromIrrlichtMatrix(const irr::core::matrix4& irrmat, btTransform &transform) 
{ 
    transform.setIdentity(); 

    transform.setFromOpenGLMatrix(irrmat.pointer()); 
}
#endif



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

  m_chassisNode=new CompoundSceneNode(this,smgr,0x51ca);

  n=m_chassis.size();
  GM_LOG("- chassis %d nodes\n",n);
  // actually build nodes
  for(i=0; i<n; ++i) {
    node=smgr->addAnimatedMeshSceneNode(m_chassis[i],m_chassisNode,0xcafe);
  }

  // calculate bounding box only for chassis !!
  recalculateBoundingBox();

  for(i=0; i<4; ++i) {
    m_wheelsNodes[i]=smgr->addAnimatedMeshSceneNode(m_wheels[i],this,0xcaf0);
  }
  
  m_using|=USE_GRAPHICS;
}

void Vehicle::deinitGraphics()
{
  if(!(m_using & USE_GRAPHICS)) {
    WARNING("cannot init graphics, already deinited\n");
    return;
  }
#if 0
  irr::u32 i,n;
  irr::scene::ISceneNode * node;
  n=m_irrNodes.size();
  for(i=0; i<n; ++i) {
    node=m_irrNodes[i];
    node->remove();
  }
  m_irrNodes.clear();
#endif
  GM_LOG("done done\n");
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
//
        } else if(strcmp("wheelFriction",xmlReader->getNodeName())==0) {
          ot=ot_wheelFriction;
        } else if(strcmp("suspensionStiffness",xmlReader->getNodeName())==0) {
          ot=ot_suspensionStiffness;
        } else if(strcmp("suspensionDamping",xmlReader->getNodeName())==0) {
          ot=ot_suspensionDamping;
        } else if(strcmp("suspensionCompression",xmlReader->getNodeName())==0) {
          ot=ot_suspensionCompression;
        } else if(strcmp("rollInfluence",xmlReader->getNodeName())==0) {
          ot=ot_rollInfluence;
        } else if(strcmp("suspensionRestLength",xmlReader->getNodeName())==0) {
          ot=ot_suspensionRestLength;
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
          case ot_wheelFriction:
            m_wheelFriction=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_wheelFriction=%f\n", m_wheelFriction);
            break;
          case ot_suspensionStiffness:
            m_suspensionStiffness=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_suspensionStiffness=%f\n", m_suspensionStiffness);
            break;
          case ot_suspensionDamping:
            m_suspensionDamping=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_suspensionDamping=%f\n", m_suspensionDamping);
            break;
          case ot_suspensionCompression:
            m_suspensionCompression=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_suspensionCompression=%f\n", m_suspensionCompression);
            break;
          case ot_rollInfluence:
            m_rollInfluence=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_rollInfluence=%f\n", m_rollInfluence);
            break;
          case ot_suspensionRestLength:
            m_suspensionRestLength=Util::parseFloat(xmlReader->getNodeName());
            GM_LOG("m_suspensionRestLength=%f\n", m_suspensionRestLength);
            break;

          case ot_wrl_position:
          case ot_wrr_position:
          case ot_wfl_position:
          case ot_wfr_position:
            {
              int widx=nodeStack[nodeStackPtr]-ot_first_position;
              assert(widx>-1 && widx<4);
              Util::parseVector(xmlReader->getNodeName(),m_wheelInitialPositions[widx]);
              GM_LOG("Position of '%s' is '%s' -> %2.3f,%2.3f,%2.3f\n",
                  wheel_names[widx],
                  xmlReader->getNodeName(),
                  m_wheelInitialPositions[widx].X,
                  m_wheelInitialPositions[widx].Y,
                  m_wheelInitialPositions[widx].Z);
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
  m_chassisNode->setPosition(pos);


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

void Vehicle::throttleUp()
{
  m_throttle+=m_throttleIncrement;
}

void Vehicle::throttleDown()
{
  m_throttle-=m_throttleIncrement;
}

void Vehicle::throttleSet(double value)
{
  m_throttle=value;
}

void Vehicle::steerLeft()
{
  m_steering += m_steeringIncrement;
  if (	m_steering > m_steeringClamp)
    m_steering = m_steeringClamp;
}

void Vehicle::steerRight()
{
  m_steering -= m_steeringIncrement;
  if (	m_steering < -m_steeringClamp)
    m_steering = -m_steeringClamp;
}

void Vehicle::step()
{
  if(m_carBody==0)
    return;
	btTransform trans;
  m_carBody->getMotionState()->getWorldTransform(trans);

#if 0
  m_chassisNode->setPosition(
          irr::core::vector3df(
            float(trans.getOrigin().getX()),
            float(trans.getOrigin().getY()),
            float(trans.getOrigin().getZ())));
#endif

  irr::core::matrix4 matr;
  PhyWorld::btTransformToIrrlichtMatrix(trans, matr);

  m_chassisNode->setPosition(matr.getTranslation());
  m_chassisNode->setRotation(matr.getRotationDegrees());

  for(int i=0; i<4; i++) {
    updateWheel(i);
  }

#if 0
  for(int i=0; i<4; i++) {
    if(isFrontWheel(i)) {
      m_raycastVehicle->setSteeringValue(m_vehicleSteering,i);
    } else {
      m_raycastVehicle->applyEngineForce(m_throttle,i);
		  m_raycastVehicle->setBrake(0.f,i);
    }

		m_raycastVehicle->updateWheelTransform(i,true);

    updateWheelsFromPhysics(i);
  }
#endif
}

static void dumpWheelInfo(const btWheelInfo & info)
{
  GM_LOG("---------------------------\n");
/*
  GM_LOG("Contact normals  : %f,%f,%f\n",
      info.m_raycastInfo.m_contactNormalWS.getX(),
      info.m_raycastInfo.m_contactNormalWS.getY(),
      info.m_raycastInfo.m_contactNormalWS.getZ());
  GM_LOG("Contact points   : %f,%f,%f\n",
      info.m_raycastInfo.m_contactPointWS.getX(),
      info.m_raycastInfo.m_contactPointWS.getY(),
      info.m_raycastInfo.m_contactPointWS.getZ());
 */

  GM_LOG("Contact points   : %f,%f,%f\n",
      info.m_raycastInfo.m_wheelAxleWS.getX(),
      info.m_raycastInfo.m_wheelAxleWS.getY(),
      info.m_raycastInfo.m_wheelAxleWS.getZ());

  GM_LOG("Wheel direction : %f,%f,%f\n",
      info.m_wheelDirectionCS.getX(),
      info.m_wheelDirectionCS.getY(),
      info.m_wheelDirectionCS.getZ());

  GM_LOG("Wheel direction WS : %f,%f,%f\n",
      info.m_raycastInfo.m_wheelDirectionWS.getX(),
      info.m_raycastInfo.m_wheelDirectionWS.getY(),
      info.m_raycastInfo.m_wheelDirectionWS.getZ());

  GM_LOG("Wheel axle : %f,%f,%f\n",
      info.m_wheelAxleCS.getX(),
      info.m_wheelAxleCS.getY(),
      info.m_wheelAxleCS.getZ());

  GM_LOG("Suspension length: %f\n",info.m_raycastInfo.m_suspensionLength);
  GM_LOG("Is incontact?  %s\n",info.m_raycastInfo.m_isInContact?"yes":"no");
  //GM_LOG("Hard point: %f,%f,%f\n",
  //    info.m_raycastInfo.m_hardPointWS.getX(),
  //    info.m_raycastInfo.m_hardPointWS.getY(),
  //    info.m_raycastInfo.m_hardPointWS.getZ());

  btTransform tr=info.m_worldTransform;
  btVector3 o=info.m_worldTransform.getOrigin();
  GM_LOG("World transform (origin): %f,%f,%f\n",
      o.getX(),o.getY(),o.getZ());

  GM_LOG("World transform (rotation): %f,%f,%f,%f  ",
      tr.getRotation().x(),
      tr.getRotation().y(),
      tr.getRotation().z(),
      tr.getRotation().w());

  btQuaternion qu=tr.getRotation();

  GM_LOG("(axis: %f,%f,%f  angle %f)\n",
      qu.getAxis().x(),
      qu.getAxis().y(),
      qu.getAxis().z(),
      qu.getAngle()*(360.0/SIMD_2_PI));

  GM_LOG("steering: %f\n",info.m_steering);
  GM_LOG("rotation: %f\n",info.m_rotation);
  GM_LOG("---------------------------\n");

}

const void Vehicle::dumpDebugInfo()
{
  GM_LOG("Vehicle info:\n");
  btTransform trans;
  m_carBody->getMotionState()->getWorldTransform(trans);
  GM_LOG("Position: %f,%f,%f\n",
            float(trans.getOrigin().getX()),
            float(trans.getOrigin().getY()),
            float(trans.getOrigin().getZ()));
  GM_LOG("Throttle: %f, Steering: %f\n",m_throttle,m_steering);
  GM_LOG("Wheels :\n");
  if(m_raycastVehicle==0)
    return;
  for(int i=0; i<4; i++) 
    dumpWheelInfo(m_raycastVehicle->getWheelInfo(i));
}

void Vehicle::applyTorque(float x, float y, float z)
{
  m_carBody->applyTorque(btVector3(x,y,z));
}



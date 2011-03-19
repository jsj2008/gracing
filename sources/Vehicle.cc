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


CFG_PARAM_D(glob_chassisDefaultMass)=1.;


//CFG_PARAM_D(glob_gVehicleSteering)=0.f;
CFG_PARAM_D(glob_steeringIncrement)=0.004f;
CFG_PARAM_D(glob_steeringClamp)=0.3f;
CFG_PARAM_D(glob_wheelRadius)=0.5f;
CFG_PARAM_D(glob_wheelWidth)=0.4f;
CFG_PARAM_D(glob_wheelFriction)=1000;//BT_LARGE_FLOAT;
CFG_PARAM_D(glob_suspensionStiffness)=20.f;
CFG_PARAM_D(glob_suspensionDamping)=2.3f;
CFG_PARAM_D(glob_suspensionCompression)=4.4f;
CFG_PARAM_D(glob_rollInfluence)=0.1f;//1.0f;
CFG_PARAM_D(glob_suspensionRestLength)=.6;
CFG_PARAM_D(glob_wheelsDampingCompression)= .90;
CFG_PARAM_D(glob_wheelsDampingRelaxation)=.83;
CFG_PARAM_D(glob_maxSuspensionTravelCm)=500.;
CFG_PARAM_D(glob_maxSuspensionForce)=6000.;

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
  m_throttleIncrement=0.00001;

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
  m_wheelsDampingCompression=glob_wheelsDampingCompression;
  m_wheelsDampingRelaxation=glob_wheelsDampingRelaxation;
  m_maxSuspensionTravelCm=glob_maxSuspensionTravelCm;
  m_maxSuspensionForce=glob_maxSuspensionForce;

  m_chassisWorldTrans=btTransform::getIdentity();

  for(int i=0; i<W_NUM_OF_WHEELS; i++)  {
    m_wheels[i]=0;
    m_wheelShapes[i]=0;
  }

  m_chassisShape=0;
  m_carBody=0;

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

  const irr::core::aabbox3d<float> & bb=m_chassisNode->getBoundingBox();

  float dX= (bb.MaxEdge.X - bb.MinEdge.X)/2.;
  float dY= (bb.MaxEdge.Y - bb.MinEdge.Y)/2.;
  float dZ= (bb.MaxEdge.Z - bb.MinEdge.Z)/2.;

  // create shapes
  m_chassisShape = new btBoxShape(btVector3(dX,dY,dZ));

  btTransform localTrans(btTransform::getIdentity());

  btTransform tr(btTransform::getIdentity());
  m_carBody=m_world->createRigidBody(0, glob_chassisDefaultMass, tr,m_chassisShape, this);

  for(int i=0; i<4; i++) {
    m_wheelsData[i].hardPointCS=btVector3(
                  m_wheelInitialPositions[i].X,
                  m_wheelInitialPositions[i].Y,
                  m_wheelInitialPositions[i].Z);

    m_wheelsData[i].radius=m_wheelRadiuses[i];
    m_wheelsData[i].suspensionLength=m_suspensionRestLength;
    m_wheelsData[i].rotation=0.;
  }
  
  m_carBody->setActivationState(DISABLE_DEACTIVATION);
  m_world->addAction(this);
  m_raycaster = new btDefaultVehicleRaycaster(m_world);

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


  m_chassisNode=new CompoundSceneNode(this,smgr,0x51ca);

  n=m_chassis.size();
  // actually build nodes
  for(i=0; i<n; ++i) {
    node=smgr->addAnimatedMeshSceneNode(m_chassis[i],m_chassisNode,0xcafe);
  }

  ((CompoundSceneNode*)m_chassisNode)->recalculateBoundingBox();

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
#if 0
  const char * wheel_names[4]={
    "rear left",
    "rear right",
    "front left",
    "front right"
  };
#endif

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
            break;
          case ot_suspensionStiffness:
            m_suspensionStiffness=Util::parseFloat(xmlReader->getNodeName());
            break;
          case ot_suspensionDamping:
            m_suspensionDamping=Util::parseFloat(xmlReader->getNodeName());
            break;
          case ot_suspensionCompression:
            m_suspensionCompression=Util::parseFloat(xmlReader->getNodeName());
            break;
          case ot_rollInfluence:
            m_rollInfluence=Util::parseFloat(xmlReader->getNodeName());
            break;
          case ot_suspensionRestLength:
            m_suspensionRestLength=Util::parseFloat(xmlReader->getNodeName());
            break;

          case ot_wrl_position:
          case ot_wrr_position:
          case ot_wfl_position:
          case ot_wfr_position:
            {
              int widx=nodeStack[nodeStackPtr]-ot_first_position;
              assert(widx>-1 && widx<4);
              Util::parseVector(xmlReader->getNodeName(),m_wheelInitialPositions[widx]);
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
            }
            break;

          case ot_wfl:
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

  m_steering=0.;
  m_throttle=0.;


  // reset position
  btTransform trans=btTransform::getIdentity();
  trans.setOrigin(btVector3(pos.X,pos.Y,pos.Z));
  m_carBody->setCenterOfMassTransform(trans);


  // reset velocity
  m_carBody->setLinearVelocity(btVector3(0,0,0));
  m_carBody->setAngularVelocity(btVector3(0,0,0));

  // TODO: reset wheels

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

void Vehicle::brake()
{
  m_throttle=0;
  m_brake=.1;
}

void Vehicle::throttleUp()
{
  m_brake=0.;
  m_throttle+=m_throttleIncrement;
}

void Vehicle::throttleDown()
{
  m_brake=0.;
  m_throttle-=m_throttleIncrement;
}

void Vehicle::throttleSet(double value)
{
  m_throttle=value;
}

void Vehicle::steerRight()
{
  m_steering += m_steeringIncrement;
  if (	m_steering > m_steeringClamp)
    m_steering = m_steeringClamp;
}

void Vehicle::steerLeft()
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
  btMotionState * ms;
  assert(m_carBody);
  ms=m_carBody->getMotionState();
  assert(ms);
  
  ms->getWorldTransform(trans);

  irr::core::matrix4 matr;
  PhyWorld::btTransformToIrrlichtMatrix(trans, matr);

  m_chassisNode->setPosition(matr.getTranslation());
  m_chassisNode->setRotation(matr.getRotationDegrees());

  for(int i=0; i<4; i++) updateWheel(i);
}

void Vehicle::applyTorque(float x, float y, float z)
{
  m_carBody->applyTorque(btVector3(x,y,z));
}

irr::core::vector3df Vehicle::getChassisPos()
{
  return m_chassisNode->getPosition();
}

void Vehicle::updateAction(btCollisionWorld* world, btScalar deltaTime)
{
  // steps:
  // 1- update wheel wolrd space transforn transform
  for(int i=0; i<4; i++) {
    WheelData & wheel=m_wheelsData[i];

    wheel.isInContact=false;
    btTransform chassisTrans = m_carBody->getCenterOfMassTransform();

    wheel.hardPointWS = chassisTrans * wheel.hardPointCS; ///chassisTrans( wheel.hardPointCS );
    wheel.directionWS = chassisTrans.getBasis() * btVector3(0.,-1.,0.);
    wheel.axleWS = chassisTrans.getBasis() *  btVector3(0.,0.,1.);

    //
	  btVector3 up = -wheel.directionWS;
	  const btVector3& right = wheel.axleWS;
    btVector3 fwd = up.cross(right);
    btScalar steering = btScalar(0.);
    if(isFrontWheel(i))
      steering=m_steering;

    btQuaternion steeringOrn(up,steering);
    btMatrix3x3 steeringMat(steeringOrn);

    btQuaternion rotatingOrn(right,-wheel.rotation);
    btMatrix3x3 rotatingMat(rotatingOrn);

#if 0
    btMatrix3x3 basis2(
        right[0],fwd[0],up[0],
        right[1],fwd[1],up[1],
        right[2],fwd[2],up[2]
        );
#endif
    wheel.worldTransform=btTransform::getIdentity();
    //wheel.worldTransform.setRotation(btQuaternion(btVector3(0.,1.,0.),M_PI/2));

    //wheel.worldTransform.setBasis(steeringMat * rotatingMat /** basis2*/);
    wheel.worldTransform.setOrigin( wheel.hardPointWS + wheel.directionWS * wheel.suspensionLength /*m_suspensionRestLength*/);
  }

  // 2- TODO: update speed km/h

  // 3- simulate suspensions
  for (int i=0;i<4;i++)
  {
    btScalar depth; 
    WheelData & wheel=m_wheelsData[i];
    depth = raycast(wheel,i);
  }

  for(int i=0; i<4; i++) 
  {
    WheelData & wheel=m_wheelsData[i];
    if ( wheel.isInContact ) {
      btScalar force;
      { // spring
        btScalar	susp_length	= m_suspensionRestLength;
        btScalar	current_length = wheel.suspensionLength;

        btScalar length_diff = (susp_length - current_length);

        force = m_suspensionStiffness
          * length_diff * wheel.clippedInvContactDotSuspension;
      }
      { // damping
        btScalar projected_rel_vel = wheel.suspensionRelativeVelocity;
        btScalar susp_damping;
        if ( projected_rel_vel < btScalar(0.0) )
          susp_damping = m_wheelsDampingCompression;
        else
          susp_damping = m_wheelsDampingRelaxation;
        force -= susp_damping * projected_rel_vel;
      }
      wheel.suspensionForce = force;

    } else {
      wheel.suspensionForce = btScalar(0.0);
    }

  }

  for (int i=0;i<4;i++)
  {
    //apply suspension force
    WheelData& wheel = m_wheelsData[i];

    btScalar suspensionForce = wheel.suspensionForce;

    if (suspensionForce > m_maxSuspensionForce)
    {
      suspensionForce = m_maxSuspensionForce;
    }
    btVector3 impulse = wheel.contactNormalWS * suspensionForce * deltaTime;
    btVector3 relpos = wheel.contactPointWS - m_carBody->getCenterOfMassPosition();

    m_carBody->applyImpulse(impulse, relpos);
  }

  // 4- update friction
  updateFriction();
  // 5- update wheels rotation
}


void Vehicle::updateFriction() 
{
  m_forwardWS.resize(4);
  m_axle.resize(4);
  m_forwardImpulse.resize(4);
  m_sideImpulse.resize(4);

  int numWheelsOnGround = 0;


  //collapse all those loops into one!
  for (int i=0;i<4;i++)
  {
    WheelData& wheel = m_wheelsData[i];
    class btRigidBody* groundObject = (class btRigidBody*) wheel.collidingObject;
    if (groundObject)
      numWheelsOnGround++;
    m_sideImpulse[i] = btScalar(0.);
    m_forwardImpulse[i] = btScalar(0.);
  }


  for (int i=0;i<4;i++) {

    WheelData& wheel = m_wheelsData[i];
    class btRigidBody* groundObject = (class btRigidBody*) wheel.collidingObject;

    if (groundObject) {
#if 0

      const btTransform& wheelTrans = wheel.worldTransform;

      btMatrix3x3 wheelBasis0 = wheelTrans.getBasis();
      m_axle[i] = btVector3(	
          wheelBasis0[0][m_indexRightAxis],
          wheelBasis0[1][m_indexRightAxis],
          wheelBasis0[2][m_indexRightAxis]);

      const btVector3& surfNormalWS = wheelInfo.m_raycastInfo.m_contactNormalWS;
      btScalar proj = m_axle[i].dot(surfNormalWS);
      m_axle[i] -= surfNormalWS * proj;
      m_axle[i] = m_axle[i].normalize();

      m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
      m_forwardWS[i].normalize();


      resolveSingleBilateral(*m_chassisBody, wheelInfo.m_raycastInfo.m_contactPointWS,
          *groundObject, wheelInfo.m_raycastInfo.m_contactPointWS,
          btScalar(0.), m_axle[i],m_sideImpulse[i],timeStep);

      m_sideImpulse[i] *= sideFrictionStiffness2;
#endif

    }
  }

}

btScalar Vehicle::raycast(WheelData & wheel, int wnumber)
{
  // NB: raycast assume that wheel has been
  // already updated!!
	btScalar depth = -1.;
	btScalar raylen = wheel.suspensionLength + wheel.radius + .5;
	btVector3 rayvector = wheel.directionWS * (raylen);
	const btVector3& source = wheel.hardPointWS;

	wheel.contactPointWS = source + rayvector;
	const btVector3& target = wheel.contactPointWS;

	btVehicleRaycaster::btVehicleRaycasterResult	rayResults;

	btScalar param = btScalar(0.);

	void* object = m_raycaster->castRay(source,target,rayResults);


  if(object) {

    wheel.collidingObject=m_carBody; // TODO: dont understand this!!

    wheel.isInContact=true;

		param = rayResults.m_distFraction;
		depth = raylen * rayResults.m_distFraction;

		wheel.contactNormalWS = rayResults.m_hitNormalInWorld;

		btScalar hitDistance = param*raylen;
		wheel.suspensionLength = hitDistance - wheel.radius;

		btScalar minSuspensionLength = m_suspensionRestLength - m_maxSuspensionTravelCm*btScalar(0.01);
		btScalar maxSuspensionLength = m_suspensionRestLength + m_maxSuspensionTravelCm*btScalar(0.01);
		if (wheel.suspensionLength < minSuspensionLength)
		{
			wheel.suspensionLength = minSuspensionLength;
		}
		if (wheel.suspensionLength > maxSuspensionLength)
		{
			wheel.suspensionLength = maxSuspensionLength;
		}

		wheel.contactPointWS = rayResults.m_hitPointInWorld;

		btScalar denominator= wheel.contactNormalWS.dot( wheel.directionWS );

		btVector3 chassis_velocity_at_contactPoint;
		btVector3 relpos = wheel.contactPointWS-m_carBody->getCenterOfMassPosition();

		chassis_velocity_at_contactPoint = m_carBody->getVelocityInLocalPoint(relpos);

		btScalar projVel = wheel.contactNormalWS.dot( chassis_velocity_at_contactPoint );

		if ( denominator >= btScalar(-0.1))
		{
			wheel.suspensionRelativeVelocity = btScalar(0.0);
			wheel.clippedInvContactDotSuspension = btScalar(1.0) / btScalar(0.1);
		}
		else
		{
			btScalar inv = btScalar(-1.) / denominator;
			wheel.suspensionRelativeVelocity = projVel * inv;
			wheel.clippedInvContactDotSuspension = inv;
		}
  } else {
    wheel.collidingObject=0;
    wheel.isInContact=false;
		wheel.suspensionLength = m_suspensionRestLength;
		wheel.suspensionRelativeVelocity = btScalar(0.0);
		wheel.contactNormalWS = - wheel.directionWS;
		wheel.clippedInvContactDotSuspension = btScalar(1.0);
  }

  return depth;
}

void Vehicle::debugDraw(btIDebugDraw* debugDrawer)
{
  btVector3 color(0,0,0);
  for(int i=0; i<4; i++) {
    WheelData & wheel=m_wheelsData[i];

		btVector3 & point1 = wheel.hardPointWS;
    btVector3  point2 = point1 + wheel.axleWS * .5;
		debugDrawer->drawLine(point1,point2,color);

    point2 = point1 + wheel.directionWS * 2.;
		debugDrawer->drawLine(point1,point2,color);
  }
}

void 	Vehicle::getWorldTransform (btTransform &worldTrans) const
{
   worldTrans=m_chassisWorldTrans;
}

void 	Vehicle::setWorldTransform (const btTransform &worldTrans)
{
   m_chassisWorldTrans=worldTrans;
   step();
}

void Vehicle::updateWheel(int index)
{
  assert(index>=0 && index<4);

  irr::scene::ISceneNode * wheel=m_wheelsNodes[index];

  assert(wheel);

  btTransform & wheelTrans=m_wheelsData[index].worldTransform;
  //btTransform wheelTrans=btTransform::getIdentity();

  irr::core::matrix4 matr;
  PhyWorld::btTransformToIrrlichtMatrix(wheelTrans, matr);

  //wheel->setRotation(matr.getRotationDegrees());
  wheel->setPosition(matr.getTranslation());
  static float r=0.;
  //r+=.5;
  wheel->setRotation(irr::core::vector3df(0.,r,0.));
}


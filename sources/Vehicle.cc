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

// TODO: 
// 1- move 'm_maxSuspensionTravel' from vehicle to wheel (one per wheel!)

#include <string.h>
#include <assert.h>

#include "Vehicle.h"
#include "util.hh"
#include "gmlog.h"
#include "XmlNode.h"

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

  enum {
    widx_wrl=0,
    widx_wrr=1,
    widx_wfl=2,
    widx_wfr=3,
  };

CFG_PARAM_D(glob_chassisDefaultMass)=5.5; // min: .8, max: 5.5
CFG_PARAM_D(glob_maxThrottle)=2.7;    // min: .7, max: 2.7
CFG_PARAM_D(glob_steeringClamp)=0.3f;
CFG_PARAM_D(glob_wheelRadius)=0.5f;
CFG_PARAM_D(glob_wheelWidth)=0.4f;
CFG_PARAM_D(glob_suspensionCompression)=4.4f;
CFG_PARAM_D(glob_rollInfluence)=0.1f;//1.0f;
CFG_PARAM_D(glob_suspensionRestLength)=.1; //.6;

CFG_PARAM_D(glob_suspensionStiffness)=80.;
CFG_PARAM_D(glob_suspensionDamping)=2.3f;
CFG_PARAM_D(glob_maxSuspensionTravel)=.1;
CFG_PARAM_D(glob_maxSuspensionForce)=6000.;
CFG_PARAM_D(glob_wheelsDampingCompression)=4.4;
CFG_PARAM_D(glob_wheelsDampingRelaxation)=20.; //.83;


CFG_PARAM_D(glob_wheelFriction)=1000;//BT_LARGE_FLOAT;
CFG_PARAM_D(glob_frictionSlip)=2.; //.5;

/////////////////////////////////////////////////////////////////////////

static void g_resolveSingleBilateral(btRigidBody& body1, const btVector3& pos1,
                      btRigidBody& body2, const btVector3& pos2,
                      btScalar distance, const btVector3& normal,
                      btScalar& impulse ,btScalar timeStep)
{
  (void)timeStep;
  (void)distance;

  btScalar normalLenSqr = normal.length2();
  assert(btFabs(normalLenSqr) < btScalar(1.1));
  if (normalLenSqr > btScalar(1.1))
  {
    impulse = btScalar(0.);
    return;
  }
  btVector3 rel_pos1 = pos1 - body1.getCenterOfMassPosition(); 
  btVector3 rel_pos2 = pos2 - body2.getCenterOfMassPosition();
  //this jacobian entry could be re-used for all iterations

  btVector3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
  btVector3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
  btVector3 vel = vel1 - vel2;

  btJacobianEntry jac(body1.getCenterOfMassTransform().getBasis().transpose(),
      body2.getCenterOfMassTransform().getBasis().transpose(),
      rel_pos1,rel_pos2,normal,body1.getInvInertiaDiagLocal(),body1.getInvMass(),
      body2.getInvInertiaDiagLocal(),body2.getInvMass());

  btScalar jacDiagAB = jac.getDiagonal();
  btScalar jacDiagABInv = btScalar(1.) / jacDiagAB;

  btScalar rel_vel = jac.getRelativeVelocity(
      body1.getLinearVelocity(),
      body1.getCenterOfMassTransform().getBasis().transpose() * body1.getAngularVelocity(),
      body2.getLinearVelocity(),
      body2.getCenterOfMassTransform().getBasis().transpose() * body2.getAngularVelocity()); 
  btScalar a;
  a=jacDiagABInv;

  rel_vel = normal.dot(vel);

  //todo: move this into proper structure
  btScalar contactDamping = btScalar(0.2);

#ifdef ONLY_USE_LINEAR_MASS
  btScalar massTerm = btScalar(1.) / (body1.getInvMass() + body2.getInvMass());
  impulse = - contactDamping * rel_vel * massTerm;
#else	
  btScalar velocityImpulse = -contactDamping * rel_vel * jacDiagABInv;
  impulse = velocityImpulse;
#endif
}


struct WheelContactPoint
{
	btRigidBody* m_body0;
	btRigidBody* m_body1;
	btVector3	m_frictionPositionWorld;
	btVector3	m_frictionDirectionWorld;
	btScalar	m_jacDiagABInv;
	btScalar	m_maxImpulse;


	WheelContactPoint(
      btRigidBody* body0,btRigidBody* body1,
      const btVector3& frictionPosWorld,
      const btVector3& frictionDirectionWorld, 
      btScalar maxImpulse)
		:m_body0(body0),
		m_body1(body1),
		m_frictionPositionWorld(frictionPosWorld),
		m_frictionDirectionWorld(frictionDirectionWorld),
		m_maxImpulse(maxImpulse)
	{
		btScalar denom0 = body0->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
		btScalar denom1 = body1->computeImpulseDenominator(frictionPosWorld,frictionDirectionWorld);
		btScalar	relaxation = 1.f;
		m_jacDiagABInv = relaxation/(denom0+denom1);
	}
};

btRigidBody& getFixedBody()
{
	static btRigidBody s_fixed(0, 0,0);
    s_fixed.setMassProps(btScalar(0.),btVector3(btScalar(0.),btScalar(0.),btScalar(0.)));
	return s_fixed;
}

static btScalar calcRollingFriction(WheelContactPoint& contactPoint)
{
	btScalar j1=0.f;

	const btVector3& contactPosWorld = contactPoint.m_frictionPositionWorld;

	btVector3 rel_pos1 = contactPosWorld - contactPoint.m_body0->getCenterOfMassPosition(); 
	btVector3 rel_pos2 = contactPosWorld - contactPoint.m_body1->getCenterOfMassPosition();
	
	btScalar maxImpulse  = contactPoint.m_maxImpulse;
	
	btVector3 vel1 = contactPoint.m_body0->getVelocityInLocalPoint(rel_pos1);
	btVector3 vel2 = contactPoint.m_body1->getVelocityInLocalPoint(rel_pos2);
	btVector3 vel = vel1 - vel2;

	btScalar vrel = contactPoint.m_frictionDirectionWorld.dot(vel);

	// calculate j that moves us to zero relative velocity
	j1 = -vrel * contactPoint.m_jacDiagABInv;
	btSetMin(j1, maxImpulse);
	btSetMax(j1, -maxImpulse);


	return j1;
}

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
  m_speedometer=0;
  m_loaded=false;
  m_controlsEnabled=true;

  m_name=source;

  // accell dynamic info
  m_throttle=0.;
#ifndef ANALOG_CONTROLS
  m_throttleIncrement=glob_throttleIncrement;
  m_throttleDecrement=glob_throttleDecrement;
#endif 

  // steering dynamic info
  m_steering = 0.;
#ifndef ANALOG_CONTROLS
  m_steeringIncrement = glob_steeringIncrement;
#endif
  m_steeringClamp = glob_steeringClamp;

  m_wheelFriction = glob_wheelFriction; // NO MORE USED !!!
  m_suspensionStiffness = glob_suspensionStiffness;
  m_suspensionDamping = glob_suspensionDamping;
  m_suspensionCompression = glob_suspensionCompression;
  m_rollInfluence = glob_rollInfluence;
  m_suspensionRestLength= glob_suspensionRestLength;
  m_wheelsDampingCompression=glob_wheelsDampingCompression;
  m_wheelsDampingRelaxation=glob_wheelsDampingRelaxation;
  m_maxSuspensionTravel=glob_maxSuspensionTravel;
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

#if 0
  if(!(m_using & USE_GRAPHICS)) {
    WARNING("cannot init phisics, must first init graphics");
    return;
  }

  if(m_using & USE_PHYSICS) {
    return;
  }
#endif

  // the chassis shape
  // TODO: use the mesh to create the shape

  const irr::core::aabbox3d<float> & bb=m_chassisNode->getBoundingBox();

  float dX= (bb.MaxEdge.X - bb.MinEdge.X)/2.;
  float dY= (bb.MaxEdge.Y - bb.MinEdge.Y)/2.;
  float dZ= (bb.MaxEdge.Z - bb.MinEdge.Z)/2.;

  float overX=(bb.MaxEdge.X + bb.MinEdge.X)/2.;
  float overY=(bb.MaxEdge.Y + bb.MinEdge.Y)/2.;
  float overZ=(bb.MaxEdge.Z + bb.MinEdge.Z)/2.;

  if(bb.MaxEdge.X != -bb.MinEdge.X) {
    GM_LOG("WARNING: chassis not centered on X %f,%f\n",bb.MinEdge.X,bb.MaxEdge.X);
    GM_LOG("WARNING: should be translated of %f\n",overX);
  }

  if(bb.MaxEdge.Y != -bb.MinEdge.Y) {
    GM_LOG("WARNING: chassis not centered on Y %f,%f\n",bb.MinEdge.Y,bb.MaxEdge.Y);
    GM_LOG("WARNING: should be translated of %f\n",overY);
  }

  if(bb.MaxEdge.Z != -bb.MinEdge.Z) {
    GM_LOG("WARNING: chassis not centered on Z %f,%f\n",bb.MinEdge.Z,bb.MaxEdge.Z);
    GM_LOG("WARNING: should be translated of %f\n",overZ);
  }

  // create shapes
  m_chassisShape = new btBoxShape(btVector3(dX,dY,dZ));


  // calc position of the center of the vehicle
  // center of mass.
  // put it in the middle of the wheels
  btVector3 centerOfMass(0.,0.,0.);
  for(int i=0; i<4; i++) {
    centerOfMass += btVector3(
        m_wheelInitialPositions[i].X,
        m_wheelInitialPositions[i].Y,
        m_wheelInitialPositions[i].Z);
  }
  centerOfMass /= 4.;
  /*
     ??????
  centerOfMass.setY(30);
  centerOfMass -= btVector3(10., 0., 0.);
  */

  btTransform tr(btTransform::getIdentity());
  tr.setOrigin(centerOfMass);

  m_carBody=m_world->createRigidBody(0, glob_chassisDefaultMass, tr,m_chassisShape, this);

  for(int i=0; i<4; i++) {
    m_wheelsData[i].hardPointCS=btVector3(
                  m_wheelInitialPositions[i].X,
                  m_wheelInitialPositions[i].Y,
                  m_wheelInitialPositions[i].Z);


    m_wheelsData[i].radius=m_wheelRadiuses[i];
    m_wheelsData[i].suspensionLength=m_suspensionRestLength;
    m_wheelsData[i].rotation=0.;
    m_wheelsData[i].frictionSlip=glob_frictionSlip;
  }
  
  m_carBody->setActivationState(DISABLE_DEACTIVATION);
  m_world->addAction(this);
  m_raycaster = new btDefaultVehicleRaycaster(m_world);

}

void Vehicle::initGraphics()
{
  if(!m_loaded) {
    WARNING("cannot init graphics, coz vehicle still not loaded");
    return;
  }

  irr::u32 n;
  irr::u32 i;
  irr::scene::ISceneManager * smgr=
    m_device->getSceneManager();

  m_chassisNode=new CompoundSceneNode(this,smgr,0x51ca);

  bool enableShadows=false;
  ResourceManager::getInstance()->cfgGet("glob_enableShadows",enableShadows);

  n=m_chassis.size();
  // actually build nodes
  for(i=0; i<n; ++i) {
    irr::scene::IAnimatedMeshSceneNode* anode = 0;
    anode=smgr->addAnimatedMeshSceneNode(m_chassis[i],m_chassisNode,0xcafe);
    if(enableShadows)
      anode->addShadowVolumeSceneNode();
  }


  ((CompoundSceneNode*)m_chassisNode)->recalculateBoundingBox();

  for(i=0; i<4; ++i) {
    m_wheelsNodes[i]=smgr->addAnimatedMeshSceneNode(m_wheels[i],this,0xcaf0);
    m_wheelsNodes[i]->grab();
  }
  
  //m_using|=USE_GRAPHICS;
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

void Vehicle::loadInfo(XmlNode * root)
{
  std::vector<XmlNode*> nodes;
  root->getChildren(nodes);
  for(unsigned int i=0; i<nodes.size(); i++) {
    XmlNode * node=nodes[i];

    if(node->getName() == "name") 
      m_name=node->getText();
  }
}

void Vehicle::load()
{
  if(m_loaded)
    return ;
  XmlNode * root=0;

  irr::io::path mypath(m_sourceName);
  bool res=m_filesystem->addFileArchive(mypath);

  if(!res) 
    return ;

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  assert(xml); // TODO: should not be an assert

  root=new XmlNode(xml);
  assert(root && root->getName() == "vehicle"); // TODO: this must not be an assert!!!

  std::vector<XmlNode*> nodes;
  root->getChildren(nodes);

  irr::scene::IAnimatedMesh* mesh;
  irr::scene::ISceneManager * smgr=m_device->getSceneManager();

  for(unsigned int i=0; i<nodes.size(); i++) {
    XmlNode * node=nodes[i];

    if(node->getName() == "chassis") {
      mesh=smgr->getMesh(node->getText().c_str());
      WARNING_IF(mesh==0," - cannot load file '%s'\n",
          node->getText().c_str());
      if(mesh) {
        mesh->grab();
        m_chassis.push_back(mesh);
      }

    } else if(node->getName() == "info") {
      loadInfo(node);
    } else if(node->getName() == "wfr_width") {
      double width=Util::parseFloat(node->getText().c_str());
      m_wheelWidths[ widx_wfr  ]=width;

    } else if(node->getName() == "wfl_width") {
      double width=Util::parseFloat(node->getText().c_str());
      m_wheelWidths[ widx_wfl  ]=width;

    } else if(node->getName() == "wrl_width") {
      double width=Util::parseFloat(node->getText().c_str());
      m_wheelWidths[ widx_wrl  ]=width;

    } else if(node->getName() == "wrr_width") {
      double width=Util::parseFloat(node->getText().c_str());
      m_wheelWidths[ widx_wrr  ]=width;

    } else if(node->getName() == "wfr_radius") {
      double radius=Util::parseFloat(node->getText().c_str());
      m_wheelRadiuses[ widx_wfr  ]=radius;
    } else if(node->getName() == "wfl_radius") {
      double radius=Util::parseFloat(node->getText().c_str());
      m_wheelRadiuses[ widx_wfl  ]=radius;

    } else if(node->getName() == "wrl_radius") {
      double radius=Util::parseFloat(node->getText().c_str());
      m_wheelRadiuses[ widx_wrl  ]=radius;

    } else if(node->getName() == "wrr_radius") {
      double radius=Util::parseFloat(node->getText().c_str());
      m_wheelRadiuses[ widx_wrr  ]=radius;

    } else if(node->getName() == "wfr_position") {
      Util::parseVector(node->getText().c_str(),m_wheelInitialPositions[widx_wfr]);

    } else if(node->getName() == "wfl_position") {
      Util::parseVector(node->getText().c_str(),m_wheelInitialPositions[widx_wfl]);

    } else if(node->getName() == "wrl_position") {
      Util::parseVector(node->getText().c_str(),m_wheelInitialPositions[widx_wrl]);

    } else if(node->getName() == "wrr_position") {
      Util::parseVector(node->getText().c_str(),m_wheelInitialPositions[widx_wrr]);

    } else if(node->getName() == "wfr") {
      mesh=smgr->getMesh(node->getText().c_str());
      WARNING_IF(mesh==0," - cannot load file '%s'\n",
          node->getText().c_str());
      if(m_wheels[widx_wfr]) {
        WARNING("Double definition of wheel: front right");
        break;
      }
      if(mesh) {
        m_wheels[widx_wfr]=mesh;
        mesh->grab();
        smgr->getMeshCache()->renameMesh(mesh, "");
      }
    } else if(node->getName() == "wfl") {
      mesh=smgr->getMesh(node->getText().c_str());
      WARNING_IF(mesh==0," - cannot load file '%s'\n",
          node->getText().c_str());
      if(m_wheels[widx_wfl]) {
        WARNING("Double definition of wheel: front right");
        break;
      }
      if(mesh) {
        m_wheels[widx_wfl]=mesh;
        mesh->grab();
        smgr->getMeshCache()->renameMesh(mesh, "");
      }
    } else if(node->getName() == "wrr") {
      mesh=smgr->getMesh(node->getText().c_str());
      WARNING_IF(mesh==0," - cannot load file '%s'\n",
          node->getText().c_str());
      if(m_wheels[widx_wrr]) {
        WARNING("Double definition of wheel: front right");
        break;
      }
      if(mesh) {
        m_wheels[widx_wrr]=mesh;
        mesh->grab();
        smgr->getMeshCache()->renameMesh(mesh, "");
      }
    } else if(node->getName() == "wrl") {
      mesh=smgr->getMesh(node->getText().c_str());
      WARNING_IF(mesh==0," - cannot load file '%s'\n",
          node->getText().c_str());
      if(m_wheels[widx_wrl]) {
        WARNING("Double definition of wheel: front right");
        break;
      }
      if(mesh) {
        m_wheels[widx_wrl]=mesh;
        mesh->grab();
        smgr->getMeshCache()->renameMesh(mesh, "");
      }

    } else if(node->getName() == "wheelFriction") {
      m_wheelFriction=Util::parseFloat(node->getText().c_str());

    } else if(node->getName() == "suspensionStiffness") {
      m_suspensionStiffness=Util::parseFloat(node->getText().c_str());

    } else if(node->getName() == "suspensionDamping") {
      m_suspensionDamping=Util::parseFloat(node->getText().c_str());

    } else if(node->getName() == "suspensionCompression") {
      m_suspensionCompression=Util::parseFloat(node->getText().c_str());

    } else if(node->getName() == "rollInfluence") {
      m_rollInfluence=Util::parseFloat(node->getText().c_str());

    } else if(node->getName() == "suspensionRestLength") {
      m_suspensionRestLength=Util::parseFloat(node->getText().c_str());
    } 
  }

  delete xml;
  res=m_filesystem->removeFileArchive(m_filesystem->getAbsolutePath(mypath));
  assert(res);
  delete root;
  m_loaded=true;

  initGraphics();
  initPhysics();

  //m_carBody->setActivationState(DISABLE_SIMULATION);
}


void Vehicle::reset(const irr::core::vector3d<float>&pos, double rotation)
{
  if(m_using & USE_PHYSICS) {
    m_steering=0.;
    m_throttle=0.;
    m_brake=0.;

    m_vehicleCommands.throttling=0.;
#ifdef ANALOG_CONTROLS
    m_vehicleCommands.steering=0.;
#else
    m_vehicleCommands.steering=IVehicle::VehicleCommands::steerNone;
#endif

    // reset position
    btTransform trans=btTransform::getIdentity();
    trans.setOrigin(btVector3(pos.X,pos.Y,pos.Z));

    // reset rotation
    btQuaternion rotQuat(btVector3(0.,1.,0.),rotation);
    btMatrix3x3  rotMat(rotQuat);
    trans.setBasis(rotMat);

    m_carBody->setCenterOfMassTransform(trans);

    // reset velocity
    m_carBody->setLinearVelocity(btVector3(0,0,0));
    m_carBody->setAngularVelocity(btVector3(0,0,0));

    // TODO: reset wheels
    for(int i=0; i<4; i++) {
      WheelData & wheel=m_wheelsData[i];
      wheel.rotation=0.;
      wheel.deltaRotation=0.;
    }

    // reset collosion
    m_world->getBroadphase()->getOverlappingPairCache()->
      cleanProxyFromPairs(m_carBody->getBroadphaseHandle(),m_world->getDispatcher()); 
  } else if(m_using & USE_GRAPHICS) {
    m_chassisNode->setPosition(pos);
    //grad = rad * 180 / PI
    m_chassisNode->setRotation(irr::core::vector3df(0.,rad2deg(rotation),0.));

    irr::core::matrix4 chMatrix= m_chassisNode->getAbsoluteTransformation();

    for(int i=0; i<4; i++) {
      irr::scene::ISceneNode * wheel=m_wheelsNodes[i];
      wheel->setPosition(m_wheelInitialPositions[i]);
      wheel->updateAbsolutePosition();
    }
  }
}


void Vehicle::unload()
{
}

void Vehicle::use(unsigned int useFlags)
{
  irr::scene::ISceneManager * smgr=
      m_device->getSceneManager();
  if(useFlags & USE_PHYSICS && !(m_using & USE_PHYSICS)) {
    m_using|=USE_PHYSICS;
    m_world->addRigidBody(m_carBody);
    for(unsigned i=0; i<4; i++) {
      irr::scene::ISceneNode * wheel=m_wheelsNodes[i];
      wheel->setParent(smgr->getRootSceneNode());
    }
    if(!(useFlags & USE_PHYSICS))
      use(USE_GRAPHICS);
  }

  if(useFlags & USE_GRAPHICS && !(m_using & USE_GRAPHICS)) {
    m_using|=USE_GRAPHICS;
    smgr->getRootSceneNode()->addChild(this);

    if(!(m_using & USE_PHYSICS)) {
      for(unsigned i=0; i<4; i++) {
        irr::scene::ISceneNode * wheel=m_wheelsNodes[i];
        wheel->setPosition(m_wheelInitialPositions[i]);
        wheel->setRotation(irr::core::vector3df(0.,0.,0.));
        wheel->setParent(m_chassisNode);
      }
    }
  }
}

void Vehicle::unuse(unsigned int useFlags)
{
  if(useFlags & USE_PHYSICS && (m_using & USE_PHYSICS)) {
    m_world->removeRigidBody(m_carBody);
    m_using&=~USE_PHYSICS;
  }
  if(useFlags & USE_GRAPHICS && (m_using & USE_GRAPHICS)) {
    irr::scene::ISceneManager * smgr=
      m_device->getSceneManager();
    m_using&=~USE_GRAPHICS;
    smgr->getRootSceneNode()->removeChild(this);
  }
}

irr::core::vector3df Vehicle::getChassisPos()
{
  return m_chassisNode->getPosition();
}


#define _clamp(_x,_min,_max) (_x) = (_x) < (_min) ? (_min) : ( (_x) > (_max) ? (_max) : (_x))

void Vehicle::updateAction(btCollisionWorld* world, btScalar deltaTime)
{
  if(m_controlsEnabled) {
#ifdef ANALOG_CONTROLS
    m_throttle = m_vehicleCommands.throttling;
    m_steering = m_vehicleCommands.steering;
    _clamp(m_throttle, -glob_maxThrottle, glob_maxThrottle);
    _clamp(m_steering, -m_steeringClamp, m_steeringClamp); 

    if(m_vehicleCommands.brake) {
      m_brake=1.;
    }
#else 
    if(m_vehicleCommands.throttling > 0.) {
      m_brake=0.;
      m_throttle+=m_throttleIncrement;
      if(m_throttle>glob_maxThrottle)
        m_throttle=glob_maxThrottle;
    } else if(m_vehicleCommands.throttling<0) {
      m_brake=0.;

      m_throttle-=m_throttleIncrement;

      if(m_throttle<-glob_maxThrottle)  {
        m_throttle=-glob_maxThrottle;
      }
    } else {
      m_throttle=0.;
    }

    if(m_vehicleCommands.brake) {
      m_brake=1.;
    }

    //m_vehicleCommands.throttling=0;

    // halde steering
    switch(m_vehicleCommands.steering) {
      case VehicleCommands::steerLeft:
        m_steering -= m_steeringIncrement;
        if (	m_steering < -m_steeringClamp)
          m_steering = -m_steeringClamp;
        break;
      case VehicleCommands::steerRite:
        m_steering += m_steeringIncrement;
        if (	m_steering > m_steeringClamp)
          m_steering = m_steeringClamp;
        break;
      default: // steeredNone
        if(m_steering > m_steeringIncrement ||
            m_steering < -m_steeringIncrement )
          m_steering -= m_steering * .5;
        else
          m_steering = 0.;
        break;
    };
    //m_vehicleCommands.steering=VehicleCommands::steerNone;
#endif
  }

  // steps:
  // 1- update wheel wolrd space transforn 
  for(int i=0; i<4; i++) 
    updateWheelPhysics(i);

  // 2- update speed km/h
  if(m_speedometer) {
    m_forwardSpeed = 3.6 * getChassisForwardDirection().dot(m_carBody->getLinearVelocity());
    m_speedometer->setValue(m_forwardSpeed);
    //m_speedometer->setValue(btScalar(3.6) * m_carBody->getLinearVelocity().length());
  }

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
      btScalar force=0.;
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
      // @@@@
      wheel.suspensionForce = force * glob_chassisDefaultMass;
      if(wheel.suspensionForce < btScalar(0.)) {
        wheel.suspensionForce = btScalar(0.);
      }
      // @@@@

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
  updateFriction(deltaTime);

  // 5- update wheels rotation
	for (int i=0;i<4;i++)
	{
		WheelData& wheel = m_wheelsData[i];
		btVector3 relpos = wheel.hardPointWS - m_carBody->getCenterOfMassPosition();
		btVector3 vel = m_carBody->getVelocityInLocalPoint( relpos );

		if (wheel.isInContact)
		{
			const btTransform&	chassisWorldTransform = m_carBody->getCenterOfMassTransform();

			btVector3 fwd (
				chassisWorldTransform.getBasis()[0][0],
				chassisWorldTransform.getBasis()[1][0],
				chassisWorldTransform.getBasis()[2][0]);

			btScalar proj = fwd.dot(wheel.contactNormalWS);
			fwd -= wheel.contactNormalWS * proj;

			btScalar proj2 = fwd.dot(vel);
			
			wheel.deltaRotation = (proj2 * deltaTime) / (wheel.radius);

			wheel.rotation += wheel.deltaRotation;

		} else
		{
			wheel.rotation += wheel.deltaRotation;
		}
		
		wheel.deltaRotation *= btScalar(0.99);//damping of rotation when not in contact

	}

  extern bool stepMode;
  if(stepMode) {
    GM_LOG("body position: %f,%f,%f\n",
      m_carBody->getCenterOfMassTransform().getOrigin().getX(),
      m_carBody->getCenterOfMassTransform().getOrigin().getY(),
      m_carBody->getCenterOfMassTransform().getOrigin().getZ());
  }
}


void Vehicle::updateFriction(btScalar timeStep) 
{
  m_forwardWS.resize(4);
  m_axle.resize(4);
  m_forwardImpulse.resize(4);
  m_sideImpulse.resize(4);

  m_nWheelTouchGround=0;

  for (int i=0;i<4;i++)
  {
    WheelData& wheel = m_wheelsData[i];
    class btRigidBody* groundObject = (class btRigidBody*) wheel.collidingObject;
    if (groundObject)
      m_nWheelTouchGround++;
    m_sideImpulse[i] = btScalar(0.);
    m_forwardImpulse[i] = btScalar(0.);
  }

  // calc side impulse
  for (int i=0;i<4;i++) {

    WheelData& wheel = m_wheelsData[i];
    class btRigidBody* groundObject = (class btRigidBody*) wheel.collidingObject;

    if (groundObject) {

      const btTransform& wheelTrans = wheel.worldTransform;

      btMatrix3x3 wheelBasis0 = wheelTrans.getBasis();

      m_axle[i] = btVector3(	
          wheelBasis0[0][2],
          wheelBasis0[1][2],
          wheelBasis0[2][2]);

      const btVector3& surfNormalWS = wheel.contactNormalWS;
      btScalar proj = m_axle[i].dot(surfNormalWS);
      m_axle[i] -= surfNormalWS * proj;
      m_axle[i] = m_axle[i].normalize();

      m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
      m_forwardWS[i].normalize();

      g_resolveSingleBilateral(
          *m_carBody, wheel.contactPointWS,
          *groundObject, wheel.contactPointWS,
          btScalar(0.), m_axle[i],m_sideImpulse[i],timeStep);

      //m_sideImpulse[i] *=  sideFrictionStiffness2;

    }
  }

  // calc forward impulse
  btScalar sideFactor = btScalar(1.);
  btScalar fwdFactor = 0.5;
  bool sliding = false;
  for (int wheelIdx =0;wheelIdx <4;wheelIdx++) {
    WheelData& wheel = m_wheelsData[wheelIdx];
    class btRigidBody* groundObject = (class btRigidBody*) wheel.collidingObject;

    btScalar	rollingFriction = 0.f;

    if (groundObject) {
      if (m_throttle != 0.f) {
        rollingFriction = m_throttle* timeStep;
      } else {
        btScalar defaultRollingFrictionImpulse = 0.f;
        btScalar maxImpulse = m_brake ? m_brake : defaultRollingFrictionImpulse;
        WheelContactPoint contactPt(m_carBody,groundObject,wheel.contactPointWS,m_forwardWS[wheelIdx],maxImpulse);
        rollingFriction = calcRollingFriction(contactPt);
      }
    }

    //switch between active rolling (throttle), braking and non-active rolling friction (no throttle/break)
    m_forwardImpulse[wheelIdx] = btScalar(0.);
    m_wheelsData[wheelIdx].skidInfo= btScalar(1.);

    if (groundObject) {
      m_wheelsData[wheelIdx].skidInfo= btScalar(1.);

      btScalar maximp = wheel.suspensionForce * timeStep * wheel.frictionSlip;

      btScalar maximpSide = maximp;

      btScalar maximpSquared = maximp * maximpSide;

      m_forwardImpulse[wheelIdx] = rollingFriction;//wheelInfo.m_engineForce* timeStep;


      // qui ok GM_LOG("%d-->%f\n",wheelIdx,m_forwardImpulse[wheelIdx]);

      btScalar x = (m_forwardImpulse[wheelIdx] ) * fwdFactor;
      btScalar y = (m_sideImpulse[wheelIdx] ) * sideFactor;

      btScalar impulseSquared = (x*x + y*y);

      if (impulseSquared > maximpSquared) {
        sliding = true;

        btScalar factor = maximp / btSqrt(impulseSquared);

        m_wheelsData[wheelIdx].skidInfo *= factor;
      }
    } 
  }
  sliding=false;
  if (sliding)  {
    for (int wheel = 0;wheel < 4; wheel++) 
      if (m_sideImpulse[wheel] != btScalar(0.)) 
        if (m_wheelsData[wheel].skidInfo< btScalar(1.)) {
          // qui ok GM_LOG("%d-->%f skidinfo %f\n",wheel,m_forwardImpulse[wheel], m_wheelsData[wheel].skidInfo);
          m_forwardImpulse[wheel] *=	m_wheelsData[wheel].skidInfo;
          m_sideImpulse[wheel] *= m_wheelsData[wheel].skidInfo;
          // qui not ok GM_LOG("%d-->%f\n",wheel,m_forwardImpulse[wheel]);
        }
  }

  for (int wheel = 0;wheel<4 ; wheel++) {
    WheelData& wheelInfo = m_wheelsData[wheel];

    btVector3 rel_pos = wheelInfo.contactPointWS - 
      m_carBody->getCenterOfMassPosition();

    if (m_forwardImpulse[wheel] != btScalar(0.)) {
      // this is the 'engine' contribute to vehicle movement
      // (the one to make the car move)
      // NB 
      // the forward impulse is applied on the car body center of mass
      // and not on the wheels contact point  (stolen from tux kart)
      m_carBody->applyImpulse(m_forwardWS[wheel]*(m_forwardImpulse[wheel]), btVector3(0.,0.,0.) /*rel_pos*/);
    }
    if (m_sideImpulse[wheel] != btScalar(0.)) {
      class btRigidBody* groundObject = (class btRigidBody*) m_wheelsData[wheel].collidingObject;

      btVector3 rel_pos2 = wheelInfo.contactPointWS - 
        groundObject->getCenterOfMassPosition();

      // adjust relative position above ground so that force only acts sideways
      // (stolen from tux kart)
      btVector3 delta_vec = (wheelInfo.hardPointWS - wheelInfo.contactPointWS);
      if (delta_vec.length() != btScalar (0))
      {
        delta_vec = delta_vec.normalize();
        rel_pos -= delta_vec * rel_pos.dot(delta_vec);
      }

      // this is the 'friction' contribute to vehcile movement
      // (the one that __should__ not make the vehicle slide on turns)
      btVector3 sideImp = m_axle[wheel] * m_sideImpulse[wheel] ;

      rel_pos[1] *= m_rollInfluence;
      m_carBody->applyImpulse(sideImp,rel_pos);

      //apply friction impulse on the ground
      groundObject->applyImpulse(-sideImp,rel_pos2);
    }
  }
}

btScalar Vehicle::raycast(WheelData & wheel, int wnumber)
{
  // NB: raycast assume that wheel has been
  // already updated!!
	btScalar depth = -1.;
	btScalar raylen = wheel.suspensionLength + wheel.radius + m_maxSuspensionTravel;

	btVector3 rayvector = wheel.directionWS * (raylen);

	const btVector3& source = wheel.hardPointWS;

	wheel.contactPointWS = source + rayvector;
	const btVector3& target = wheel.contactPointWS;

	btVehicleRaycaster::btVehicleRaycasterResult	rayResults;

	btScalar param = btScalar(0.);

	void* object = m_raycaster->castRay(source,target,rayResults);

#if 0
  if(wnumber == 0) {
    GM_LOG("%s ",object?"    in contact":"not in contact");
    GM_LOG("contronus binosus: %f,%f,%f  -> ",source.getX(),source.getY(),source.getZ());
    GM_LOG(" %f,%f,%f\n",target.getX(),target.getY(),target.getZ());
  }
#endif


  if(object) {

    wheel.collidingObject=&getFixedBody();

    wheel.isInContact=true;

		param = rayResults.m_distFraction;
		depth = raylen * rayResults.m_distFraction;

		wheel.contactNormalWS = rayResults.m_hitNormalInWorld;

		btScalar hitDistance = param*raylen;
		wheel.suspensionLength = hitDistance - wheel.radius;

		btScalar minSuspensionLength = m_suspensionRestLength - m_maxSuspensionTravel;
		btScalar maxSuspensionLength = m_suspensionRestLength + m_maxSuspensionTravel;
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

void Vehicle::setDebugDrawFlags(unsigned flags)    
{
  m_debugDrawFlags=flags;
}

void Vehicle::debugDraw(btIDebugDraw* debugDrawer)
{
  btVector3 color(1.,1.,1.);

  for(int i=0; i<4; i++) {
    WheelData & wheel=m_wheelsData[i];

    btVector3  point1 = wheel.hardPointWS;
    btVector3  point2;

    if(m_debugDrawFlags & db_wheelsAxle)  {
      if(isLeftWheel(i)) {
        point2 = point1 + wheel.axleWS * 5.;
      } else {
        point2 = point1 - wheel.axleWS * 5.;
      }
      debugDrawer->drawLine(point1,point2,color);
    }

    if(m_debugDrawFlags & db_raycastDirection) {
      point2 = point1 + wheel.directionWS * 5.;
      point1 -= wheel.directionWS * 5.;
      debugDrawer->drawLine(point1,point2,color);
    }

    if((m_debugDrawFlags & db_forwardImpulse) && i<m_forwardImpulse.size()) {
      point1 = wheel.hardPointWS;
      point2 = point1 + m_forwardImpulse[i] * m_forwardWS[i] * 100.;
      debugDrawer->drawLine(point1,point2,color);
    }

    if((m_debugDrawFlags & db_sideImpulse) && i<m_sideImpulse.size()) {
      point1 = wheel.hardPointWS;
      point2 = point1 + m_sideImpulse[i] * m_axle[i] * 100.;
      debugDrawer->drawLine(point1,point2,color);
    }

    if(m_debugDrawFlags & db_suspensions && wheel.isInContact) {
      btScalar raylen = wheel.suspensionLength + wheel.radius;
      btVector3 rayvector = wheel.directionWS * (raylen);
      const btVector3& source = wheel.hardPointWS;
      const btVector3& target = source + rayvector; //.contactPointWS;
      debugDrawer->drawLine(source,target,color);
    }

    debugDrawer->drawLine(m_carBody->getCenterOfMassPosition(),wheel.hardPointWS,color);
  }

}

void 	Vehicle::getWorldTransform (btTransform &worldTrans) const
{
   worldTrans=m_chassisWorldTrans;
}

void Vehicle::setWorldTransform (const btTransform &worldTrans)
{
   m_chassisWorldTrans=worldTrans;
   step();
}

void Vehicle::step()
{
  if(m_carBody==0)
    return;

	btTransform trans;
  assert(m_carBody);
  
  trans= m_chassisWorldTrans; //m_carBody->getCenterOfMassTransform();

  irr::core::matrix4 matr;
  PhyWorld::btTransformToIrrlichtMatrix(trans, matr);

  m_chassisNode->setPosition(matr.getTranslation());
  m_chassisNode->setRotation(matr.getRotationDegrees());

  for(int i=0; i<4; i++) updateWheel(i);
}

void Vehicle::updateWheelPhysics(int index)
{
  WheelData & wheel=m_wheelsData[index];

  wheel.isInContact=false;
  btTransform chassisTrans = m_carBody->getCenterOfMassTransform();

  wheel.hardPointWS = chassisTrans * wheel.hardPointCS; 
  wheel.directionWS = chassisTrans.getBasis() * btVector3(0.,-1.,0.);
  wheel.axleWS = chassisTrans.getBasis() *  btVector3(0.,0.,1.);

  btVector3 up = -wheel.directionWS;
  const btVector3& right = wheel.axleWS;
  btVector3 fwd = up.cross(right);
  btScalar steering = btScalar(0.);
  if(isFrontWheel(index))
    steering=m_steering;

  btQuaternion steeringOrn(up,steering);
  btMatrix3x3 steeringMat(steeringOrn);

  btQuaternion rotatingOrn(right,-wheel.rotation);
  btMatrix3x3 rotatingMat(rotatingOrn);

  wheel.worldTransform.setBasis(steeringMat * rotatingMat * chassisTrans.getBasis());
  wheel.worldTransform.setOrigin( wheel.hardPointWS + wheel.directionWS * wheel.suspensionLength /*m_suspensionRestLength*/);
}

void Vehicle::updateWheel(int index)
{
  assert(index>=0 && index<4);

  irr::scene::ISceneNode * wheel=m_wheelsNodes[index];
  if(!wheel)
    return;

  assert(wheel);

  btTransform & wheelTrans=m_wheelsData[index].worldTransform;

  irr::core::matrix4 matr;
  PhyWorld::btTransformToIrrlichtMatrix(wheelTrans, matr);

  wheel->setPosition(matr.getTranslation());
  wheel->setRotation(matr.getRotationDegrees());

  extern bool stepMode;
  if(stepMode) {
    WheelData & wheel=m_wheelsData[index];
    GM_LOG("wheel: %d Y: %f\n",index,matr.getTranslation().X);
    GM_LOG("susp.: %f, radius: %f, pos: %f, tot: %f\n",
        wheel.suspensionLength,
        wheel.radius, m_wheelInitialPositions[index].Y,
        wheel.radius + m_wheelInitialPositions[index].Y +
        wheel.suspensionLength);


  }
}

void Vehicle::setSpeedOMeter(INumberOutput * speedometer)
{
  m_speedometer=speedometer;
}


#if 0
double Vehicle::getStartHeight(float x, float z)
{
  double startHeight=20.;
  double endHeight=-20.;
  double height[4];
  double totHeight;

  for(int i; i<4; i++) {
    WheelData & wheel=m_wheelsData[i];

    btVector3 source = btVector3(x,startHeight,z) + btVector3(
        m_wheelInitialPositions[i].X,
        m_wheelInitialPositions[i].Y,
        m_wheelInitialPositions[i].Z);


    btVector3 target = btVector3(x,endHeight,z) + btVector3(
        m_wheelInitialPositions[i].X,
        m_wheelInitialPositions[i].Y,
        m_wheelInitialPositions[i].Z);

    btVehicleRaycaster::btVehicleRaycasterResult	rayResults;

    m_raycaster->castRay(source,target,rayResults);
    
    height[i] = endHeight - (startHeight - endHeight) * rayResults.m_distFraction;

    GM_LOG("wheel %d cast ground at %f\n",i,rayResults.m_distFraction);

    height[i] += wheel.radius;

    totHeight += height[i];
  }

  return totHeight / 4.;
}
#endif

void Vehicle::setEnableControls(bool enable)
{
  m_controlsEnabled=enable;
}

btRigidBody * Vehicle::getRigidBody()
{
  return m_carBody;
}

btVector3 Vehicle::getChassisForwardDirection()
{
  btTransform chassisTrans = m_carBody->getCenterOfMassTransform();
  return chassisTrans.getBasis() * btVector3(1.,0.,0.);
}

btVector3 Vehicle::getChassisRightDirection()
{
  btTransform chassisTrans = m_carBody->getCenterOfMassTransform();
  return chassisTrans.getBasis() * btVector3(0.,0.,1.);
}

btVector3 Vehicle::getChassisUpDirection()
{
  btTransform chassisTrans = m_carBody->getCenterOfMassTransform();
  return chassisTrans.getBasis() * btVector3(0.,1.,0.);
}

double Vehicle::getRestHeight(/*float x, float y*/)
{
  return 
    m_wheelsData[0].radius - m_wheelInitialPositions[0].Y;
}

bool Vehicle::getIfChassisIsTouchingTheGround()
{
  return m_nWheelTouchGround!=0;
}

double Vehicle::getSpeed()
{
  return m_forwardSpeed;
}


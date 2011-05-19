#include <btBulletDynamicsCommon.h>
#include "TestVehicle.h"

#define DIM_X 1.f
#define DIM_Y .5f
#define DIM_Z 2.f

#define CUBE_HALF_EXTENTS 1.


static float	wheelRadius = 0.5f;
static float  wheelWidth = 0.4f;

TestVehicle::TestVehicle(irr::scene::ISceneNode * parent, irr::scene::ISceneManager * smgr, irr::s32 id)
  : IVehicle(parent, smgr,id)
{
#if 0
	btCollisionShape* chassisShape = new btBoxShape(btVector3(DIM_X,DIM_Y,DIM_Z));
	//m_collisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	//m_collisionShapes.push_back(compound);
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(0,1,0));

	compound->addChildShape(localTrans,chassisShape);

  btTransform tr;
	tr.setOrigin(btVector3(0,0.f,0));

	//m_carChassis = localCreateRigidBody(800,tr,compound);//chassisShape);
	//m_carChassis->setDamping(0.2,0.2);
	
	m_wheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));

  m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_dynamicsWorld);
  m_vehicle = new btRaycastVehicle(m_tuning,m_carChassis,m_vehicleRayCaster);
		
		///never deactivate the vehicle
  m_carChassis->setActivationState(DISABLE_DEACTIVATION);

  m_dynamicsWorld->addVehicle(m_vehicle);

  float connectionHeight = 1.2f;

	
  bool isFrontWheel=true;

		//choose coordinate system
  m_vehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

	btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
	m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

  connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
  m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

  connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);

  isFrontWheel = false;
  m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);

  connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
  m_vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		
  for (int i=0;i<m_vehicle->getNumWheels();i++)
  {
    btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
    wheel.m_suspensionStiffness = suspensionStiffness;
    wheel.m_wheelsDampingRelaxation = suspensionDamping;
    wheel.m_wheelsDampingCompression = suspensionCompression;
    wheel.m_frictionSlip = wheelFriction;
    wheel.m_rollInfluence = rollInfluence;
  }
#endif
}


void TestVehicle::load()
{
}

void TestVehicle::unload()
{
}

void TestVehicle::use(unsigned int useFlags)
{
}

void TestVehicle::unuse(unsigned int useFlags)
{
}

void TestVehicle::reset(const irr::core::vector3d<float>&pos)
{
}

// querying
irr::core::vector3df TestVehicle::getChassisPos() {
  return irr::core::vector3df(0,0,0);
}

// commands
void TestVehicle::throttleUp()
{
}

void TestVehicle::throttleDown()
{
}

void TestVehicle::throttleSet(double value)
{
}

void TestVehicle::brake()
{
}

void TestVehicle::steerLeft()
{
}

void TestVehicle::steerRight()
{
}

// phisics
void TestVehicle::applyTorque(float x, float y, float z)
{
}

// debug
void TestVehicle::dumpDebugInfo()
{
}

// the speedometer
void TestVehicle::setSpeedOMeter(INumberOutput * speedometer)
{
}

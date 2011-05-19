#ifndef TEST_VEHICLE_H
#define TEST_VEHICLE_H
#include "IVehicle.h"

class TestVehicle : public IVehicle
{
  public:

    TestVehicle(irr::scene::ISceneNode * parent, irr::scene::ISceneManager * smgr, irr::s32 id=-1);

    virtual void load();

    virtual void unload();

    virtual void use(unsigned int useFlags);;

    virtual void unuse(unsigned int useFlags);;

    virtual void reset(const irr::core::vector3d<float>&pos);

    // querying
    virtual irr::core::vector3df getChassisPos();

    // commands
    virtual void throttleUp();
    virtual void throttleDown();
    virtual void throttleSet(double value);
    virtual void brake();


    virtual void steerLeft();
    virtual void steerRight();

    // phisics
    virtual void applyTorque(float x, float y, float z);

    // debug
    virtual void dumpDebugInfo();

    // the speedometer
    virtual void setSpeedOMeter(INumberOutput * speedometer);
  private:
    btRaycastVehicle::btVehicleTuning	m_tuning;
    btVehicleRaycaster*	m_vehicleRayCaster;
    btRaycastVehicle*	m_vehicle;
    btCollisionShape*	m_wheelShape;
    btRigidBody* m_carChassis;
};


#endif

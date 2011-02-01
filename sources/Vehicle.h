#ifndef VEHICLE_H
#define VEHICLE_H

#include <irrlicht.h>
#include "PhyWorld.h"

class Vehicle 
{
  public:
    Vehicle(
        irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * source);

    void load();

  private:
    enum {
      MAX_NUM_OF_WHEELS=4
    };

    const char * m_sourceName;

    int          m_numWheels;

    PhyWorld *   m_world;

    bool         m_loaded;

    irr::IrrlichtDevice *
                 m_device;

    irr::io::IFileSystem * 
                 m_filesystem;
    
    irr::core::array<irr::scene::IAnimatedMesh*>   
                         m_chassis;

    irr::core::array<irr::scene::IAnimatedMesh*>
      m_wheel_rl;

    irr::core::array<irr::scene::IAnimatedMesh*>
      m_wheel_rr;

    irr::core::array<irr::scene::IAnimatedMesh*>
      m_wheel_fl;

    irr::core::array<irr::scene::IAnimatedMesh*>
      m_wheel_fr;
};

#endif

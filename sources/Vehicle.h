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

  private:
    enum {
      MAX_NUM_OF_WHEELS=4
    };
    const char * m_sourceName;
    int          m_numWheels;
    PhyWorld *   m_world;
    irr::IrrlichtDevice *
                 m_device;
    
    irr::core::array<irr::scene::IAnimatedMeshSceneNode*>   
                         m_chassisNodes;
};

#endif

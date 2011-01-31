#include "Vehicle.h"


Vehicle::Vehicle(
        irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * source)
{
   m_device=device;
   m_world=world;
   m_sourceName=source;
}

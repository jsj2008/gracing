//  gracing - a idiot (but physically powered) racing game 
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

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
#ifndef CAMERADATAMANAGER_H
#define CAMERADATAMANAGER_H
#include <irrlicht.h>
#include "util.hh"

class CameraDataManager 
{
  public:
    CameraDataManager (irr::io::IReadFile * file);

    inline void getPositionAndRotation(irr::core::vector3df & position,
                 irr::core::vector3df & rotation)
    {
      position.X=pos[0];
      position.Y=pos[1];
      position.Z=pos[2];

      rotation.X=rad2deg(rot[0]);
      rotation.Y=rad2deg(rot[1]);
      rotation.Z=rad2deg(rot[2]);
    }

  private:
    double pos[3];
    double rot[3];
};
#endif 

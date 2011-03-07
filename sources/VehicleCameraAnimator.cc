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
#include <assert.h>
#include "VehicleCameraAnimator.h"

VehicleCameraAnimator::VehicleCameraAnimator(IVehicle * vehicle)
{
  m_vehicle=vehicle;
}

void 	VehicleCameraAnimator::animateNode (irr::scene::ISceneNode *node, irr::u32 timeMs)
{
  assert(node->getType() == irr::scene::ESNT_CAMERA);

  irr::scene::ICameraSceneNode * cam=(irr::scene::ICameraSceneNode*)node;

  irr::core::vector3df pos; // m_vehicle->getChassisPos


  cam->setTarget(pos);
  cam->setPosition(pos);
#ifdef TRENTA
  irr::core::vector3df real_pos;
  real_pos=which_car->get_position();
  const irr::core::CMatrix4<irr::f32> & rot=which_car->get_matrix();
  irr::scene::ICameraSceneNode * camera=cameras[which_camera];
  if(camera_type[which_camera]) {
    camera->setTarget(real_pos);
    real_pos.X+=camera_pos[which_camera][0]*camera_distance[which_camera];
    real_pos.Y+=camera_pos[which_camera][1]*camera_distance[which_camera];
    real_pos.Z+=camera_pos[which_camera][2]*camera_distance[which_camera];
    camera->setPosition(real_pos);
  } else {
    irr::core::vector3df target;
    target.X=real_pos.X;
    target.Y=real_pos.Y+2.;
    target.Z=real_pos.Z;
    camera->setTarget(target);
    real_pos.X+=fp_camera_pos[0]*rot[0];//-20.*rot[0];
    real_pos.Y+=fp_camera_pos[1];   // 10.;
    real_pos.Z+=fp_camera_pos[2]*rot[8]; //20.*rot[8];
    camera->setPosition(real_pos);
  }
#endif
}

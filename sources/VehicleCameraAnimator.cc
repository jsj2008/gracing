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
#include "gmlog.h"
#include "VehicleCameraAnimator.h"

VehicleCameraAnimator::VehicleCameraAnimator(IVehicle * vehicle)
{
  m_vehicle=vehicle;
  m_cameraPos=irr::core::vector3df(10,2,10);
  m_cameraDistance=1.;
}

void 	VehicleCameraAnimator::animateNode (irr::scene::ISceneNode *node, irr::u32 timeMs)
{
  assert(node->getType() == irr::scene::ESNT_CAMERA);

  irr::scene::ICameraSceneNode * cam=(irr::scene::ICameraSceneNode*)node;

  irr::core::vector3df pos=m_vehicle->getChassisPos();

  cam->setTarget(pos);

  pos+=m_cameraPos*m_cameraDistance;
  cam->setPosition(pos);
}

irr::scene::ISceneNodeAnimator * VehicleCameraAnimator::createClone(irr::scene::ISceneNode *node, 
        irr::scene::ISceneManager *newManager)
{
  return 0;
}

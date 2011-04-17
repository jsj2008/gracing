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
#include "config.h"

CFG_PARAM_D(glob_type0cam_phi)=0.f;
CFG_PARAM_D(glob_type0cam_distance)=5.f;
CFG_PARAM_D(glob_type0cam_height)=5.f;

VehicleCameraAnimator::VehicleCameraAnimator(IVehicle * vehicle)
{
  m_vehicle=vehicle;
  m_camType=0;

  // type 0 init
  m_type0_parms.phi=glob_type0cam_phi;
  m_type0_parms.distance=glob_type0cam_distance;
  m_type0_parms.height=glob_type0cam_height;

  type0_updateDerivate();
}

void 	VehicleCameraAnimator::animateNode (irr::scene::ISceneNode *node, irr::u32 timeMs)
{
  assert(node->getType() == irr::scene::ESNT_CAMERA);

  switch(m_camType) 
  {
    case 0:
    default:
      irr::scene::ICameraSceneNode * cam=(irr::scene::ICameraSceneNode*)node;
      irr::core::vector3df pos=m_vehicle->getChassisPos();
      cam->setTarget(pos);
      pos+=m_type0_parms.pos*m_type0_parms.distance;
      cam->setPosition(pos);
      break;
  }
}

irr::scene::ISceneNodeAnimator * VehicleCameraAnimator::createClone(irr::scene::ISceneNode *node, 
        irr::scene::ISceneManager *newManager)
{
  return 0;
} 

void VehicleCameraAnimator::moveXY(float dx, float dy)
{
  switch(m_camType)
  {
    case 0:
    default:
      type0_moveXY(dx,dy);
      break;
  }
}

void VehicleCameraAnimator::type0_moveXY(float dx, float dy)
{
  m_type0_parms.phi+=dx;
  m_type0_parms.height+=dy;

  if(m_type0_parms.phi<=0.)
    m_type0_parms.phi=2.*M_PI;
  else if(m_type0_parms.phi>=2.*M_PI)
    m_type0_parms.phi=0.;

  type0_updateDerivate();
}

void VehicleCameraAnimator::type0_updateDerivate()
{
  double sphi=sin(m_type0_parms.phi);
  double cphi=cos(m_type0_parms.phi);
   
  m_type0_parms.pos.X=m_type0_parms.distance * sphi;
  m_type0_parms.pos.Y=m_type0_parms.height;
  m_type0_parms.pos.Z=m_type0_parms.distance * cphi;
}

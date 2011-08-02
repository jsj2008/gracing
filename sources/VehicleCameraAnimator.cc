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
CFG_PARAM_D(glob_type0cam_distance)=3.f;
CFG_PARAM_D(glob_type0cam_height)=2.f;


class BirdFlyCamAnimator : public ICamAnimator
{
  public:

    BirdFlyCamAnimator()
      : m_up(1.,0.,0.)
    {
    }

    virtual void doit(const irr::core::vector3df & pos,
        const irr::core::vector3df & forward,
        irr::core::vector3df & camPosition,
        irr::core::vector3df & camTarget,
        irr::core::vector3df & camUpDir)
    {
      camTarget = pos;
      camPosition = pos;
      camPosition.Y += 10.;
      camUpDir = m_up;
    }
    virtual void move(float,float) { }

  private:
    irr::core::vector3df m_up;
};

class FirstPersonCamAnimator : public ICamAnimator
{
  public:
    virtual void doit(const irr::core::vector3df & position,
        const irr::core::vector3df & forward,
        irr::core::vector3df & camPosition,
        irr::core::vector3df & camTarget,
        irr::core::vector3df & camUpDir)
    {
      double oo1=1.1;
      double oo2=.9;
      double aa=3.;

      irr::core::vector3df direction = irr::core::vector3df(forward.X,forward.Y+oo1,forward.Z);
      camTarget = position + direction;

      camPosition = position;
      camPosition.X -= direction.X * aa; 
      camPosition.Y += oo2;
      camPosition.Z -= direction.Z * aa; 
      camUpDir=irr::core::vector3df(0.,1.,0);
    }
    virtual void move(float,float) { }
};

class ThirdPersonCamAnimator : public ICamAnimator
{
  public:
    ThirdPersonCamAnimator()
    {
      m_phi=glob_type0cam_phi;
      m_distance=glob_type0cam_distance;
      m_height=glob_type0cam_height;
      update();
    }

    virtual void doit(const irr::core::vector3df & pos,
        const irr::core::vector3df & forward,
        irr::core::vector3df & camPosition,
        irr::core::vector3df & camTarget,
        irr::core::vector3df & camUpDir)
    {
      camTarget = pos;
      camPosition = pos + m_pos * m_distance;
      camUpDir=irr::core::vector3df(0.,1.,0);
    }

    virtual void move(float dx,float dy)
    {
      m_phi+=dx;
      m_height+=dy;

      if(m_phi<=0.)
        m_phi=2.*M_PI;
      else if(m_phi>=2.*M_PI)
        m_phi=0.;

      update();
    }

  private:
    void update() {
      double sphi=sin(m_phi);
      double cphi=cos(m_phi);

      m_pos.X=m_distance * sphi;
      m_pos.Y=m_height;
      m_pos.Z=m_distance * cphi;
    }
    float                m_distance;
    float                m_phi;
    float                m_height;

    // derivate
    irr::core::vector3df m_pos;


};

VehicleCameraAnimator::VehicleCameraAnimator(IVehicle * vehicle)
{
  m_vehicle=vehicle;
  m_camType=ct_tuxracer;

  m_animators[0]=new ThirdPersonCamAnimator();
  m_animators[1]=new FirstPersonCamAnimator();
  m_animators[2]=new BirdFlyCamAnimator();
  // type micromachine init
#if 0
  m_type0_parms.phi=glob_type0cam_phi;
  m_type0_parms.distance=glob_type0cam_distance;
  m_type0_parms.height=glob_type0cam_height;
  type0_updateDerivate();

  setCameraType(1);
#endif
}

void VehicleCameraAnimator::setCameraType(unsigned camType)
{

  m_camType=camType % 3;
#if 0
  if(camType == 1) {
    m_camType = ct_tuxracer;
  } else {
    m_camType = ct_micromachine;
  }
  init(m_camType);
#endif
}

void VehicleCameraAnimator::changeVehicle(IVehicle * vehicle)
{
  m_vehicle=vehicle;
}


void 	VehicleCameraAnimator::animateNode (irr::scene::ISceneNode *node, irr::u32 timeMs)
{
  ICamAnimator * camAnim = m_animators[m_camType];

  assert(node->getType() == irr::scene::ESNT_CAMERA);
  irr::scene::ICameraSceneNode * cam=(irr::scene::ICameraSceneNode*)node;

  irr::core::vector3df pos=m_vehicle->getChassisPos();

  btVector3 vd = m_vehicle->getChassisForwardDirection();
  irr::core::vector3df forward = irr::core::vector3df(vd.getX(),vd.getY(),vd.getZ());

  camAnim->doit(pos,forward, m_camPosition, m_camTarget,m_camUpDir);

  cam->setPosition(m_camPosition);
  cam->setTarget(m_camTarget);
  cam->setUpVector(m_camUpDir);

#if 0
  assert(node->getType() == irr::scene::ESNT_CAMERA);
  irr::core::vector3df pos=m_vehicle->getChassisPos();

  irr::scene::ICameraSceneNode * cam=(irr::scene::ICameraSceneNode*)node;
  
  irr::core::vector3df tgt = pos;

  switch(m_camType) 
  {
    case 1:
      {
        double oo1=1.1;
        double oo2=.9;
        double aa=3.;

        btVector3 vd = m_vehicle->getChassisForwardDirection();
        irr::core::vector3df direction = irr::core::vector3df(vd.getX(),vd.getY()+oo1,vd.getZ());
        cam->setTarget(pos + direction);
        pos.X -= direction.X * aa; //pos.Y += oo;
        pos.Y += oo2;
        pos.Z -= direction.Z * aa; //pos.Y += oo;
        cam->setPosition(pos);
        cam->setUpVector(irr::core::vector3df(0.,1.,0));
      }
      break;
    case 2:
      cam->setTarget(pos);
      pos.Y += 10.;
      cam->setUpVector(m_type1_parms.up);
      cam->setPosition(pos);
      break;
    case 0:
    default:
      cam->setTarget(pos);
      pos+=m_type0_parms.pos*m_type0_parms.distance;
      cam->setPosition(pos);
      cam->setUpVector(irr::core::vector3df(0.,1.,0));
      break;
  }
#endif
}

irr::scene::ISceneNodeAnimator * VehicleCameraAnimator::createClone(irr::scene::ISceneNode *node, 
        irr::scene::ISceneManager *newManager)
{
  return 0;
} 

void VehicleCameraAnimator::type1_init()
{
  btVector3 vd = m_vehicle->getChassisForwardDirection();
  irr::core::vector3df direction = irr::core::vector3df(vd.getX(),vd.getY(),vd.getZ());
  m_type1_parms.up=direction;
}

void VehicleCameraAnimator::type0_init()
{

}

void VehicleCameraAnimator::init(unsigned camType) 
{
  switch(camType) {
    case 1:
      m_camType = 1;
      type1_init();
      break;
    default:
    case 0:
      m_camType = 0;
      type0_init();
      break;
  }
   
}

void VehicleCameraAnimator::moveXY(float dx, float dy)
{
  switch(m_camType)
  {
    case 1:
      type1_moveXY(dx,dy);
      break;
    case 0:
    default:
      type0_moveXY(dx,dy);
      break;
  }
}

void VehicleCameraAnimator::type1_moveXY(float dx, float dy)
{
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

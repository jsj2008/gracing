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
    FirstPersonCamAnimator()
    {
      m_lookingBackward=false;
      m_lookingHeight=.9;
    }

    virtual void doit(const irr::core::vector3df & position,
        const irr::core::vector3df & forward,
        irr::core::vector3df & camPosition,
        irr::core::vector3df & camTarget,
        irr::core::vector3df & camUpDir)
    {
      double oo1=1.1;
      //double oo2=.9;
      double aa=3.;

      irr::core::vector3df direction = irr::core::vector3df(forward.X,forward.Y+oo1,forward.Z);

      if(m_lookingBackward) {
        camPosition = position + direction;
        camTarget = position;
        camTarget.X -= direction.X * aa; 
        camTarget.Y += m_lookingHeight;
        camTarget.Z -= direction.Z * aa; 
        camUpDir=irr::core::vector3df(0.,1.,0);
        m_lookingBackward = false;
      } else {
        camTarget = position + direction;
        camPosition = position;
        camPosition.X -= direction.X * aa; 
        camPosition.Y += m_lookingHeight;
        camPosition.Z -= direction.Z * aa; 
        camUpDir=irr::core::vector3df(0.,1.,0);
      }
    }
    virtual void move(float dx,float dy) 
    { 
      if(dx != 0.) 
        m_lookingBackward = true;
      m_lookingHeight += dy;
      if(m_lookingHeight < .3) 
        m_lookingHeight = .3;
      else if(m_lookingHeight > 1.5)
        m_lookingHeight = 1.5;
    }

   private:
    bool     m_lookingBackward;
    double   m_lookingHeight;
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

VehicleCameraAnimator::~VehicleCameraAnimator()
{
  delete m_transiction;
  delete m_animators[0];
  delete m_animators[1];
  delete m_animators[2];
}

VehicleCameraAnimator::VehicleCameraAnimator(IVehicle * vehicle)
{
  m_vehicle=vehicle;
  m_camType=ct_tuxracer;

  m_animators[0]=new ThirdPersonCamAnimator();
  m_animators[1]=new FirstPersonCamAnimator();
  m_animators[2]=new BirdFlyCamAnimator();

  m_changingCamera=false;
  //m_transiction=new TweenBounceOut<double>();
  m_transiction=new TweenQuadIn<double>();
}

void VehicleCameraAnimator::setCameraType(unsigned camType)
{

  GM_LOG("suha\n");
  if(m_changingCamera)
    return;

  camType %= 3;

  if(m_transiction) {
    m_oldCamType=m_camType;
    m_camType=camType;

    m_transiction->init(0.,0.,1.);
    m_transictionTime=0.;
    m_transictionStep= .5 /  15; // TODO: request resource manager the framerare
    m_changingCamera=true;
  } else {
    m_oldCamType=m_camType;
    m_camType=camType % 3;
  }
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

  if(m_changingCamera) {
    assert(m_transiction);

    irr::core::vector3df ccpos,cctar,ccup;
    camAnim->doit(pos,forward, m_camPosition, m_camTarget,m_camUpDir);
    camAnim = m_animators[m_oldCamType];
    camAnim->doit(pos,forward, ccpos, cctar,ccup);
  

    double t,t1;
    m_changingCamera=!m_transiction->doit(m_transictionTime, t);

    t1=1.-t;
    m_transictionTime += m_transictionStep;
    m_camPosition= t1 * ccpos + t * m_camPosition;
    m_camTarget= t1* cctar + t * m_camTarget;
    m_camUpDir= t1 * ccup + t * m_camUpDir;
  } else {
    camAnim->doit(pos,forward, m_camPosition, m_camTarget,m_camUpDir);
  }

  cam->setPosition(m_camPosition);
  cam->setTarget(m_camTarget);
  cam->setUpVector(m_camUpDir);
}

irr::scene::ISceneNodeAnimator * VehicleCameraAnimator::createClone(irr::scene::ISceneNode *node, 
        irr::scene::ISceneManager *newManager)
{
  return 0;
} 

void VehicleCameraAnimator::moveXY(float dx, float dy)
{
  m_animators[m_camType]->move(dx,dy);
}


#if 0
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
#endif


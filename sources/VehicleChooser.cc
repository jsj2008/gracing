//  gracing - an idiot (but physically powered) racing game 
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

#include "VehicleChooser.h"
#include "ResourceManager.h"
#include "gmlog.h"

struct voidTrans : public VehicleChooser::iTransiction
{
  virtual void init(double t, const irr::core::vector3df & startPos, const irr::core::vector3df & endPos)
  {
  
  }

  virtual bool doit(double t, irr::core::vector3df & position, double  & rotation)
  {
    return false;

  }
} g_voidTrans;

struct rotateTrans : public VehicleChooser::iTransiction
{
  virtual void init(double t, const irr::core::vector3df & startPos, const irr::core::vector3df & endPos)
  {
    // nothing to do here
  }

  virtual bool doit(double t, irr::core::vector3df & position, double  & rotation)
  {
    rotation = t;
    return false;
  }
};

struct linearTrans : public VehicleChooser::iTransiction
{
  virtual void init(double t, const irr::core::vector3df & sPos, const irr::core::vector3df & ePos)
  {
    startTime=t;
    start=sPos;
    end=ePos;
    rotation=0.;
  }

  virtual bool doit(double t, irr::core::vector3df & position, double  & rot)
  {
    double tf=t-startTime;
    position=start + (end - start) * tf;
    rot=t;

    if(tf >= 1.)
      return true;
    return false;
  }

  private:
    double startTime;
    double rotation;
    irr::core::vector3df start,end;
};

VehicleChooser::VehicleChooser(irr::IrrlichtDevice * device,
        PhyWorld * world)
  : IPhaseHandler(device,world)
{

  double o=3.;
  m_pos0= irr::core::vector3df(0.,0.,0);
  m_pos1= irr::core::vector3df(0.,0.,o);
  m_pos2= irr::core::vector3df(0,0.,-o);

  m_transictions[trans_stay]=new rotateTrans;
  m_transictions[trans_enter]=new linearTrans();
  m_transictions[trans_exit]=new linearTrans();
}

void VehicleChooser::prepare()
{
  const std::vector<IVehicle*> & vehicles=
    ResourceManager::getInstance()->getVehiclesList();

  m_currentVehicle=0;
  m_maxVehicles=vehicles.size();
  m_vehicle=0;

  changeVehicle();

  // should be loaded from 
  // xml node

  m_camera = m_device->getSceneManager()->addCameraSceneNode();
  m_camera->setPosition(irr::core::vector3df(3.,1.,0.));
  m_camera->setTarget(irr::core::vector3df(2.5,1.,0.));

  irr::video::SColor   ambient_color = irr::video::SColor(255, 120, 120, 120);
  irr::video::SColor   sun_specular_color = irr::video::SColor(255, 255, 255, 255);
  irr::video::SColor   sun_diffuse_color = irr::video::SColor(255, 255, 255, 255); 
  irr::core::vector3df sun_position  = irr::core::vector3df(0, 0, 0);

  m_sun = m_device->getSceneManager()->addLightSceneNode(NULL, 
      sun_position,
      sun_diffuse_color);
  m_sun->setLightType(irr::video::ELT_DIRECTIONAL);
  m_sun->setRotation( irr::core::vector3df(180, 45, 45) );
  m_sun->getLightData().SpecularColor = sun_specular_color;
}


void VehicleChooser::changeVehicle()
{
  const std::vector<IVehicle*> & vehicles=
    ResourceManager::getInstance()->getVehiclesList();

  if(m_vehicle) {
    m_vehicle->unuse(IVehicle::USE_GRAPHICS);
    m_currentVehicle++;
    if(m_currentVehicle == vehicles.size())
      m_currentVehicle = 0;
  }


  m_vehicle=vehicles[m_currentVehicle];
  m_vehicle->use(IVehicle::USE_GRAPHICS);
  //m_vehicle->reset(irr::core::vector3df(0.,0.,0.),0.);
  m_status = status_vehicle_entering;
  m_nextStatus = status_uninited;
  m_transictions[trans_enter]->init(0., m_pos1, m_pos0);
  m_transictionTime = 0.;
  m_timeStep = 1. / 80.; // get the frame rate from the resource manager
}

void VehicleChooser::step()
{
  EventReceiver * erec;

  erec=ResourceManager::getInstance()->getEventReceiver();

  switch(m_status) {

    case status_vehicle_entering:
      if(!m_transictions[trans_enter]->doit(m_transictionTime,m_vPosition,m_vRotation)) {
        break;
      }
      m_nextStatus=status_vehicle_staying;
      GM_LOG("next status: staying\n");
      break;

    case status_vehicle_staying:
      m_transictions[trans_stay]->doit(m_transictionTime,m_vPosition,m_vRotation);
      if(erec->OneShotKey(irr::KEY_LEFT))  {
        m_nextStatus = status_vehicle_exiting;
        m_transictions[trans_exit]->init(m_transictionTime, m_pos0, m_pos2);
        GM_LOG("next status: exiting\n");
      }
      break;

    case status_vehicle_exiting:
      if(!m_transictions[trans_exit]->doit(m_transictionTime,m_vPosition,m_vRotation)) {
        break;
      }

      m_nextStatus=status_vehicle_entering;
      m_transictions[trans_enter]->init(m_transictionTime, m_pos1, m_pos0);

      break;

        
    default:
      if(m_transictions[trans_stay])
        m_transictions[trans_stay]->doit(m_transictionTime,m_vPosition,m_vRotation);
      break;
  }

  m_transictionTime += m_timeStep;

  m_vehicle->reset(m_vPosition,m_vRotation);

  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  m_sceneManager->drawAll();
  m_guiEnv->drawAll();
  m_world->debugDrawWorld();
  m_driver->endScene();
  if(m_nextStatus != status_uninited) {
    m_status = m_nextStatus;
    m_nextStatus = status_uninited;

    if(m_status == status_vehicle_entering) {
      changeVehicle();
    }
  }
}

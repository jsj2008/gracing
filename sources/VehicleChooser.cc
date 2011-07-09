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

#include <assert.h>

#include "VehicleChooser.h"
#include "ResourceManager.h"
#include "gmlog.h"
#include "util.hh"


VehicleChooser::VehicleChooser(irr::IrrlichtDevice * device,
        PhyWorld * world)
  : IPhaseHandler(device,world)
{

  m_transiction=new TweenBounceOut1d;   //new linearTrans();

  m_radius=20.;
  m_angleSpan=deg2rad(8.);
  m_timeStep=1./80.;
}

void VehicleChooser::prepare(unsigned nHumanVehicles, unsigned TotVehicles, unsigned * vehiclesListPtr)
{

  assert(nHumanVehicles == 1);

  m_angle=0.;


  const std::vector<IVehicle*> & vehicles=
    ResourceManager::getInstance()->getVehiclesList();

  assert(m_shownVehicles <= vehicles.size());

  double offset=0.;

  for(unsigned i=0; i<m_shownVehicles; i++, offset+=m_angleSpan) {
    m_infos[i].vehicle=vehicles[i];
    m_infos[i].angleOffset=offset;

    m_infos[i].index=i;
    m_infos[i].rotation=0.;
    m_infos[i].vehicle->use(IVehicle::USE_GRAPHICS);
  }

  m_vehicleIndex=0;
  m_status=status_still;

  m_choosenVehicles=vehiclesListPtr;
  m_totChooseableVehicles=TotVehicles;
  m_humanVehicles=nHumanVehicles;

  m_camera = m_device->getSceneManager()->addCameraSceneNode();
  m_camera->setPosition(irr::core::vector3df(3.,0.,0.));
  m_camera->setTarget(irr::core::vector3df(0,0.,0.));

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

void VehicleChooser::unprepare()
{
  m_camera->remove();
  m_sun->remove();
}

bool VehicleChooser::step()
{
  EventReceiver * erec;
  erec=ResourceManager::getInstance()->getEventReceiver();

  if(m_status == status_still) {
    if(erec->OneShotKey(irr::KEY_RIGHT) && m_vehicleIndex < m_shownVehicles-1) {
      m_vehicleIndex++;
      m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
      m_status=status_rotating;
      m_transictionTime = 0.;
      GM_LOG("moving to %d\n",m_vehicleIndex);
    } 

    if(erec->OneShotKey(irr::KEY_LEFT) && m_vehicleIndex > 0) {
      m_vehicleIndex--;
      m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
      m_status=status_rotating;
      m_transictionTime = 0.;
      GM_LOG("moving to %d\n",m_vehicleIndex);
    } 

  } else {
    if(m_transiction->doit(m_transictionTime,m_angle)) {
      m_status=status_still;
      GM_LOG("done!!!\n");
    } else {
    }
    m_transictionTime += m_timeStep;
  }

  bool done=false;


  double angle;

  irr::core::vector3df pos; 

  for(unsigned i=0; i<m_shownVehicles; i++) {
    angle=m_infos[i].angleOffset + m_angle;
    pos.X=cos(angle) * m_radius - m_radius;
    pos.Y=0.;
    pos.Z=sin(angle) * m_radius;
    m_infos[i].rotation+=.05;

    m_infos[i].vehicle->reset(pos,m_infos[i].rotation);
  }


  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  m_sceneManager->drawAll();
  m_guiEnv->drawAll();
  m_world->debugDrawWorld();
  m_driver->endScene();

  return done;
}

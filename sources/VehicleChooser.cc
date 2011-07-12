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
  m_vehiclesHeight=1.;
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
    m_infos[i].height=m_infos[i].vehicle->getRestHeight();
  }

  m_vehicleIndex=0;
  m_status=status_still;

  m_choosenVehicles=vehiclesListPtr;
  m_totChooseableVehicles=TotVehicles;
  m_humanVehicles=nHumanVehicles;



  // should be loaded from the XML
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
    if(erec->OneShotKey(irr::KEY_RIGHT)) {
      if(m_vehicleIndex < m_shownVehicles-2) {
        m_vehicleIndex++;
        m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
        m_status=status_rotating;
        m_transictionTime = 0.;
      } else {
        const std::vector<IVehicle*> & vehicles=
          ResourceManager::getInstance()->getVehiclesList();
        if(m_infos[m_shownVehicles-1].index < vehicles.size()-1) {
          unsigned nindex=m_infos[m_shownVehicles-1].index+1;
          double   offset=m_infos[m_shownVehicles-1].angleOffset + m_angleSpan;
          unsigned i;
          m_infos[0].vehicle->unuse(IVehicle::USE_GRAPHICS);
          for(i=0; i<m_shownVehicles-1; i++) 
            memcpy(&m_infos[i],&m_infos[i+1],sizeof(m_infos[0]));
          m_infos[i].vehicle=vehicles[nindex];
          m_infos[i].angleOffset=offset;
          m_infos[i].index=nindex;
          m_infos[i].rotation=0.;
          m_infos[i].vehicle->use(IVehicle::USE_GRAPHICS);
          m_infos[i].height=m_infos[i].vehicle->getRestHeight();

          m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
          m_status=status_rotating;
          m_transictionTime=0.;
        } else if(m_infos[m_vehicleIndex].index < vehicles.size()-1) {
          m_vehicleIndex++;
          m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
          m_status=status_rotating;
          m_transictionTime = 0.;
        }
      }
    } 

    if(erec->OneShotKey(irr::KEY_LEFT)) {
      if(m_vehicleIndex > 1) {
        m_vehicleIndex--;
        m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
        m_status=status_rotating;
        m_transictionTime = 0.;
      } else if (m_infos[m_vehicleIndex].index > 1) {
        unsigned nindex=m_infos[m_vehicleIndex-1].index-1;
        double   offset=m_infos[m_vehicleIndex-1].angleOffset - m_angleSpan;
        unsigned i;
        const std::vector<IVehicle*> & vehicles=
          ResourceManager::getInstance()->getVehiclesList();
        for(i=m_shownVehicles-1; i>0; i-- )
          memcpy(&m_infos[i],&m_infos[i-1],sizeof(m_infos[0]));

        m_infos[i].vehicle=vehicles[nindex];
        m_infos[i].angleOffset=offset;
        m_infos[i].index=nindex;
        m_infos[i].rotation=0.;
        m_infos[i].vehicle->use(IVehicle::USE_GRAPHICS);
        m_infos[i].height=m_infos[i].vehicle->getRestHeight();

        m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
        m_status=status_rotating;
        m_transictionTime=0.;
      } else if (m_infos[m_vehicleIndex].index == 1) {
        m_vehicleIndex--;
        m_transiction->init(0., m_angle, -m_infos[m_vehicleIndex].angleOffset);
        m_status=status_rotating;
        m_transictionTime = 0.;
      }
    }
  } else {
    if(m_transiction->doit(m_transictionTime,m_angle)) {
      m_status=status_still;
    } else {
    }
    m_transictionTime += m_timeStep;
  }

  bool done=false;

  if(erec->OneShotKey(irr::KEY_RETURN))
  {
    for(unsigned i=0; i < m_shownVehicles; i++) {
      m_infos[i].vehicle->unuse(IVehicle::USE_GRAPHICS);
    }
    m_choosenVehicles[0]=m_infos[m_vehicleIndex].index;
    done=true;
  }

  double angle;

  irr::core::vector3df pos; 

  for(unsigned i=0; i<m_shownVehicles; i++) {
    angle=m_infos[i].angleOffset + m_angle;
    pos.X=cos(angle) * m_radius - m_radius;
    pos.Y=m_infos[i].height - m_vehiclesHeight;
    pos.Z=sin(angle) * m_radius;
    m_infos[i].rotation+=.05;

    m_infos[i].vehicle->reset(pos,m_infos[i].rotation);
  }

  ResourceManager *  resman=ResourceManager::getInstance();


  unsigned width;
  resman->getScreenWidth(width);
  irr::gui::IGUIFont *  m_font=resman->getSystemFontBig();
  irr::gui::IGUIFont *  m_fontSmall=resman->getSystemFontSmall();

  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  m_sceneManager->drawAll();
  irr::core::rect<irr::s32> rect(0,0,width,100);

  m_font->draw("CHOOSE VEHICLE",rect,irr::video::SColor(255,255,255,255),true,true);

  if(m_status == status_still) {
    irr::core::rect<irr::s32> rect(0,100,width,200);
    const std::string & name=m_infos[m_vehicleIndex].vehicle->getName();
    m_fontSmall->draw(name.c_str(),rect,irr::video::SColor(255,255,255,255),true,true);
  }
  m_guiEnv->drawAll();
  m_world->debugDrawWorld();
  m_driver->endScene();

  return done;
}

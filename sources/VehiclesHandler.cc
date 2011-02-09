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
#include "VehiclesHandler.h"
#include "Vehicle.h"
#include "ResourceManager.h"

#ifndef BASE_DIR
#define BASE_DIR "./"
#endif

enum {
  INVALID=0xffff
};

static const char * cars[]={
  "test-1.zip"//,
  //"test-2.zip",
  //"arrow-car.zip"
};

VehiclesHandler::VehiclesHandler(
    irr::IrrlichtDevice * device, 
    PhyWorld * world)
  : IPhaseHandler(device,world)
{
  enum { buffer_len=1024 };
  char buffer[buffer_len];

  unsigned n_vehicles=sizeof(cars)/sizeof(cars[0]);
  GM_LOG("Loading %d vehcles\n",n_vehicles);

  for(unsigned int i=0; i<n_vehicles; i++) {
    GM_LOG("building '%s'\n",cars[i]);
    ResourceManager::getVehicleCompletePath(cars[i], buffer,buffer_len);
      Vehicle * hh=new Vehicle(
        m_sceneManager->getRootSceneNode(),
        m_device,world,buffer);
      if(hh==0) {
        GM_LOG("Cannot load '%s'\n",buffer);
        continue;
      }
      GM_LOG("loading '%s'\n",cars[i]);
      hh->load();
      GM_LOG("pushing\n");
      m_vehicles.push_back(hh);
  }

	m_sceneManager->setAmbientLight(irr::video::SColorf(0.2f, 0.2f, 0.2f));

  irr::scene::ILightSceneNode *light;

  light = m_sceneManager->addLightSceneNode(0, 
      irr::core::vector3df(10.f,40.f,-5.f),
      irr::video::SColorf(0.2f, 0.2f, 0.2f), 90.f);

  m_currentVehicleIndex=INVALID;
  m_status=ST_SHOWING;
  m_rotator=0;

  startVehicle();
}

void VehiclesHandler::startVehicle(unsigned index)
{
  GM_LOG("--------------------\n");
  GM_LOG("starting vehicle %d\n",index);
  if(m_currentVehicleIndex!=INVALID) {
    IVehicle * p=m_vehicles[m_currentVehicleIndex];
    GM_LOG("unusing  vehicle %d\n", m_currentVehicleIndex);
    p->removeAnimator(m_rotator);
    p->unuse(IVehicle::USE_GRAPHICS);
  }
  IVehicle * h=m_vehicles[index];
  m_currentVehicleIndex=index;
  m_status=ST_SHOWING;
  h->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);

  if(m_rotator==0) {
    m_rotator=m_sceneManager->createRotationAnimator(
        irr::core::vector3df(0.f, 0.8f, 0.f));
    if(m_rotator) {
      m_rotator->grab();
    }
  }
  h->addAnimator(m_rotator);
}

VehiclesHandler::~VehiclesHandler()
{
  m_rotator->grab();
}

void VehiclesHandler::prevVehicle()
{
  if(m_currentVehicleIndex==0)
    startVehicle(m_vehicles.size()-1);
  else
    startVehicle(m_currentVehicleIndex-1);
}

void VehiclesHandler::nextVehicle()
{
  unsigned i=m_currentVehicleIndex+1;
  if(i>=m_vehicles.size())
    i=0;
  startVehicle(i);
}

void VehiclesHandler::step()
{
  m_sceneManager->drawAll();
}

bool VehiclesHandler::OnEvent(const irr::SEvent& event)
{

  irr::core::vector3df position;

  if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
    switch(event.KeyInput.Key) {
      case irr::KEY_RETURN:
        if(event.KeyInput.PressedDown==false) {
          GM_LOG("select a vehicle\n");
        }
        break;
      case irr::KEY_LEFT:
        if(event.KeyInput.PressedDown==false)
          nextVehicle();
      break;
      case irr::KEY_RIGHT:
        if(event.KeyInput.PressedDown==false)
          prevVehicle();
        break;
      default:
      break;
    }
  }
  return true;
}

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

VehiclesHandler::VehiclesHandler(
    irr::IrrlichtDevice * device, 
    PhyWorld * world)
  : IPhaseHandler(device,world)
{
  Vehicle * h=new Vehicle(
      m_sceneManager->getRootSceneNode(),
      m_device,world,"car_ab.zip");
  h->load();
  h->use(IVehicle::USE_GRAPHICS);

  irr::scene::ISceneNodeAnimator* anim =
		m_sceneManager->createRotationAnimator(
        irr::core::vector3df(0.f, 0.8f, 0.f));

	if(anim)
	{
		h->addAnimator(anim);
		anim->drop();
		anim = 0;
	}
	m_sceneManager->setAmbientLight(irr::video::SColorf(0.2f, 0.2f, 0.2f));

  irr::scene::ILightSceneNode *light;

  light = m_sceneManager->addLightSceneNode(0, 
      irr::core::vector3df(10.f,40.f,-5.f),
      irr::video::SColorf(0.2f, 0.2f, 0.2f), 90.f);

  m_vehicles.push_back(h);
  m_currentVehicleIndex=0;
}


bool VehiclesHandler::OnEvent(const irr::SEvent& event)
{
  IVehicle * vehicle=0;
  if(m_currentVehicleIndex<m_vehicles.size()) 
    vehicle=m_vehicles[m_currentVehicleIndex];

  double vel=.01;

  irr::core::vector3df position;

  if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
    switch(event.KeyInput.Key) {
      case irr::KEY_LEFT:
        if(vehicle) {
          position=vehicle->getPosition();
          position.X=position.X-vel;
          vehicle->setPosition(position);
        }
      break;
      case irr::KEY_RIGHT:
        if(vehicle) {
          position=vehicle->getPosition();
          position.X=position.X+vel;
          vehicle->setPosition(position);
        }
        break;
      default:
      break;
    }
  }
  return true;
}

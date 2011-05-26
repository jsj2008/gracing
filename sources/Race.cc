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

#include "Race.h"


Race::Race(irr::IrrlichtDevice * device, PhyWorld * world)
  : IPhaseHandler(device,world)
{
  m_readySetGo = new GuiReadySetGo(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(0,0,200,100));
  assert(m_readySetGo);

  m_cronometer = new GuiCronometer(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(0,100,400,200),device->getTimer());
  assert(m_cronometer);

  m_nVehicles=0;
  m_track=0;

  restart();
}

void Race::step()
{
  // status handling
  switch(m_status) {
    case rs_readySetGo:
      if(m_readySetGo->isEnded()) {
        GM_LOG("going into started state\n");
        assert(gotoState(rs_started));
      }
      break;
  }


  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  m_sceneManager->drawAll();
  m_world->step();
  m_guiEnv->drawAll();
  m_world->debugDrawWorld();
  m_driver->endScene();
}

bool Race::gotoState(unsigned state)
{
  switch(state) {
    case rs_readySetGo:
      // reset vehicles
      for(int i=0; i<m_nVehicles; i++)  {
        m_vehicles[i].vehicle->reset(m_vehicles[i].startPosition,
            m_vehicles[i].startRotation);
        m_vehicles[i].vehicle->setEnableControls(false);
      }
      // reset gui controls
      m_readySetGo->restart();
      m_cronometer->stop();
      m_status=rs_readySetGo;
      break;
    case rs_started:
      if(m_status!=rs_readySetGo)
        return false;
      for(int i=0; i<m_nVehicles; i++) 
        m_vehicles[i].vehicle->setEnableControls(true);
      m_status=rs_started;
      m_cronometer->start();
  }

  return m_status==state;
}


bool Race::addVehicle(IVehicle * vehicle)
{
  if(m_nVehicles == max_vehicles)
    return false;
  if(m_track == 0)
    return false;
  m_sceneManager->getRootSceneNode()->addChild(vehicle);
  vehicle->reset(m_track->getStartPosition(),m_track->getStartRotation());

  m_vehicles[m_nVehicles].vehicle=vehicle;
  m_vehicles[m_nVehicles].startPosition=m_track->getStartPosition();
  m_vehicles[m_nVehicles].startRotation=m_track->getStartRotation();
  m_vehicles[m_nVehicles].vehicle->setEnableControls(false);
  m_nVehicles++;

  return true;
}

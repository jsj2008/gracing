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

// !'!'!'!'!unsigned
static inline unsigned nextIndexVehicleControlPoint(unsigned currentIndex, 
    const std::vector<btVector3> & controlPoints)  
{ 
  return (currentIndex+1) % controlPoints.size(); 
}

static bool evolveVehicleControlPoint(
    unsigned &                        currentIndex,
    const    btVector3 &              position,
    const    std::vector<btVector3> & controlPoints)
{
  double   d0,d1; 
  unsigned i1; 

  assert(currentIndex < controlPoints.size());

  i1=nextIndexVehicleControlPoint(currentIndex,controlPoints);


  d0=(controlPoints[currentIndex]-position).length2();
  d1=(controlPoints[i1]-position).length2();

  if(d0 <= d1) 
    return false;

  currentIndex = i1;

  return true;
}

static inline double distanceVehicleControlPoint(
    unsigned &                        currentIndex,
    const    btVector3 &              position,
    const    std::vector<btVector3> & controlPoints)
{
  assert(currentIndex < controlPoints.size());

  unsigned i1=nextIndexVehicleControlPoint(currentIndex,controlPoints);

  btVector3 relPos = position - controlPoints[i1];

  double d=(controlPoints[currentIndex] - controlPoints[i1]).dot(relPos);

  return d;
}

static unsigned initVehicleControlPoint(
    const btVector3 & position,
    const std::vector<btVector3> & controlPoints)
{
  unsigned iMin;
  double min,dis;

  if(controlPoints.size() == 0)
    return 0;

  iMin=0;
  min=(controlPoints[iMin]-position).length2();

  for(unsigned i=1; i < controlPoints.size(); i++) 
    if((dis=(controlPoints[i]-position).length2()) < min) {
      min=dis;
      iMin=i;
    }

  return iMin;
}

static btVector3 vectIrrToBullet(const irr::core::vector3df & irrv)
{
  return btVector3(irrv.X, irrv.Y, irrv.Z);
}

// !'!'!'!'!


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

void Race::updateVehiclesInfo()
{
  const std::vector<btVector3> & controlPoints=m_track->getControlPoints();
  for(unsigned index=0; index<m_nVehicles; index++) {
    VehicleInfo & vinfo=m_vehicles[index];

    btVector3 pos=vectIrrToBullet(vinfo.vehicle->getChassisPos());
    double newDist;

    if(evolveVehicleControlPoint(vinfo.controlPointIndex,
          pos,controlPoints)) {
      if(vinfo.controlPointIndex == controlPoints.size()-1) 
        vinfo.waitingForLapTrigger=true;
      newDist=
        distanceVehicleControlPoint(
            vinfo.controlPointIndex,
            pos,
            controlPoints);
    } else {

      newDist=
        distanceVehicleControlPoint(
            vinfo.controlPointIndex,
            pos,
            controlPoints);

      if(newDist > (vinfo.ctrlPntDistance+0.5))
        vinfo.wrongWay=true;
      else
        vinfo.wrongWay=false;
    }

    if(vinfo.wrongWay) {
      static int so=0;
      GM_LOG("Wrong way %d!!\n",so++);
    }
    
    vinfo.ctrlPntDistance=newDist;
  }
}

void Race::step()
{
  // status handling
  switch(m_status) {
    case rs_readySetGo:
      if(m_readySetGo->isEnded()) {
        GM_LOG("going into started state\n");
        gotoState(rs_started);
      }
      break;
    case rs_started:
      updateVehiclesInfo();
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
      for(unsigned i=0; i<m_nVehicles; i++)  {
        m_vehicles[i].vehicle->reset(m_vehicles[i].startPosition,
            m_vehicles[i].startRotation);
        m_vehicles[i].vehicle->setEnableControls(false);

        btVector3 pos=vectIrrToBullet(m_vehicles[i].startPosition);
        m_vehicles[i].controlPointIndex=
          initVehicleControlPoint(pos,m_track->getControlPoints());
        m_vehicles[i].wrongWay=false;
        m_vehicles[i].ctrlPntDistance=
          distanceVehicleControlPoint(
              m_vehicles[i].controlPointIndex,
              pos,
              m_track->getControlPoints());
      }
      // reset gui controls
      m_readySetGo->restart();
      m_cronometer->stop();
      m_status=rs_readySetGo;
      break;
    case rs_started:
      if(m_status!=rs_readySetGo)
        return false;
      for(unsigned i=0; i<m_nVehicles; i++) 
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
  m_vehicles[m_nVehicles].waitingForLapTrigger=false;
  m_track->registerLapCallback(this, &(m_vehicles[m_nVehicles]));

  m_nVehicles++;


  return true;
}


void Race::lapTriggered(void * userdata)
{
  VehicleInfo * vinfo=(VehicleInfo*)userdata;
  if(vinfo->waitingForLapTrigger) {
    vinfo->waitingForLapTrigger=false;
    GM_LOG("A new lap\n");
  }
}

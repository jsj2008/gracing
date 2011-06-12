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
#include "IrrDebugDrawer.h"

extern IrrDebugDrawer * debugDrawer;

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

  irr::s32 px1,px2,py1,py2;
  irr::s32 width,height;

  width=400; height=100;
  px1=1024-width;     py1=0;

  px2=px1+width;
  py2=py1+height;

  m_cronometer = new GuiCronometer(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),device->getTimer());
      //irr::core::rect<irr::s32>(0,100,400,200),device->getTimer());

  width=1024; height=100;
  px1=1024-width;     py1=(576-height)/2-50;
  px2=px1+width;
  py2=py1+height;

  m_communicator = new GuiCommunicator(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2));
  assert(m_cronometer);

  m_nVehicles=0;
  m_track=0;

  restart();
}

void Race::updateVehiclesInfo()
{
  const std::vector<btVector3> & controlPoints=m_track->getControlPoints();
  
  if(debugDrawer) {
    btVector3 color(0.,0.,1.);
    btVector3 color2(0.,1.,0.);
    for(unsigned i=0; i<controlPoints.size(); i++)
      if(i%2) 
      debugDrawer->drawLine(controlPoints[i],
          controlPoints[nextIndexVehicleControlPoint(i,controlPoints)],
          color);
      else
      debugDrawer->drawLine(controlPoints[i],
          controlPoints[nextIndexVehicleControlPoint(i,controlPoints)],
          color2);
  }

  for(unsigned index=0; index<m_nVehicles; index++) {
    VehicleInfo & vinfo=m_vehicles[index];

    assert(vinfo.controller);

    btVector3 vehicleDirection;
    btVector3 vehicleRightDirection;
    btVector3 vehiclePosition;

    vehicleDirection = vinfo.vehicle->getChassisForwardDirection();
    vehicleRightDirection = vinfo.vehicle->getChassisRightDirection();
    vehiclePosition = vectIrrToBullet(vinfo.vehicle->getChassisPos());

    vinfo.controller->updateCommands(
        vehicleDirection,
        vehicleRightDirection,
        vehiclePosition,
        vinfo.controlPointIndex,
        controlPoints,
        vinfo.vehicle->getVehicleCommands());

    double newDist;

    if(evolveVehicleControlPoint(vinfo.controlPointIndex,
          vehiclePosition,controlPoints)) {
      if(vinfo.controlPointIndex == controlPoints.size()-1) 
        vinfo.waitingForLapTrigger=true;
      newDist=
        distanceVehicleControlPoint(
            vinfo.controlPointIndex,
            vehiclePosition,
            controlPoints);
    } else {

      newDist=
        distanceVehicleControlPoint(
            vinfo.controlPointIndex,
            vehiclePosition,
            controlPoints);

      if(newDist > (vinfo.ctrlPntDistance+0.5)) {
        if(!vinfo.wrongWay)
          m_communicator->show("Wrong way");
        vinfo.wrongWay=true;
      } else
        //m_communicator->unshow();
        vinfo.wrongWay=false;
    }

    if(vinfo.wrongWay) {
    }

    vinfo.ctrlPntDistance=newDist;

    // check for overturn
    if(vinfo.vehicle->getIfChassisIsTouchingTheGround()) {
      vinfo.overturnCountDown=0;
    } else {
      if(vinfo.overturnCountDown) {
        if(--vinfo.overturnCountDown == 0) {
          GM_LOG("vehicle overturn!!!!");
        }
      } else {
        vinfo.overturnCountDown=160;
      }
    }

    // check to see if the vehicle is on the road
    unsigned nindex=
      nextIndexVehicleControlPoint(vinfo.controlPointIndex,controlPoints);
    btVector3 p1=vehiclePosition - controlPoints[vinfo.controlPointIndex];
    btVector3 p2=controlPoints[nindex] - controlPoints[vinfo.controlPointIndex];

    //btScalar p1_project_length=p1.dot(p2) /  p2.length();
    //btVector3 p1_proejct= p2 * (p1_project_length / p2.length());
    // to previous two line shold be equivalent to the following two
    btScalar p1_project_length=p1.dot(p2) /  p2.length2();
    btVector3 p1_project= p2 * p1_project_length ;

    btScalar dist=
        (vehiclePosition-
            controlPoints[vinfo.controlPointIndex] + p1_project).length();

    if(debugDrawer) {
      btVector3 color(0.,0.,1.);
      btVector3 color2(0.,1.,0.);
      if(vinfo.controlPointIndex%2)
        debugDrawer->drawLine(vehiclePosition,
            controlPoints[vinfo.controlPointIndex] + p1_project,
            color);
      else
        debugDrawer->drawLine(vehiclePosition,
            controlPoints[vinfo.controlPointIndex] + p1_project,
            color2);
    }
  }
}

void Race::step()
{
  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
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
        m_vehicles[i].lapNumber=0;
        m_vehicles[i].overturnCountDown=0;
        m_vehicles[i].waitingForLapTrigger=false;
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

bool Race::addVehicle(IVehicle * vehicle,IVehicleController * controller)
{
  if(m_nVehicles == max_vehicles)
    return false;
  if(m_track == 0)
    return false;
  m_sceneManager->getRootSceneNode()->addChild(vehicle);
  vehicle->reset(m_track->getStartPosition(),m_track->getStartRotation());

  m_vehicles[m_nVehicles].vehicle=vehicle;
  m_vehicles[m_nVehicles].vehicle->setEnableControls(false);
  m_vehicles[m_nVehicles].waitingForLapTrigger=false;
  m_vehicles[m_nVehicles].controller=controller;

  m_track->registerLapCallback(this, vehicle, &(m_vehicles[m_nVehicles]));

  m_nVehicles++;

  recalcVehicleVehiclesStartPositions();

  return true;
}

void Race::lapTriggered(void * userdata)
{
  VehicleInfo * vinfo=(VehicleInfo*)userdata;
  if(vinfo->waitingForLapTrigger) {
    vinfo->waitingForLapTrigger=false;
    GM_LOG("Finished lap %d\n",vinfo->lapNumber+1);
    m_communicator->show("lap %d",vinfo->lapNumber+1);
    vinfo->lapNumber++;
  }
}

void Race::recalcVehicleVehiclesStartPositions()
{
  const double vehicleSpace=3.;
  double thlen=(m_nVehicles-1)*vehicleSpace*.5;
  double angle=m_track->getStartRotation();

  for(unsigned i=0; i<m_nVehicles; i++) {
    double zp= (i * vehicleSpace) - thlen;
    double xp;

    xp = sin(angle) * zp;
    zp = cos(angle) * zp;

    m_vehicles[i].startPosition.X = m_track->getStartPosition().X + xp;
    m_vehicles[i].startPosition.Y = m_track->getStartPosition().Y ;
    m_vehicles[i].startPosition.Z = m_track->getStartPosition().Z + zp;
    m_vehicles[i].startRotation=m_track->getStartRotation();
  }
}


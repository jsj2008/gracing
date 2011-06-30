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

#define CAMERA_STEP 0.05

CFG_PARAM_D(glob_epsilonDist)=0.1;

/* versione due */

// !'!'!'!'!
static inline unsigned nextIndexVehicleControlPoint(unsigned currentIndex, 
    const std::vector<btVector3> & controlPoints)  
{ 
  return (currentIndex+1) % controlPoints.size(); 
}

static inline double distanceVehicleControlPoint(
    unsigned &                        currentIndex,
    const    btVector3 &              position,
    const    std::vector<btVector3> & controlPoints)
{
  assert(currentIndex < controlPoints.size());

  unsigned i1=nextIndexVehicleControlPoint(currentIndex,controlPoints);

  btVector3 relPos = position - controlPoints[i1];

  if( relPos.length() < .001)
    return 0.;

  double d=(controlPoints[currentIndex] - controlPoints[i1]).dot(relPos);

  d /= (controlPoints[currentIndex] - controlPoints[i1]).length2();

  if(d < 0) {
    GM_LOG("<<<<%f\n",d);
  }

  //assert( d >= 0);
  //assert( d <= (controlPoints[currentIndex] - controlPoints[i1]).length());

  return d;
}

static bool evolveVehicleControlPoint(
    unsigned &                        currentIndex,
    const    btVector3 &              position,
    const    std::vector<btVector3> & controlPoints)
{
  //double   d0,d1; 
  //unsigned i1; 

  assert(currentIndex < controlPoints.size());

  // TODO: optimize the double calc of the control
  //       point distance
  double dist=
    distanceVehicleControlPoint(currentIndex, position,controlPoints); 

  if (dist < glob_epsilonDist) {
    currentIndex = nextIndexVehicleControlPoint(currentIndex,controlPoints);
    return true;
  }

  return false;
}

void Race::setTrack(Track * track) { 
  m_track=track;
  m_track->load();
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

static irr::core::vector3df vectBulletToIrr(const btVector3 & vect)
{
  return irr::core::vector3df(vect.getX(),vect.getY(),vect.getZ());
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

#if 0
  width=400; height=100;
  px1=1024-width;     py1=0;

  px2=px1+width;
  py2=py1+height;

  m_cronometer = new GuiCronometer(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),device->getTimer());
#endif

  width=1024; height=600;
  px1=1024-width;     py1=(576-height)/2-50;
  px2=px1+width;
  py2=py1+height;

  m_communicator = new GuiCommunicator(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2));
  assert(m_communicator);

  m_nVehicles=0;
  m_track=0;
  m_totalLaps=1;
  m_camera = device->getSceneManager()->addCameraSceneNode();
  m_device = device;
  m_cameraAnim = 0;
  m_followedVehicleIndex=invalidVehicleIndex;

  width=200; height=100;
  px1=5;     py1=740-height;

  px2=px1+width;
  py2=py1+height;
  m_cockpit = new GuiCockpit(m_guiEnv,m_guiEnv->getRootGUIElement(),3,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),device->getTimer());

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
        if(!vinfo.wrongWay && m_followedVehicleIndex == vinfo.index)
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
          restoreVehicle(vinfo);
        }
      } else {
        vinfo.overturnCountDown=160;
      }
    }

    // check to see if the vehicle is on the road

//    unsigned nindex=
//      nextIndexVehicleControlPoint(vinfo.controlPointIndex,controlPoints);
//    btVector3 p1=vehiclePosition - controlPoints[vinfo.controlPointIndex];
//    btVector3 p2=controlPoints[nindex] - controlPoints[vinfo.controlPointIndex];
//
//    btScalar p1_project_length=p1.dot(p2) /  p2.length2();
//    btVector3 p1_project= p2 * p1_project_length ;

#if 0
    btScalar dist=
        (vehiclePosition-
            controlPoints[vinfo.controlPointIndex] + p1_project).length();
#endif

//    if(debugDrawer) {
//      btVector3 color(0.,0.,1.);
//      btVector3 color2(0.,1.,0.);
//      if(vinfo.controlPointIndex%2)
//        debugDrawer->drawLine(vehiclePosition,
//            controlPoints[vinfo.controlPointIndex] + p1_project,
//            color);
//      else
//        debugDrawer->drawLine(vehiclePosition,
//            controlPoints[vinfo.controlPointIndex] + p1_project,
//            color2);
//    }
  }
}

void Race::step()
{
  // status handling
  switch(m_status) {

    case rs_finished:
      m_communicator->refreshTime();
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      m_sceneManager->drawAll();
      m_guiEnv->drawAll();
      m_world->step();
      m_world->debugDrawWorld();
      m_driver->endScene();
      break;

    case rs_paused:
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      m_communicator->show("PAUSED");
      m_sceneManager->drawAll();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      updateKeyboard();
      break;
    case rs_readySetGo:
      if(m_readySetGo->isEnded()) {
        gotoState(rs_started);
      }
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      m_sceneManager->drawAll();
      m_world->step();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      break;
    case rs_started:
      updateRanking();

      // update cockpit
      m_cockpit->setRank(
          m_vehicles[m_followedVehicleIndex].rank+1,m_nVehicles);
      m_cockpit->setLap(m_vehicles[m_followedVehicleIndex].lapNumber+1,m_totalLaps);
      ///////////////////////

      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      updateKeyboard();
      updateVehiclesInfo();
      m_sceneManager->drawAll();
      m_world->step();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      break;
    default:
      break;
  }
}

void Race::updateRanking()
{
  //unsigned rank[max_vehicles];
  unsigned i,j,m,o;

  for(i=m_nFinishedVehicles; i<m_nVehicles; i++) {
    m=i;
    for(j=i+1; j<m_nVehicles; j++) {
      VehicleInfo & a=m_vehicles[m_rank[m]];
      VehicleInfo & b=m_vehicles[m_rank[j]];
      if( vehicleInfoCmp(a,b) > 0) 
        m=j;
    }
    if(m!=i) {
      o=m_rank[i];
      m_rank[i]=m_rank[m];
      m_rank[m]=o;
    }
  }


  for(i=m_nFinishedVehicles; i<m_nVehicles; i++) 
    m_vehicles[m_rank[i]].rank=i;
}

void Race::updateKeyboard()
{
  EventReceiver * erec;

  erec=ResourceManager::getInstance()->getEventReceiver();

  if(erec->OneShotKey(irr::KEY_KEY_C)) 
    restart();

  if(erec->OneShotKey(irr::KEY_KEY_P)) 
    togglePause();

  if(erec->OneShotKey(irr::KEY_TAB)) {
    GM_LOG("changing vehicle\n");
    followNextVehicle();
  }

  if(erec->IsKeyDown(irr::KEY_KEY_W))
    m_cameraAnim->moveXY(0.,CAMERA_STEP);

  if(erec->IsKeyDown(irr::KEY_KEY_Z)) 
    m_cameraAnim->moveXY(0.,-CAMERA_STEP);

  if(erec->IsKeyDown(irr::KEY_KEY_A))
    m_cameraAnim->moveXY(CAMERA_STEP,0.);

  if(erec->IsKeyDown(irr::KEY_KEY_S)) 
    m_cameraAnim->moveXY(-CAMERA_STEP,0.);
}

void Race::followNextVehicle()
{
  assert(m_followedVehicleIndex != invalidVehicleIndex);

  m_vehicles[m_followedVehicleIndex].vehicle->setSpeedOMeter(0);

  m_followedVehicleIndex ++;

  if(m_followedVehicleIndex == m_nVehicles)
    m_followedVehicleIndex = 0;

  m_vehicles[m_followedVehicleIndex].vehicle->setSpeedOMeter(m_cockpit);
  m_cameraAnim->changeVehicle(m_vehicles[m_followedVehicleIndex].vehicle);
  //m_cockpit->setLap(m_vehicles[m_followedVehicleIndex].lapNumber+1,m_totalLaps);
  GM_LOG("following next vehicle: '%s' rank: %d\n",m_vehicles[m_followedVehicleIndex].name.c_str(),
                m_vehicles[m_followedVehicleIndex].rank);

}


bool Race::gotoState(unsigned state)
{
  switch(state) {
    case rs_paused:
      if(m_status != rs_started)
        return false;
      m_cockpit->pause();
      m_status=rs_paused;
      break;
    case rs_finished:
      GM_LOG("race finished\n");
      m_communicator->show("race rank");
      m_cockpit->stop();
      for(unsigned i=0; i < m_nVehicles; i++) {
       VehicleInfo & vinfo=m_vehicles[m_rank[i]];
       m_communicator->add(false,"[%d] %s",i+1,vinfo.name.c_str());
      }
      m_status=rs_finished;
      break;
    case rs_readySetGo:
      for(unsigned i=0; i<m_nVehicles; i++)  {
        m_vehicles[i].vehicle->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);

        m_vehicles[i].vehicle->reset(m_vehicles[i].startPosition,
            m_vehicles[i].startRotation);
        m_vehicles[i].vehicle->setEnableControls(false);

        btVector3 pos=vectIrrToBullet(m_vehicles[i].startPosition);
        m_vehicles[i].controlPointIndex=
          initVehicleControlPoint(pos,m_track->getControlPoints());
        m_firstControlPoint=m_vehicles[i].controlPointIndex;

        m_vehicles[i].wrongWay=false;
        m_vehicles[i].ctrlPntDistance=
          distanceVehicleControlPoint(
              m_vehicles[i].controlPointIndex,
              pos,
              m_track->getControlPoints());
        m_vehicles[i].lapNumber=0;
        m_vehicles[i].overturnCountDown=0;
        m_vehicles[i].waitingForLapTrigger=false;
        m_vehicles[i].raceFinished=false;
        m_track->registerLapCallback(this, 
            m_vehicles[i].vehicle, 
            &(m_vehicles[i]));
      }
      // reset gui controls
      m_readySetGo->restart();
      m_cockpit->stop();
      m_status=rs_readySetGo;
      m_nFinishedVehicles=0;
      break;
    case rs_started:
      if(m_status==rs_paused) {
        m_status=rs_started;
        m_cockpit->unpause();
      } else if(m_status==rs_readySetGo) {
        for(unsigned i=0; i<m_nVehicles; i++) 
          m_rank[i]=i;
        for(unsigned i=0; i<m_nVehicles; i++) 
          m_vehicles[i].vehicle->setEnableControls(true);
        m_status=rs_started;
        m_cockpit->start();
      }
  }

  return m_status==state;
}

bool Race::addVehicle(IVehicle * vehicle,IVehicleController * controller, 
    const char * name,
    bool followed)

{
  if(m_nVehicles == max_vehicles)
    return false;
  if(m_track == 0)
    return false;

  m_vehicles[m_nVehicles].index=m_nVehicles;
  m_vehicles[m_nVehicles].vehicle=vehicle;
  m_vehicles[m_nVehicles].vehicle->setEnableControls(false);
  m_vehicles[m_nVehicles].waitingForLapTrigger=false;
  m_vehicles[m_nVehicles].controller=controller;
  if(!name) {
    char buffer[32];
    snprintf(buffer,32,"vehicle %d",m_nVehicles);
    m_vehicles[m_nVehicles].name=buffer;
  } else {
    m_vehicles[m_nVehicles].name=name;
  }


  //m_sceneManager->getRootSceneNode()->addChild(vehicle);
  //vehicle->reset(m_track->getStartPosition(),m_track->getStartRotation());
  //m_track->registerLapCallback(this, vehicle, &(m_vehicles[m_nVehicles]));

  if(followed) {
    if(m_cameraAnim)
      m_cameraAnim->drop();
    m_cameraAnim=new
      VehicleCameraAnimator(vehicle);
    m_camera->addAnimator(m_cameraAnim);
    m_followedVehicleIndex = m_nVehicles;
    vehicle->setSpeedOMeter(m_cockpit);
    //m_cockpit->setLap(1,m_totalLaps);
  }

  m_nVehicles++;

  recalcVehicleVehiclesStartPositions();

  return true;
}

void Race::lapTriggered(void * userdata)
{
  VehicleInfo * vinfo=(VehicleInfo*)userdata;

  if(vinfo->waitingForLapTrigger) {
    vinfo->waitingForLapTrigger=false;
    vinfo->lapNumber++;
    if(vinfo->index==m_followedVehicleIndex) {
      m_communicator->show("lap %d",vinfo->lapNumber+1);
      //m_cockpit->setLap(vinfo->lapNumber+1,m_totalLaps);
    }
    if(vinfo->lapNumber == m_totalLaps) 
      vehicleFinished(*vinfo);
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

void Race::restoreVehicle(VehicleInfo & vinfo)
{
  unsigned nindex=
    nextIndexVehicleControlPoint(vinfo.controlPointIndex,
          m_track->getControlPoints());
  btVector3 v=
          m_track->getControlPoints()[nindex]-
          m_track->getControlPoints()[vinfo.controlPointIndex];
  btScalar  l=v.length();

  btScalar  c = v.dot(btVector3(0.,0.,1.)) / l;
  btScalar  s = v.dot(btVector3(1.,0.,0.)) / l;
  double    a = atan2(c,s);

  vinfo.vehicle->reset(
    vectBulletToIrr(m_track->getControlPoints()[vinfo.controlPointIndex]),a);
}

void Race::vehicleFinished(VehicleInfo & vinfo)
{
  vinfo.raceFinished=true; 
  vinfo.vehicle->setEnableControls(false);

  m_nFinishedVehicles++;

  if(m_nFinishedVehicles == 1) {
    m_communicator->show("%s wins!",vinfo.name.c_str());
  }
  
  GM_LOG("vehicle '%s' finished, still to finish %d\n",
      vinfo.name.c_str(),m_nVehicles - m_nFinishedVehicles);

  if(m_nFinishedVehicles == m_nVehicles) {
    gotoState(rs_finished);
  }
}

void Race::togglePause()
{
  if(m_status==rs_paused) {
    m_communicator->unshow();
    gotoState(rs_started);
  } else if(m_status==rs_started)  
    gotoState(rs_paused);
}

int Race::vehicleInfoCmp(const VehicleInfo & a, const VehicleInfo & b)
{

  // return:
  // -1 : if the vehicle a is more advanced than b
  //  1 : if the vehciel b is more advanced than c
  //  0 : in the (remote) case that they have both the same rank

  if(a.lapNumber > b.lapNumber) 
    return -1;

  if(a.lapNumber < b.lapNumber)
    return 1;

  unsigned na,nb;

  const std::vector<btVector3> & controlPoints=m_track->getControlPoints();

  na = (a.controlPointIndex +  controlPoints.size() - m_firstControlPoint) % controlPoints.size();
  nb = (b.controlPointIndex +  controlPoints.size() - m_firstControlPoint) % controlPoints.size();

  if( na > nb ) {
    return -1;
  }
    
  if( na < nb ) {
    return 1;
  }


#if 0
  static int ufo=0;
  if(a.index == 1 )
    GM_LOG("%d zogna %d: %f '%s'  %d: %f\n",ufo++,a.index,a.ctrlPntDistance,a.name.c_str(),b.index,b.ctrlPntDistance);
#endif

  if( a.ctrlPntDistance <  b.ctrlPntDistance )
    return -1;

  if( a.ctrlPntDistance >  b.ctrlPntDistance )
    return 1;

  return 0;
}

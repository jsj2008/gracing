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
GuiCommunicator * TEMP_communicator=0;

#define CAMERA_STEP 0.05
#define WRONG_WAY_EPSILON .001

CFG_PARAM_D(glob_epsilonDist)=0.1;

// !'!'!'!'!
static inline unsigned nextIndexVehicleControlPoint(
    unsigned currentIndex, 
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

  return d;
}

static bool evolveVehicleControlPoint(
    unsigned &                        currentIndex,
    const    btVector3 &              position,
    const    std::vector<btVector3> & controlPoints)
{
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

  for(unsigned i=1; i < controlPoints.size(); i++) {
    bool good;
    double posInSegment;
    posInSegment=distanceVehicleControlPoint(i,position,controlPoints);
    good = (posInSegment <= 1.) && (posInSegment >= 0);
    //GM_LOG("Control point %d distance: %f, good: %s\n",i,d,good?"yes":"no");
    if(((dis=(controlPoints[i]-position).length2()) < min) && good) {
      min=dis;
      iMin=i;
    }
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

Race::Race(irr::IrrlichtDevice * device, PhyWorld * world)
  : IPhaseHandler(device,world)
{
  m_readySetGo = new GuiReadySetGo(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(0,0,200,100));
  assert(m_readySetGo);

  irr::s32 px1,px2,py1,py2;
  irr::s32 width,height;

  width=1024; height=600;
  px1=1024-width;     py1=(576-height)/2-50;
  px2=px1+width;
  py2=py1+height;

  m_communicator = new GuiCommunicator(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2));
  TEMP_communicator = m_communicator;

  m_communicator->setPosition(GuiCommunicator::posLow);

  assert(m_communicator);

  m_nVehicles=0;
  m_nCameras=0;
  m_track=0;
  m_totalLaps=3;
  m_device = device;
  m_splitType=0;

#ifdef ONLY_1_CAM
  m_camera = 0;
  m_cameraAnim = 0;
  m_followedVehicleIndex=invalidVehicleIndex;
#endif


  // cock pit!!
  unsigned sw,sh;
  ResourceManager::getInstance()->getScreenHeight(sh);
  ResourceManager::getInstance()->getScreenWidth(sw);
  width=200; height=140;
  px1=5;     py1=sh-height;
  px2=px1+width;
  py2=py1+height;

#ifdef ONLY_1_CAM
  m_cockpit = new GuiCockpit(m_guiEnv,m_guiEnv->getRootGUIElement(),3,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),device->getTimer());
#endif

  m_status=rs_notRunning;
  //restart();
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

  SVehicleParameters vpar;

  for(unsigned index=0; index<m_nVehicles; index++) {
    VehicleInfo & vinfo=m_vehicles[index];

    assert(vinfo.controller);

    btVector3 vehicleDirection;
    btVector3 vehicleRightDirection;
    btVector3 vehiclePosition;

    vpar.vehicleDirection = vinfo.vehicle->getChassisForwardDirection();
    vpar.vehicleRightDirection = vinfo.vehicle->getChassisRightDirection();
    vehiclePosition = vpar.vehiclePosition = vectIrrToBullet(vinfo.vehicle->getChassisPos());
    vpar.vehicleSpeed = vinfo.vehicle->getSpeed();

    vinfo.controller->updateCommands(
        vpar,
        controlPoints,
        vinfo.vehicle->getVehicleCommands());

////////////////////////////////////////////////////////////
    if(vinfo.cameraData) {
      IVehicle::VehicleCommands & cmds=vinfo.vehicle->getVehicleCommands();
    
      if(vinfo.waitChangeCamera && cmds.changeCamera) {
        vinfo.cameraData->cameraAnim->nextCameraType();
        vinfo.waitChangeCamera=false;
      } else {
        vinfo.waitChangeCamera=true;
      }
    }
////////////////////////////////////////////////////////////

    double newDist;

    if(evolveVehicleControlPoint(
          vinfo.controlPointIndex,
          vehiclePosition,
          controlPoints))
    {
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


      if(newDist > vinfo.ctrlPntDistance + WRONG_WAY_EPSILON) {
#if 0
        if(!vinfo.wrongWay && m_followedVehicleIndex == vinfo.index && m_status == rs_started)
          m_communicator->show("Wrong way");
#endif
        vinfo.wrongWay=true;
      } else {
        vinfo.wrongWay=false;
      }
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

bool Race::step()
{
  bool ret=false;
  
  unsigned width,height;
  ResourceManager::getInstance()->getScreenHeight(height);
  ResourceManager::getInstance()->getScreenWidth(width);

  EventReceiver * erec=ResourceManager::getInstance()->getEventReceiver();
  // status handling
  switch(m_status) {

    case rs_notRunning:
      break;

    case rs_finished:
      m_communicator->refreshTime();
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      drawScene();
      m_guiEnv->drawAll();
      m_world->step();
      m_world->debugDrawWorld();
      m_driver->endScene();
      if(erec->IsAnyKeyDown()) {
        ret=true;
        m_status=rs_notRunning;
      }
      break;

    case rs_paused:
      m_driver->setViewPort(m_wholeScreenViewPort);
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      drawScene();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      ret=updateKeyboard();
      break;

    case rs_readySetGo:
      if(m_readySetGo->isEnded()) {
        gotoState(rs_started);
      }
      m_driver->setViewPort(m_wholeScreenViewPort);
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      updateVehiclesInfo();
      drawScene();
      m_world->step();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      break;

    case rs_started:
      updateRanking();
      m_driver->setViewPort(m_wholeScreenViewPort);
      m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
      ret=updateKeyboard();
      updateVehiclesInfo();
      drawScene();
      m_world->step();
      m_guiEnv->drawAll();
      m_world->debugDrawWorld();
      m_driver->endScene();
      break;
    default:
      break;
  }
  return ret;
}

void Race::drawScene()
{
  for(unsigned i=0; i<m_nCameras; i++) {
    CameraData * cd=m_cameraData[i];
    m_driver->setViewPort(cd->viewport);
    m_sceneManager->setActiveCamera(cd->camera);
    m_sceneManager->drawAll();
  }
  m_driver->setViewPort(m_wholeScreenViewPort);
}

void Race::updateRanking()
{
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

bool Race::updateKeyboard()
{
  EventReceiver * erec;
  bool ret=false;

  erec=ResourceManager::getInstance()->getEventReceiver();

  if(erec->OneShotKey(irr::KEY_KEY_C)) 
    restart();

  if(erec->OneShotKey(irr::KEY_ESCAPE))  {
    GM_LOG("toggling pause\n");
    togglePause();
  }

  if(erec->OneShotKey(irr::KEY_KEY_Q)) {
    ret=true;
    m_status=rs_notRunning;
  }

#ifdef ONLY_1_CAM
  if(erec->OneShotKey(irr::KEY_TAB)) {
    GM_LOG("changing camera\n");
    m_cameraAnim->nextCameraType();
  }

  if(erec->OneShotKey(irr::KEY_KEY_Y)) 
    restoreVehicle(m_vehicles[0]);

  if(erec->IsKeyDown(irr::KEY_KEY_W))
    m_cameraAnim->moveXY(0.,CAMERA_STEP);

  if(erec->IsKeyDown(irr::KEY_KEY_Z)) 
    m_cameraAnim->moveXY(0.,-CAMERA_STEP);

  if(erec->IsKeyDown(irr::KEY_KEY_A))
    m_cameraAnim->moveXY(CAMERA_STEP,0.);

  if(erec->IsKeyDown(irr::KEY_KEY_S)) 
    m_cameraAnim->moveXY(-CAMERA_STEP,0.);
#endif


  return ret;
}

#if 0
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
#endif


bool Race::gotoState(unsigned state)
{
  switch(state) {
    case rs_paused:
      if(m_status != rs_started)
        return false;
      for(unsigned i=0; i<m_nVehicles; i++) {
        CameraData * cd=m_vehicles[i].cameraData;
        if(cd) cd->cockpit->pause();
      }
      m_communicator->unshow();
      ResourceManager::getInstance()->showMenu("in-game");
      m_status=rs_paused;
      break;
    case rs_finished:
      m_communicator->show("race rank");
      for(unsigned i=0; i<m_nVehicles; i++) {
        CameraData * cd=m_vehicles[i].cameraData;
        if(cd) cd->cockpit->stop();
      }
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
        if(m_vehicles[i].cameraData) {
          m_vehicles[i].cameraData->cockpit->stop();
          m_vehicles[i].cameraData->cockpit->setVisible(true);
        }
      }
      // reset gui controls
      m_readySetGo->restart();
      m_status=rs_readySetGo;
      m_nFinishedVehicles=0;
      break;
    case rs_started:
      if(m_status==rs_paused) {
        m_status=rs_started;
        for(unsigned i=0; i<m_nVehicles; i++) {
          CameraData * cd=m_vehicles[i].cameraData;
          if(cd) cd->cockpit->unpause();
        }
        ResourceManager::getInstance()->hideMenu();
      } else if(m_status==rs_readySetGo) {
        for(unsigned i=0; i<m_nVehicles; i++) 
          m_rank[i]=i;
        for(unsigned i=0; i<m_nVehicles; i++) 
          m_vehicles[i].vehicle->setEnableControls(true);
        m_status=rs_started;
        for(unsigned i=0; i<m_nVehicles; i++) {
          CameraData * cd=m_vehicles[i].cameraData;
          if(cd) cd->cockpit->start();
          VehicleInfo & vinfo = m_vehicles[i];
          const std::vector<btVector3> & controlPoints=m_track->getControlPoints();
          btVector3 vehicleDirection = vinfo.vehicle->getChassisForwardDirection();
          btVector3 vehiclePosition = vectIrrToBullet(vinfo.vehicle->getChassisPos());
          m_vehicles[i].controller->init(controlPoints,vehicleDirection,vehiclePosition);
        }
      }
  }

  return m_status==state;
}

bool Race::addVehicle(IVehicle * vehicle,IVehicleController * controller, 
    const char * name,
    bool followed)

{
  assert(controller);
  if(m_nVehicles == max_vehicles)
    return false;
  if(m_track == 0)
    return false;

  m_vehicles[m_nVehicles].index=m_nVehicles;
  m_vehicles[m_nVehicles].vehicle=vehicle;
  m_vehicles[m_nVehicles].vehicle->setEnableControls(false);
  m_vehicles[m_nVehicles].waitingForLapTrigger=false;
  m_vehicles[m_nVehicles].controller=controller;
  m_vehicles[m_nVehicles].cameraData=0;
  if(!name) {
    char buffer[32];
    snprintf(buffer,32,"vehicle %d",m_nVehicles);
    m_vehicles[m_nVehicles].name=buffer;
  } else {
    m_vehicles[m_nVehicles].name=name;
  }

  if(followed) {
    CameraData * cameraData;
    assert(m_nCameras < max_cameras);

    cameraData = new CameraData(m_vehicles[m_nVehicles],m_guiEnv,m_device);
    m_cameraData[m_nCameras] = cameraData;
    m_vehicles[m_nVehicles].cameraData=cameraData;
    vehicle->setSpeedOMeter(cameraData->cockpit);
    cameraData->cockpit->setLap(1,m_totalLaps);

    m_nCameras++;
    ResourceManager::getInstance()->cfgGet("video/splitconfig",m_splitType);
    updateCamerasViewPort();
  }

  m_nVehicles++;

  recalcVehicleVehiclesStartPositions();

  return true;
}

void Race::updateCamerasViewPort()
{
  unsigned width,height;
  ResourceManager::getInstance()->getScreenHeight(height);
  ResourceManager::getInstance()->getScreenWidth(width);
  m_wholeScreenViewPort=irr::core::rect<irr::s32>(0,0,width,height);

  if(m_nCameras==1) {
    // one camera -> whole screen
    m_cameraData[0]->setViewPort(0,0,width,height);
  } else if(m_nCameras==2) {
    if(m_splitType == 0) { // vertical
      m_cameraData[0]->setViewPort(0,0,width/2,height);
      m_cameraData[1]->setViewPort(1+width/2,0,width,height);
    } else { // horizontal
      m_cameraData[0]->setViewPort(0,0,width,height/2);
      m_cameraData[1]->setViewPort(0,1+height/2,width,height);
    }
  } else {
    assert(0 && "Not implemented");
  }
}

void Race::lapTriggered(void * userdata)
{
  VehicleInfo * vinfo=(VehicleInfo*)userdata;

  if(vinfo->waitingForLapTrigger) {
    vinfo->waitingForLapTrigger=false;
    vinfo->lapNumber++;
#if ONLY_1_CAM
    if(vinfo->index==m_followedVehicleIndex) {
      m_communicator->show("lap %d",vinfo->lapNumber+1);
      m_cockpit->setLap(vinfo->lapNumber+1,m_totalLaps);
    }
#endif
    if(vinfo->lapNumber == m_totalLaps) 
      vehicleFinished(*vinfo);
  }
}

void Race::recalcVehicleVehiclesStartPositions()
{
  const double vehicleSpace=2.;
  double thlen=(m_nVehicles-1)*vehicleSpace*.5;
  double angle=m_track->getStartRotation();

  for(unsigned i=0; i<m_nVehicles; i++) {
    double zp= (i * vehicleSpace) - thlen;
    double xp;

    xp = sin(angle) * zp;
    zp = cos(angle) * zp;

    m_vehicles[i].startPosition.X = m_track->getStartPosition().X + xp;
    m_vehicles[i].startPosition.Y = m_vehicles[i].vehicle->getRestHeight();
     // m_track->getStartPosition().Y ;

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
  double    a = -atan2(c,s);

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
    GM_LOG("unpausing\n");
    m_communicator->unshow();
    gotoState(rs_started);
  } else if(m_status==rs_started)  {
    GM_LOG("pausing\n");
    gotoState(rs_paused); 
  }
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

  if( a.ctrlPntDistance <  b.ctrlPntDistance )
    return -1;

  if( a.ctrlPntDistance >  b.ctrlPntDistance )
    return 1;

  return 0;
}

void Race::setSplitModality(int modality)
{
  m_splitType=modality;
  updateCamerasViewPort();
}

void Race::unprepare()
{
  m_track->unload();

  for(unsigned i=0; i<m_nVehicles; i++) {
    VehicleInfo & vinfo=m_vehicles[i];
    vinfo.vehicle->unuse(IVehicle::USE_PHYSICS|IVehicle::USE_GRAPHICS);
  }
  m_nVehicles=0;

  for(unsigned i=0; i<m_nCameras; i++) {
    delete m_cameraData[i];
  }
  m_nCameras=0;
}


Race::CameraData::CameraData(
    const VehicleInfo & vinfo, 
    irr::gui::IGUIEnvironment * guiEnv,
    irr::IrrlichtDevice * device)
{
  cameraAnim=new
    VehicleCameraAnimator(vinfo.vehicle);
  camera = device->getSceneManager()->addCameraSceneNode();
  camera->grab();
  camera->addAnimator(cameraAnim);

  cockpit=0;

  unsigned width,height;
  ResourceManager::getInstance()->getScreenHeight(height);
  ResourceManager::getInstance()->getScreenWidth(width);

  viewport=irr::core::rect<irr::s32>(0,0,width,height);

  irr::s32 px1,px2,py1,py2;
  px1=1024-width;     py1=(576-height)/2-50;
  px2=px1+width;
  py2=py1+height;

  cockpit = new GuiCockpit(
      guiEnv,
      guiEnv->getRootGUIElement(),
      3,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),
      device->getTimer());
  cockpit->grab();
  vinfo.vehicle->setSpeedOMeter(cockpit);
}


void Race::CameraData::setViewPort(const irr::core::rect<irr::s32> & _viewport)
{
  viewport = _viewport;
  irr::f32 aspectRatio = (irr::f32)viewport.getWidth() / (irr::f32)viewport.getHeight();
  camera->setAspectRatio(aspectRatio);

  if(cockpit)  {
    irr::s32 px,py;
    px=viewport.UpperLeftCorner.X;
    py=viewport.LowerRightCorner.Y - 120; // TODO: ...
    cockpit->setPosition(px,py);
  }
}


void Race::CameraData::setViewPort(irr::s32 x1, irr::s32 y1, irr::s32 x2, irr::s32 y2)
{
  setViewPort(irr::core::rect<irr::s32>(x1,y1,x2,y2));
}


Race::CameraData::~CameraData()
{
  camera->remove();
  camera->drop();
  cameraAnim->drop();
  cockpit->remove();
  cockpit->drop();
  camera=0;
  cameraAnim=0;
  cockpit=0;
}

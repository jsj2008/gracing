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

#include "GuiCockpit.h"

extern IrrDebugDrawer * debugDrawer;

#define CAMERA_STEP 0.05

/* versione due */
static void draw2DImage_v2(irr::video::IVideoDriver *driver, 
    irr::video::ITexture* texture , 
    irr::core::rect<irr::s32> sourceRect, 
    irr::core::position2d<irr::s32> position, 
    irr::core::position2d<irr::s32> rotationPoint, 
    irr::f32 rotation, 
    irr::core::vector2df scale, 
    bool useAlphaChannel, 
    irr::video::SColor color) 
{ 

   irr::video::SMaterial material; 

   // Store and clear the projection matrix 
   irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION); 
   driver->setTransform(irr::video::ETS_PROJECTION,irr::core::matrix4()); 

   // Store and clear the view matrix 
   irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW); 
   driver->setTransform(irr::video::ETS_VIEW,irr::core::matrix4()); 

   // Store and clear the world matrix 
   irr::core::matrix4 oldWorldMat = driver->getTransform(irr::video::ETS_WORLD); 
   driver->setTransform(irr::video::ETS_WORLD, irr::core::matrix4()); 

   // Find the positions of corners 
   irr::core::vector2df corner[4]; 

   corner[0] = irr::core::vector2df(position.X,position.Y); 
   corner[1] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y); 
   corner[2] = irr::core::vector2df(position.X,position.Y+sourceRect.getHeight()*scale.Y); 
   corner[3] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y+sourceRect.getHeight()*scale.Y); 

   // Rotate corners 
   if (rotation != 0.0f) 
      for (int x = 0; x < 4; x++) 
         corner[x].rotateBy(rotation,irr::core::vector2df(rotationPoint.X, rotationPoint.Y)); 


   // Find the uv coordinates of the sourceRect 
   irr::core::vector2df uvCorner[4]; 
   uvCorner[0] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[1] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[2] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.LowerRightCorner.Y); 
   uvCorner[3] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.LowerRightCorner.Y); 
   for (int x = 0; x < 4; x++) { 
      float uvX = uvCorner[x].X/(float)texture->getSize().Width; 
      float uvY = uvCorner[x].Y/(float)texture->getSize().Height; 
      uvCorner[x] = irr::core::vector2df(uvX,uvY); 
   } 

   // Vertices for the image 
   irr::video::S3DVertex vertices[4]; 
   irr::u16 indices[6] = { 0, 1, 2, 3 ,2 ,1 }; 

   // Convert pixels to world coordinates 
   float screenWidth = driver->getScreenSize().Width; 
   float screenHeight = driver->getScreenSize().Height; 
   for (int x = 0; x < 4; x++) { 
      float screenPosX = ((corner[x].X/screenWidth)-0.5f)*2.0f; 
      float screenPosY = ((corner[x].Y/screenHeight)-0.5f)*-2.0f; 
      vertices[x].Pos = irr::core::vector3df(screenPosX,screenPosY,1); 
      vertices[x].TCoords = uvCorner[x]; 
      vertices[x].Color = color; 
   } 

   material.Lighting = false; 
   material.ZWriteEnable = false; 
   material.ZBuffer = false; 
   material.TextureLayer[0].Texture = texture; 
   material.MaterialTypeParam = irr::video::pack_texureBlendFunc(irr::video::EBF_SRC_ALPHA, 
       irr::video::EBF_ONE_MINUS_SRC_ALPHA, 
       irr::video::EMFN_MODULATE_1X, 
       irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR); 

   if (useAlphaChannel) 
      material.MaterialType = irr::video::EMT_ONETEXTURE_BLEND; 
   else 
      material.MaterialType = irr::video::EMT_SOLID; 

   driver->setMaterial(material); 
   driver->drawIndexedTriangleList(&vertices[0],4,&indices[0],2); 

   // Restore projection and view matrices 
   driver->setTransform(irr::video::ETS_PROJECTION,oldProjMat); 
   driver->setTransform(irr::video::ETS_VIEW,oldViewMat); 
   driver->setTransform(irr::video::ETS_WORLD,oldWorldMat); 
}






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


  width=400; height=100;
  px1=1024-width;     py1=0;

  px2=px1+width;
  py2=py1+height;

  m_cronometer = new GuiCronometer(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2),device->getTimer());

  width=1024; height=100;
  px1=1024-width;     py1=(576-height)/2-50;
  px2=px1+width;
  py2=py1+height;

  m_communicator = new GuiCommunicator(m_guiEnv,m_guiEnv->getRootGUIElement(),2,
      irr::core::rect<irr::s32>(px1,py1,px2,py2));
  assert(m_cronometer);

  m_nVehicles=0;
  m_track=0;
  m_totalLaps=2;
  m_camera = device->getSceneManager()->addCameraSceneNode();
  m_device = device;
  m_cameraAnim = 0;
  m_followedVehicleIndex=invalidVehicleIndex;

  width=200; height=100;
  px1=5;     py1=750-height;

  px2=px1+width;
  py2=py1+height;
  GuiCockpit * cockpit = new GuiCockpit(m_guiEnv,m_guiEnv->getRootGUIElement(),3,
      irr::core::rect<irr::s32>(px1,py1,px2,py2));

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
  for(unsigned i=0; i<m_nVehicles; i++) {
  }
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

  m_followedVehicleIndex ++;

  if(m_followedVehicleIndex == m_nVehicles)
    m_followedVehicleIndex = 0;

  m_cameraAnim->changeVehicle(m_vehicles[m_followedVehicleIndex].vehicle);
}


bool Race::gotoState(unsigned state)
{
  switch(state) {
    case rs_paused:
      if(m_status != rs_started)
        return false;
      m_status=rs_paused;
      break;
    case rs_finished:
      GM_LOG("race finished\n");
      m_status=rs_finished;
      exit(0);
      break;
    case rs_readySetGo:
      // reset vehicles
      for(unsigned i=0; i<m_nVehicles; i++)  {
        m_vehicles[i].vehicle->use(IVehicle::USE_GRAPHICS | IVehicle::USE_PHYSICS);

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
        m_vehicles[i].raceFinished=false;
        m_track->registerLapCallback(this, 
            m_vehicles[i].vehicle, 
            &(m_vehicles[i]));
      }
      // reset gui controls
      m_readySetGo->restart();
      m_cronometer->stop();
      m_status=rs_readySetGo;
      m_nFinishedVehicles=0;
      break;
    case rs_started:
      if(m_status==rs_paused) {
        m_status=rs_started;
      } else if(m_status==rs_readySetGo) {
        for(unsigned i=0; i<m_nVehicles; i++) 
          m_vehicles[i].vehicle->setEnableControls(true);
        m_status=rs_started;
        m_cronometer->start();
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
    m_communicator->show("lap %d",vinfo->lapNumber+1);
    vinfo->lapNumber++;
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
  if(a.lapNumber > b.lapNumber) 
    return 1;
  else if(a.lapNumber < b.lapNumber)
    return -1;

  // the vehicles are on the same lap number
  return 0;
}

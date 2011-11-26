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
#include "gmlog.h"
#include "TrackChooser.h"
#include "ResourceManager.h"

DefaultTrackChooser::DefaultTrackChooser(irr::IrrlichtDevice * device,
        PhyWorld * world)
  : ITrackChooser(device,world)
{
  ResourceManager * resmanager=
    ResourceManager::getInstance();
  // TODO: choose the best image size 
  //       depending on the screen resolution
  std::string bgpath = resmanager->getResourcePath() + "/background.png";

  unsigned w,h;
  resmanager->getScreenWidth(w);
  resmanager->getScreenHeight(h);

  m_background = resmanager->getVideoDriver()->getTexture(bgpath.c_str());

  if(m_background) {
    m_background->grab();
    m_dstRect.UpperLeftCorner.X = 0;
    m_dstRect.UpperLeftCorner.Y = 0;
    m_dstRect.LowerRightCorner.X = w;
    m_dstRect.LowerRightCorner.Y = h;

    irr::core::dimension2d<irr::u32> dim=m_background->getSize();

    m_srcRect.UpperLeftCorner.X = 0;
    m_srcRect.UpperLeftCorner.Y = 0;
    m_srcRect.LowerRightCorner.X = dim.Width;
    m_srcRect.LowerRightCorner.Y = dim.Height;
  }

  std::string imgpath = resmanager->getResourcePath() + "/default-track-shot.png";
  m_defaultTrackShot =
    resmanager->getVideoDriver()->getTexture(imgpath.c_str());

  m_trackShot = 0;
  m_animating = false;
  m_currentTrackIndex = 0;
  setTrack(0);

  if(!m_defaultTrackShot) {
    GM_LOG("cannot load the default track shot\n");
  }
}

void DefaultTrackChooser::prepare()
{
  m_currentTrackIndex=0;
}

void DefaultTrackChooser::unprepare()
{
}

void DefaultTrackChooser::setTrack(unsigned index)
{
  if(m_animating)
    return;
  ResourceManager * resmanager=ResourceManager::getInstance();

  m_currentTrackIndex = index;
  std::string filename;

  resmanager->getTracksList()[m_currentTrackIndex]->
      getShotImageFilename(filename);

  m_trackShot=
    resmanager->getTracksList()[m_currentTrackIndex]->getShotImage();
  if(m_trackShot == 0)
    m_trackShot = m_defaultTrackShot;
}

bool DefaultTrackChooser::step()
{
  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  if(m_background)
    m_driver->draw2DImage (
        m_background,
        m_dstRect,
        m_srcRect,
        0,
        0,
        true);
  m_guiEnv->drawAll();
  m_driver->endScene();
#if 0
  ResourceManager * resman=ResourceManager::getInstance();
  EventReceiver * erec;
  erec=resman->getEventReceiver();


  bool done=false;
  m_driver->beginScene(true, true, irr::video::SColor(255,100,101,140));
  if(m_background)
    m_driver->draw2DImage (
        m_background,
        m_dstRect,
        m_srcRect,
        0,
        0,
        true);
  m_guiEnv->drawAll();

  // !?!?!?

  irr::core::rect<irr::s32> rect(0,0,300,100);

  resman->getSystemFontBig()->draw(
      resman->getTracksList()[m_currentTrackIndex]->getName().c_str(),
      rect,
      irr::video::SColor(255,255,255,255),
      false,false);

  if(m_trackShot) {
    irr::core::rect<irr::s32>  srcRect;
    irr::core::rect<irr::s32>  dstRect;
    dstRect.UpperLeftCorner.X = 100;
    dstRect.UpperLeftCorner.Y = 100;
    dstRect.LowerRightCorner.X = 200;
    dstRect.LowerRightCorner.Y = 200;


    irr::core::dimension2d<irr::s32> dim;
    dim=m_trackShot->getSize();

    srcRect.UpperLeftCorner.X = 0 ;
    srcRect.UpperLeftCorner.Y = 0;
    srcRect.LowerRightCorner.X = dim.Width;
    srcRect.LowerRightCorner.Y = dim.Height;

    m_driver->draw2DImage(
      m_trackShot,
      dstRect,srcRect,0,0,true);
  }
  // !?!?!?

  m_driver->endScene();

  if(erec->OneShotKey(irr::KEY_LEFT)) {
    unsigned index;
    if(m_currentTrackIndex)
      index=m_currentTrackIndex-1;
    else
      index=ResourceManager::getInstance()->getTracksList().size()-1;
    setTrack(index);
  }

  if(erec->OneShotKey(irr::KEY_RIGHT)) {
    unsigned index;
    if(m_currentTrackIndex == 
        ResourceManager::getInstance()->getTracksList().size()-1)
      index=0;
    else
      index=m_currentTrackIndex+1;
    setTrack(index);
  }

  if(erec->OneShotKey(irr::KEY_RETURN)) {
  
    resman->setUsedTrack(
      resman->getTracksList()[m_currentTrackIndex]);

    done=true;
  }

  return done;
#endif
  bool done=false;
  return done;
}

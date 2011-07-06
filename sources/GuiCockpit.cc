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

#include <string>

#include "ResourceManager.h"
#include "gmlog.h"
#include "XmlNode.h"
#include "Util.hh"

#include "GuiCockpit.h"

#define MANIFEST_NAME "COCKPIT"

#if 0
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
#endif

GuiCockpit::GuiCockpit(
    irr::gui::IGUIEnvironment* environment,
		irr::gui::IGUIElement* parent, irr::s32 id, const irr::core::rect<irr::s32>& rectangle,
    irr::ITimer * timer)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  ResourceManager * resmanager=ResourceManager::getInstance();
  m_guiEnv=environment;

  IsVisible=false;

  XmlNode * root=0;

  std::string path_name;
  resmanager->getResourceCompletePath("cockpit.zip",path_name);

  irr::io::path mypath(path_name.c_str());
  bool res=resmanager->getFileSystem()->addFileArchive(mypath);
  assert(res);

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  assert(xml); // TODO: should not be an assert

  root=new XmlNode(xml);

  if(!root) {
    GM_LOG("Cannot load cockpit \n");
  }

  assert(res);

  std::vector<XmlNode*> nodes;
  XmlNode * node;
  root->getChildren(nodes);

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  for(unsigned i=0; i<11; i++)
    m_digitsPresence[i]=false;

  for(unsigned i=0; i<nodes.size(); i++) {
    node=nodes[i];

    if(node->getName() == "texture") {
      std::string src;
      int idx; 
      node->get("src",src);
      node->get("idx",idx);
      irr::video::ITexture *      texture = 0;
      texture = driver->getTexture(src.c_str());
      assert(texture);
      texture->grab();
      m_textures.push_back(texture);
    } else if(node->getName() == "body") {
      node->get("texture",m_bodyTidx);
      node->get("rect",m_bodyRect);
      node->get("hand-center",m_handPosition);
    } else if(node->getName() == "lap") {
      node->get("pos",m_lapPosition);
    } else if(node->getName() == "rank") {
      node->get("pos",m_rankPosition);
    } else if(node->getName() == "timer") {
      node->get("pos",m_timerPosition);
    } else if(node->getName() == "hand") {
      node->get("texture",m_handTidx);
      node->get("rect",m_handRect);
      node->get("center",m_handCenter);
    } else if(node->getName() == "digits") {
      std::vector<XmlNode*> digits;
      node->getChildren(digits);
      XmlNode * digit;

      for(unsigned j=0; j < digits.size(); j++) {
        std::string s;
        char c;      
        unsigned index;
        digit=digits[j];
        digit->get("d",s);

        if(!strlen(s.c_str())) continue;

        c=s.c_str()[0];

        if(isdigit(c))
          index=c-'0';
        else if(c == ':') 
          index=10;
        else
          continue;

        digit->get("r",m_digitsRect[index]);
        digit->get("t",m_digitsTexture[index]);
        m_digitsPresence[index]=true;
      }
    } 
#if 0
    else {
      GM_LOG("- node: '%s'\n",node->getName().c_str());
    }
#endif
  }

  // TODO
  // this must be checked before the assert
  // and if not verified, the cockpit must
  // not loaded
  assert(m_bodyTidx < m_textures.size());
  for(unsigned i=0; i<11; i++) {
    assert(m_digitsPresence[i]);
  }

  AbsoluteRect.LowerRightCorner.X = AbsoluteRect.UpperLeftCorner.X + 200;
  AbsoluteRect.LowerRightCorner.Y = AbsoluteRect.UpperLeftCorner.Y + 115;

  m_handDstRect.UpperLeftCorner = AbsoluteRect.UpperLeftCorner + m_handPosition - m_handCenter;
  m_handDstRect.LowerRightCorner = m_handDstRect.UpperLeftCorner + m_handRect.getWidth();

  m_handCenter += m_handDstRect.UpperLeftCorner;
  m_timerPosition += AbsoluteRect.UpperLeftCorner;
  m_lapPosition += AbsoluteRect.UpperLeftCorner;
  m_rankPosition += AbsoluteRect.UpperLeftCorner;

  res=resmanager->getFileSystem()->removeFileArchive(resmanager->getFileSystem()->getAbsolutePath(mypath));

  m_timer=timer;
  m_started=false;
  m_paused=false;
  m_totalLaps=0;
  m_lap=0;
  m_rank=0;
  m_totRank=0;

}

GuiCockpit::~GuiCockpit()
{
  for(unsigned i=0; i < m_textures.size(); i++)
    m_textures[i]->drop();
}

void GuiCockpit::drawString(const char * str, const irr::core::vector2d<irr::s32> & pos)
{
  const char * c;
  unsigned index=0;
  unsigned tidx=0;
  irr::core::rect<irr::s32> dstRect;

  dstRect.UpperLeftCorner=pos;
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  for(c=str; *c; c++) {
    if(isdigit(*c))
      index=*c - '0';
    else if(*c == ':') 
      index=10;
    else
      continue;

    tidx=m_digitsTexture[index];

    dstRect.LowerRightCorner=
      dstRect.UpperLeftCorner + m_digitsRect[index].getSize();


    driver->draw2DImage (
        m_textures[tidx],
        dstRect,
        m_digitsRect[index],
        0,
        0, //irr::video::SColor(255,255,255,255),
        true);

    dstRect.UpperLeftCorner.X = dstRect.LowerRightCorner.X;
  }
}

void GuiCockpit::draw()
{
  if(!IsVisible)
    return;
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  // draw the body
  driver->draw2DImage (
      m_textures[m_bodyTidx],
      AbsoluteRect,
      m_bodyRect,
      0,
      0, //irr::video::SColor(255,255,255,255),
      true);

  // draw the hand
  static double angle=0.;
  angle+=1.;
  Util::draw2DImage_v2(driver,
    m_textures[m_handTidx],
    m_handRect,
    m_handDstRect.UpperLeftCorner,
    m_handCenter,
    2.*getValue(), // rotation
    irr::core::vector2df(1.,1.),  // scale
    true,
    irr::video::SColor(255,255,255,255));

  char buffer[64];

  // draw time
  unsigned time;
  if(m_paused)
    time=m_showedTime;
  else if(m_started) 
      time=m_timer->getRealTime()-m_startTime;
  else
      time=0;
  
  unsigned cs,s,m;

  cs=(time / 10);
  s=cs / 100;
  m=s / 60;
  cs=cs % 100;
  s=s % 60;

  snprintf(buffer,64,"%02d:%02d:%02d",m,s,cs);
  drawString(buffer,m_timerPosition);

  // draw lap
  snprintf(buffer,64,"%02d:%02d",m_lap,m_totalLaps);
  drawString(buffer,m_lapPosition);

  // draw rank
  snprintf(buffer,64,"%02d:%02d",m_rank,m_totRank);
  drawString(buffer,m_rankPosition);
}

void GuiCockpit::unpause()
{
  m_paused=false;
  m_startTime+=m_timer->getRealTime() - m_pausedTime;
}

void GuiCockpit::pause()
{
  m_paused=true;
  m_pausedTime=m_timer->getRealTime();
  m_showedTime=m_pausedTime - m_startTime;
}

void GuiCockpit::stop()
{
  m_started=false;
}

void GuiCockpit::start()
{
  m_started=true;
  IsVisible=true;
  m_startTime=m_timer->getRealTime();
}

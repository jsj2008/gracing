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
#include <assert.h>

#include "gmlog.h"
#include "GuiFrame.h"
#include "ResourceManager.h"

#define MANIFEST_NAME "FRAME"

/*
GuiFrame::GuiFrame(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
  */
GuiFrame::GuiFrame(const irr::core::rect<irr::s32> &rectangle)
{
  ResourceManager * resmanager=ResourceManager::getInstance();
  std::string path_name;
  resmanager->getResourceCompletePath("frame.zip",path_name);

  m_requestedRectangle=rectangle;
  m_backgroundColor=irr::video::SColor(100,100,100,100);


#if 0
  irr::io::path mypath(path_name.c_str());
  bool res=resmanager->getFileSystem()->addFileArchive(mypath);
  assert(res);

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  assert(xml); // TODO: should not be an assert
  
  XmlNode * root=new XmlNode(xml);
  //load(root);
#endif 

  adjustPositions();
}

void GuiFrame::setSize(const irr::core::rect<irr::s32> & size)
{
  m_requestedRectangle=size;
  adjustPositions();
}

void GuiFrame::load(XmlNode * root)
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  std::vector<XmlNode*> nodes;

  root->getChildren(nodes);


  for(unsigned i=0; i<nodes.size(); i++) {
    XmlNode * node;
    node=nodes[i];
    if(node->getName() == "top-left-border") {
      unsigned a=bp_top_left; 
      node->get("texture",m_angles[a].tidx);
      node->get("size",m_angles[a].size);
      node->get("rect",m_angles[a].srcRect);
    } else if(node->getName() == "top-right-border") {
      unsigned a=bp_top_right; 
      node->get("texture",m_angles[a].tidx);
      node->get("size",m_angles[a].size);
      node->get("rect",m_angles[a].srcRect);
    } else if(node->getName() == "bottom-left-border") {
      unsigned a=bp_bottom_left; 
      node->get("texture",m_angles[a].tidx);
      node->get("size",m_angles[a].size);
      node->get("rect",m_angles[a].srcRect);
    } else if(node->getName() == "bottom-right-border") {
      unsigned a=bp_bottom_right; 
      node->get("texture",m_angles[a].tidx);
      node->get("size",m_angles[a].size);
      node->get("rect",m_angles[a].srcRect);
    } else if(node->getName() == "fill") {
      // ...
    } else if(node->getName() == "texture") {
      irr::video::ITexture * texture;
      std::string src;
      node->get("src",src);
      driver->getTexture(src.c_str());
      m_textures.push_back(texture);
    }
  }

  adjustPositions();
}

void GuiFrame::adjustPositions()
{
  m_fillRectangle=m_requestedRectangle;

  // top left
#if 0
  m_angles[bp_top_left].dstRect.UpperLeftCorner =
    m_fillRectangle.UpperLeftCorner - srcRect.getSize();
  m_angles[bp_top_left].dstRect.LowerRightCorner =
    m_angles[bp_top_left].dstRect.UpperLeftCorner + srcRect.getSize();

  // top right
  m_angles[bp_top_right].dstRect.UpperLeftCorner.X =
    m_fillRectangle.UpperLeftCorner.X;
  m_angles[bp_top_right].dstRect.UpperLeftCorner.Y =
    m_fillRectangle.UpperLeftCorner.Y - srcRect.getSize().Y;
  m_angles[bp_top_right].dstRect.LowerRightCorner =
    m_angles[bp_top_left].dstRect.UpperLeftCorner + srcRect.getSize();

 // bottom right 
  m_angles[bp_top_left].dstRect.UpperLeftCorner =
    m_fillRectangle.LowerRightCorner + srcRect.getSize();

  m_angles[bp_top_right].dstRect.LowerRightCorner =
    m_angles[bp_top_left].dstRect.UpperLeftCorner + srcRect.getSize();
#endif

}

void GuiFrame::draw()
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  // draw background

  driver->draw2DRectangle (
      m_backgroundColor,m_fillRectangle);

#if 0
    // draw angles
    for(unsigned i=0; i<4; i++) {
      driver->draw2DImage(
          m_textures[m_angles[i].tidx],
          m_angles[i].dstRect.
          m_angles[i].srcRect,
          0,
          irr::video::SColor(255,255,255,255),
          true);
    }
#endif
}

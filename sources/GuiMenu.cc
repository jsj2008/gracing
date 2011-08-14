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
#include "GuiMenu.h"
#include "Util.hh"

#define MANIFEST_NAME "THEME"

GuiTheme::GuiTheme(const char * filename)
{
  irr::io::IFileSystem * fileSystem = ResourceManager::getInstance()->getFileSystem();
  std::string respat=ResourceManager::getInstance()->getResourcePath() + filename;

  irr::io::path mypath(respat.c_str());

  fileSystem->addFileArchive(mypath);

  bool res=ResourceManager::getInstance()->getFileSystem()->addFileArchive(mypath);

  if(!res) 
    return; // !!!

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  m_root=new XmlNode(xml);
  assert(m_root && m_root->getName() == "theme"); // TODO: this must not be an assert!!!

  // load images 
  std::vector<XmlNode*> nodes;
  m_root->getChildren("img",nodes);

  for(unsigned i=0; i<nodes.size(); i++) {
    XmlNode * node=nodes[i];
    std::string src;
    unsigned id;
    // TODO: use the "id" field to enter the
    //       image into the m_images vector.
    //       up to now the id is taken from
    //       the order of the images into the xml file
    node->get("src",src);
    node->get("id",id); 
    GuiImage * texture = 0;
    texture = ResourceManager::getInstance()->getVideoDriver()->getTexture(src.c_str());
    if(texture) {
      m_images.push_back(texture);
    }
  }

  res=fileSystem->removeFileArchive(fileSystem->getAbsolutePath(mypath));

  assert(res);
#if 0
  nodes.clear();
  m_root->getChildren("img",nodes);
  
  for(unsigned i=0; i<nodes.size(); i++) {
    GM_LOG("[%d] %s\n",i,nodes[i]->getName().c_str());
  }
#endif
}

const XmlNode * GuiTheme::getNode(const char * name)
{
  return m_root->getChild(name);
}

GuiMenu::GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  ResourceManager *  resman=ResourceManager::getInstance();

  m_frame=new GuiFrame(rectangle);
  m_hasFrame=true;
  m_growSize=true;
  m_font= ResourceManager::getInstance()->getSystemFontBig();
  m_policy=new GuiContainerPolicy_GrowVertical();

  m_focusedItem = m_items.size();

  m_theme=new GuiTheme("theme-default.zip");

  _H(m_dimension)=100;
  _W(m_dimension)=600;
  _X(m_position)=0;
  _Y(m_position)=0;

  // tmp tmp tmp
  unsigned width;
  resman->getScreenWidth(width);
  width-=100;
  irr::core::rect<irr::s32> rect(0,0,width,100);
  AbsoluteRect=rect;
  // tmp tmp tmp

  refreshSize();
}

void GuiMenu::draw()
{
  if(m_hasFrame && m_frame)
    m_frame->draw();


  if(m_focusedItem < m_items.size()) {
    IGuiMenuItem * item=m_items[m_focusedItem];

    if(item->selfDrawFocused()) {
      // TODO: call the draw focus
    } else {
      irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

      GuiRect rect= item->getRectangle();
      irr::video::SColor color(200,200,200,200);
      driver->draw2DRectangle(color,rect);
    }
  }

  for(unsigned i=0; i < m_items.size(); i++) 
    m_items[i]->draw();

}

void GuiContainerPolicy_GrowHorizontal::applyPolicy(
    GuiPoint & position,
    GuiDimension & dimension,
    std::vector<IGuiMenuItem *> & items)
{
  GuiPoint pos = position;
  
  _W(dimension)=0;

  std::vector<IGuiMenuItem *>::iterator  it;

  for(it=items.begin(); it < items.end(); it++) {
    GuiDimension dim = (*it)->getPreferredSize();

    (*it)->setPosition(pos);

    _W(dimension) += _W(dim);

    _X(pos) += _W(dim);
    _H(dim) = _H(dimension);
  
    (*it)->setSize(dim);
  }
}

void GuiContainerPolicy_GrowVertical::applyPolicy(
    GuiPoint & position,
    GuiDimension & dimension,
    std::vector<IGuiMenuItem *> & items)
{
  GuiPoint pos = position;

  std::vector<IGuiMenuItem *>::iterator  it;

  _H(dimension)=0;

  for(it=items.begin(); it < items.end(); it++) {
    GuiDimension dim = (*it)->getPreferredSize();

    (*it)->setPosition(pos);

    _H(dimension) += _H(dim);
    _Y(pos) += _H(dim);

    if(_W(dim) > _W(dimension))
      _W(dimension) = _W(dim);
    else
      _W(dim) = _W(dimension);


#if 0
    _W(dim) = _W(dimension);
#endif

    GM_LOG("setting width: %d\n",_W(dim));
    (*it)->setSize(dim);

  }
}

void GuiMenu::refreshSize()
{
  if(m_growSize) {

    m_policy->applyPolicy(m_position,m_dimension,m_items);

    AbsoluteRect.UpperLeftCorner.X = 
      _X(m_position);

    AbsoluteRect.UpperLeftCorner.Y = 
      _Y(m_position);

    AbsoluteRect.LowerRightCorner.X = 
     AbsoluteRect.UpperLeftCorner.X + _W(m_dimension);

    AbsoluteRect.LowerRightCorner.Y = 
     AbsoluteRect.UpperLeftCorner.Y + _H(m_dimension);
  }

  m_frame->setSize(AbsoluteRect);
}

unsigned GuiMenu::pickupItemByPoint(const GuiPoint & point)
{
  unsigned i;

  for(i=0; i < m_items.size(); i++) {
    //_X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
    if(m_items[i]->isPointInside(point))
      break;
  }

  return i;
}

void GuiMenu::selectItemByPoint(const GuiPoint & point)
{
  m_focusedItem = pickupItemByPoint(point);
}

void GuiMenu::mouseEvent(const irr::SEvent::SMouseInput & MouseInput)
{
  switch(MouseInput.Event) {
    case irr::EMIE_LMOUSE_PRESSED_DOWN:
      {
#if 0
      GM_LOG("mouse down button at %d,%d\n",
          MouseInput.X,
          MouseInput.Y);
#endif
      }

      break;

    case irr::EMIE_LMOUSE_LEFT_UP:
      {
        unsigned i;
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        i = pickupItemByPoint(pnt);

        if(isItemIndexValid(i)) {
          m_items[i]->onMouseClick(pnt);
        }
      }
      break;

    case irr::EMIE_MOUSE_MOVED:
      {
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        selectItemByPoint(pnt);
      }
      break;
    default:
      break;
  }
}

GuiItemCheckbox * GuiMenu::addCheckbox(const std::wstring & caption)
{
  GuiItemCheckbox * st;

  st = new GuiItemCheckbox(caption);

  if(m_theme)
    st->setTheme(m_theme);

  m_items.push_back(st);
  refreshSize();

  m_focusedItem = m_items.size() - 1;

  return st;
}

GuiItemStaticText * GuiMenu::addStaticText(const std::wstring & caption)
{
  GuiItemStaticText * st;

  st = new GuiItemStaticText(caption);

  if(m_theme)
    st->setTheme(m_theme);

  m_items.push_back(st);
  refreshSize();

  return st;
}

void GuiMenu::centerOnTheScreen()
{
  unsigned width,height;

  ResourceManager::getInstance()->getScreenWidth(width);
  ResourceManager::getInstance()->getScreenHeight(height);

  _X(m_position) = (width - _W(m_dimension) ) / 2;
  _Y(m_position) = (height - _H(m_dimension) ) / 2;

  refreshSize();
}

////////////////////////////////////////////////// GUI STATIC TEXT START
GuiItemStaticText::GuiItemStaticText(const std::wstring & caption)
{
  m_caption = caption;
  m_font = ResourceManager::getInstance()->getSystemFontSmall();

  _X(m_position)=0;
  _Y(m_position)=0;
  setSize(m_font->getDimension(m_caption.c_str()));
}

GuiDimension GuiItemStaticText::getPreferredSize()
{
  return m_font->getDimension(m_caption.c_str());
}

void GuiItemStaticText::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);
}

void GuiItemStaticText::setTheme(GuiTheme * theme)
{
  const XmlNode * node = theme->getNode("static-text");
  std::string value;

  if(node && node->get("font",value)) {
    if(value == "big") 
      m_font = ResourceManager::getInstance()->getSystemFontBig();
    else if(value == "small") 
      m_font = ResourceManager::getInstance()->getSystemFontSmall();
    else if(value == "normal")
      m_font = ResourceManager::getInstance()->getSystemFont();
  }
}
////////////////////////////////////////////////// GUI STATIC TEXT END


GuiItemCheckbox::GuiItemCheckbox(const std::wstring & caption)
{
  m_caption=caption;
  m_checked=false;
  m_boxImage=0;
  m_checkerImage=0;

  m_font = ResourceManager::getInstance()->getSystemFont();
}

GuiDimension GuiItemCheckbox::getPreferredSize()
{
  GuiDimension fdim=m_font->getDimension(m_caption.c_str());

  if(m_boxImage) {
    GuiDimension bdim;

#if 0
    GM_LOG("before: ");
    _LOGDIM(fdim);
#endif

    //bdim = m_boxImage->getSize();
    _W(bdim) = _RW(m_boxSrcRect);
    _H(bdim) = _RH(m_boxSrcRect);

    _W(fdim) += _W(bdim);

    if(_H(fdim) < _H(bdim))
      _H(fdim) = _H(bdim);

#if 0
    GM_LOG("after: ");
    _LOGDIM(fdim);
#endif
  }

  return fdim;
}

void GuiItemCheckbox::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  if(m_boxImage) {
    driver->draw2DImage (
        m_boxImage,
        m_boxDstRect,
        m_boxSrcRect,
        0,
        0, //irr::video::SColor(255,255,255,255),
        true);
  }

  if(m_checked && m_checkerImage) {
    driver->draw2DImage (
        m_checkerImage,
        m_boxDstRect,
        m_checkerSrcRect,
        0,
        0, //irr::video::SColor(255,255,255,255),
        true);
  }

}

void GuiItemCheckbox::setTheme(GuiTheme * theme)
{
  const XmlNode * root = theme->getNode("checkbox");
  std::string value;
  unsigned idx;

  if(!root) // no checkbox theme present
    return;

  const XmlNode * boxNode = root->getChild("box");

  GuiRect  rect;
  if(boxNode) { 
    if(boxNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_boxSrcRect);

    if(boxNode->get("img",idx)) 
      m_boxImage = theme->getImage(idx);
  }

  const XmlNode * checkNode = root->getChild("check");

  if(checkNode) {
    if(checkNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_checkerSrcRect);

    if(checkNode->get("img",idx)) 
      m_checkerImage = theme->getImage(idx);
  }

  updateGeometry();
}

void GuiItemCheckbox::onMouseClick(const GuiPoint & pnt)
{
  m_checked = ! m_checked;
}

void GuiItemCheckbox::updateGeometry() 
{
  m_boxDstRect.UpperLeftCorner.X = 
    m_rectangle.LowerRightCorner.X - _RW(m_boxSrcRect);

  m_boxDstRect.UpperLeftCorner.Y = 
    m_rectangle.UpperLeftCorner.Y;

  m_boxDstRect.LowerRightCorner.X = 
    m_boxDstRect.UpperLeftCorner.X + _RW(m_boxSrcRect);

  m_boxDstRect.LowerRightCorner.Y = 
    m_boxDstRect.UpperLeftCorner.Y + _RW(m_boxSrcRect);

  
}





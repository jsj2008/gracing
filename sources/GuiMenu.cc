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
#include "GuiFrame.h"
#include "Util.hh"
#include "GuiItemCheckBox.h"
#include "GuiItemListBox.h"
#include "GuiItemStaticText.h"

#include <lunar.h>

#define MANIFEST_NAME "THEME"

GuiTheme::GuiTheme(const char * filename)
{
  irr::io::IFileSystem * fileSystem = ResourceManager::getInstance()->getFileSystem();
  std::string respat=ResourceManager::getInstance()->getResourcePath() + filename;

  irr::io::path mypath(respat.c_str());

  bool res=fileSystem->addFileArchive(mypath);

  GM_LOG("loading theme from '%s'\n",mypath.c_str());

  //bool res=ResourceManager::getInstance()->getFileSystem()->addFileArchive(mypath);

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
}

const XmlNode * GuiTheme::getNode(const char * name)
{
  return m_root->getChild(name);
}

IGuiMenuItem::IGuiMenuItem(const char * className)
{
  m_selectable = true;
  m_className = className;
  m_font = ResourceManager::getInstance()->getSystemFont();
}

void IGuiMenuItem::setTheme(GuiTheme * theme) 
{
  const XmlNode * node = theme->getNode(m_className.c_str());
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

void IGuiMenuItem::init(XmlNode * node) 
{
}

GuiMenu::GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  ResourceManager *  resman=ResourceManager::getInstance();

  m_frame=new GuiFrame(rectangle);
  m_hasFrame=true;
  m_isVisible=true;
  m_growSize=true;
  m_font= ResourceManager::getInstance()->getSystemFontBig();
  m_policy=new GuiContainerPolicy_GrowVertical();

  m_itemWhichCapturedMouse=0;

  m_focusedItem = m_items.size();

  m_theme=new GuiTheme("theme-default.zip");

  _H(m_dimension)=100;
  _W(m_dimension)=600;
  _X(m_position)=0;
  _Y(m_position)=0;

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  if (driver->queryFeature(irr::video::EVDF_RENDER_TO_TARGET)) {
    m_renderTarget = driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(256,256), "RTT1");
    //test->setMaterialTexture(0, rt); // set material of cube to render target

    GM_LOG("is able to render to texture\n");
  }
  

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
  if(!m_isVisible)
    return;

  if(m_hasFrame && m_frame)
    m_frame->draw();

  if(m_focusedItem < m_items.size()) {
    IGuiMenuItem * item=m_items[m_focusedItem];

    if(item->selfDrawFocused()) {
      item->drawFocus();
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
    if(m_items[i]->isPointInside(point))
      break;
  }

  return i;
}

void GuiMenu::selectItemByPoint(const GuiPoint & point)
{
  unsigned o= pickupItemByPoint(point);
  if(isItemIndexValid(o) && m_items[o]->isSelectable())
    m_focusedItem = o;
  else
    m_focusedItem = m_items.size();
}

void GuiMenu::mouseEvent(const irr::SEvent::SMouseInput & MouseInput)
{
  switch(MouseInput.Event) {
    case irr::EMIE_LMOUSE_PRESSED_DOWN:
      {
        unsigned i;
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        i = pickupItemByPoint(pnt);
        if(isItemIndexValid(i)) {
          IGuiMenuItem * item = m_items[i];
          if(item->onMouseLButton(true,pnt)) {
            m_itemWhichCapturedMouse=item;
          }
        }
      }
      break;

    case irr::EMIE_LMOUSE_LEFT_UP:
      if(m_itemWhichCapturedMouse) {
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        m_itemWhichCapturedMouse->onMouseLButton(false,pnt);
        m_itemWhichCapturedMouse=0;
      } else {
        unsigned i;
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        i = pickupItemByPoint(pnt);

        if(isItemIndexValid(i)) {
          IGuiMenuItem * item = m_items[i];
          item->onMouseLButton(false,pnt);
          item->onMouseClick(pnt);
        }
      }
      break;

    case irr::EMIE_MOUSE_MOVED:
      if(m_itemWhichCapturedMouse) {
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
         m_itemWhichCapturedMouse->onMouseMove(pnt);
      } else {
        GuiPoint pnt;
        _X(pnt) = MouseInput.X; _Y(pnt) = MouseInput.Y;
        // TODO: avoid double "selectItemByPoint" and "pickupItemByPoint"
        selectItemByPoint(pnt);

        unsigned i;
        i = pickupItemByPoint(pnt);

        if(isItemIndexValid(i)) {
          IGuiMenuItem * item = m_items[i];
          item->onMouseMove(pnt);
        }
      }
      break;
    default:
      break;
  }
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
////////////////////////////////////////////////// GUI STATIC TEXT END




///////////////////////////////////////
GuiItemSlider::GuiItemSlider(const std::wstring & caption)
  : IGuiMenuItem("slider")
{
  const unsigned defRangeLen=100;
  m_caption = caption;
  m_rangeLen = defRangeLen;
  m_font= ResourceManager::getInstance()->getSystemFont();
  m_handleFocused=false;
  m_draggingHandle=false;
  m_handleValue=0;
  m_minValue=0.;
  m_maxValue=1.;
}

void GuiItemSlider::init(XmlNode * node)
{
  node->get("min",m_minValue);
  node->get("max",m_minValue);
  // TODO: handle initial value
}

GuiDimension GuiItemSlider::getPreferredSize()
{
  GuiDimension dim;
  GuiDimension idim;

  if(m_font)
   dim = m_font->getDimension(m_caption.c_str());

  if(m_leftEdgeImage)  
    _W(dim) += _RW(m_leftEdgeDstRect);

  if(m_riteEdgeImage)  
    _W(dim) += _RW(m_riteEdgeDstRect);

  _W(dim) += m_rangeLen;

  return dim;
}

void GuiItemSlider::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  if(m_leftEdgeImage) 
    driver->draw2DImage (
        m_leftEdgeImage, m_leftEdgeDstRect,
        m_leftEdgeSrcRect, 0, 0, true); 

  if(m_leftEdgeImage) 
    driver->draw2DImage (
        m_riteEdgeImage, m_riteEdgeDstRect,
        m_riteEdgeSrcRect, 0, 0, true);


  if(m_fillerImage)
    Util::drawRectWithBackgroung(driver, 
        m_fillerImage,m_fillerDstRect,true,0,0);

  if(m_handleFocused) {
    irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
    irr::video::SColor c(100,100,100,255);
    driver->draw2DRectangle(c,m_handleDstRect);
  }

  if(m_handleImage) 
    driver->draw2DImage (
        m_handleImage, m_handleDstRect, m_handleSrcRect,
        0, 0, true);
}

void GuiItemSlider::updateGeometry()
{ 
  unsigned offset = (_RH(m_rectangle) - _RH(m_riteEdgeSrcRect)) / 2;

  if(m_font) {
    GuiDimension dim;
    dim=m_font->getDimension(m_caption.c_str());
    m_rangeLen = (_RW(m_rectangle) - _W(dim) - _RW(m_riteEdgeSrcRect) - _RW(m_leftEdgeSrcRect)) * 2 / 3;
  } else {
    m_rangeLen = 100;
  }

  _RMINX(m_riteEdgeDstRect) = _RMAXX(m_rectangle) - _RW(m_riteEdgeSrcRect);
  _RMINY(m_riteEdgeDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_riteEdgeDstRect) = _RMINX(m_riteEdgeDstRect) + _RW(m_riteEdgeSrcRect);
  _RMAXY(m_riteEdgeDstRect) = _RMINY(m_riteEdgeDstRect) + _RH(m_riteEdgeSrcRect);

  _RMINX(m_fillerDstRect) = _RMINX(m_riteEdgeDstRect) - m_rangeLen;
  _RMINY(m_fillerDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_fillerDstRect) = _RMINX(m_fillerDstRect) + m_rangeLen;
  _RMAXY(m_fillerDstRect) = _RMINY(m_fillerDstRect) + _RH(m_riteEdgeSrcRect);

  _RMINX(m_leftEdgeDstRect) = _RMINX(m_fillerDstRect) - _RW(m_leftEdgeSrcRect);
  _RMINY(m_leftEdgeDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_leftEdgeDstRect) = _RMINX(m_leftEdgeDstRect) + _RW(m_leftEdgeSrcRect);
  _RMAXY(m_leftEdgeDstRect) = _RMINY(m_leftEdgeDstRect) + _RH(m_leftEdgeSrcRect);

   updateHandlePosition();
}

void GuiItemSlider::updateHandlePosition()
{
  //unsigned gvalue=m_rangeLen / 2;
  unsigned hw = _RW(m_handleSrcRect) / 2;
  unsigned offset = (_RH(m_rectangle) - _RH(m_handleSrcRect)) / 2;

  _RMINX(m_handleDstRect) = (m_handleValue - hw) + _RMINX(m_fillerDstRect);
  _RMINY(m_handleDstRect) = _RMINY(m_rectangle) + offset;

  _RMAXX(m_handleDstRect) = _RMINX(m_handleDstRect) + _RW(m_handleSrcRect);
  _RMAXY(m_handleDstRect) = _RMINY(m_handleDstRect) + _RH(m_handleSrcRect);

}

void GuiItemSlider::setTheme(GuiTheme * theme)
{
  const XmlNode * root = theme->getNode("slider");

  const XmlNode * node;
  unsigned idx;
  std::string value;

  if(!root)
    return;

  node = root->getChild("left-edge");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_leftEdgeSrcRect);

    if(node->get("img",idx)) 
      m_leftEdgeImage = theme->getImage(idx);
  }

  node = root->getChild("right-edge");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_riteEdgeSrcRect);

    if(node->get("img",idx)) 
      m_riteEdgeImage = theme->getImage(idx);
  }

  node = root->getChild("handle");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_handleSrcRect);

    if(node->get("img",idx)) 
      m_handleImage = theme->getImage(idx);
  }

  node = root->getChild("filler");
  if(node) {
    if(node->get("img",idx)) 
      m_fillerImage = theme->getImage(idx);
  }
}

void GuiItemSlider::onMouseMove(const GuiPoint & point) 
{
  if(m_draggingHandle) {
    int dx = _X(point) - _X(m_lastMousePoint);

    m_handleValue += dx;

    m_lastMousePoint = point;

    if(m_handleValue < 0)
      m_handleValue = 0;
    else if(m_handleValue >= m_rangeLen)
      m_handleValue = m_rangeLen - 1;

    updateHandlePosition();
  }
}
bool GuiItemSlider::onMouseLButton(bool down, const GuiPoint & point) 
{
  if(down && _PINR(point, m_handleDstRect)) {
    m_lastMousePoint = point; 
    m_draggingHandle = true;
    return true;
  } else {
    m_draggingHandle = false;
  }
  return false;
}

void GuiItemSlider::drawFocus() 
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  irr::video::SColor color(200,200,200,200);
  irr::video::SColor c(100,100,100,255);
  driver->draw2DRectangle(color,m_rectangle);
}

double GuiItemSlider::getValue()
{
  return ((double)m_handleValue / (double)m_rangeLen) * (m_maxValue - m_minValue) + m_minValue;
}

void GuiMenu::setGroup(const std::wstring & name)
{
  m_items.clear();
  for(unsigned i=0; i<m_groups.size(); i++) 
    if(m_groups[i]->getName() == name) {
      m_groups[i]->fillVector(m_items);
    }

  refreshSize();
}

void GuiMenu::load(const std::string & xmlFileName)
{
  std::string respat=ResourceManager::getInstance()->getResourcePath() + xmlFileName;

  irr::io::path mypath(respat.c_str());

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(mypath.c_str());

  XmlNode * root=new XmlNode(xml);

  if(!root || root->getName() != "menu") {
    GM_LOG("invalid xml file\n");
    return;
  }

  GM_LOG("*****************************\n");
  GM_LOG("loading menu\n");
  GM_LOG("*****************************\n");

  std::vector<XmlNode*> nodes;
  root->getChildren(nodes);
  for(unsigned i=0; i<nodes.size(); i++) {
    XmlNode * node=nodes[i];
    if(node->getName() == "group") {
      GuiItemGroup * group = new GuiItemGroup(node);
      m_groups.push_back(group);
      if(m_theme) 
        group->setTheme(m_theme);
    } else if(node->getName() == "script") {
      std::string scriptSource;
      if(node->get("src",scriptSource)) {
        ResourceManager::getInstance()->lua_doFile(scriptSource.c_str());
      }
    }
  }
  
  xml->drop();
}

////////////////////////////////////////////////////////////////////////////////////

GuiMenu::GuiItemGroup::GuiItemGroup(XmlNode * root)
{

  m_name = L"noname";

  if(!root || root->getName() != "group") {
    GM_LOG("not a valid xml node\n");
    return;
  }

  root->get("name",m_name);

  std::vector<XmlNode*> nodes;
  root->getChildren(nodes);

  if(nodes.size() == 0) {
    GM_LOG("no item in group\n");
  }

  for(unsigned i=0; i<nodes.size(); i++) {
    XmlNode * node=nodes[i];
    IGuiMenuItem * item=0;

    item=GuiMenuItemFactory::build(node);

    if(item) 
      m_items.push_back(item);
  }
}
  

IGuiMenuItem * GuiMenuItemFactory::build(XmlNode * node)
{
  std::wstring caption;
  IGuiMenuItem * item;

  item=0;

  if(!node->get("caption",caption))
      caption=L"noname";

  if(node->getName() == LISTBOX_CLASSNAME)
    item = new GuiItemListBox(caption);
  else if(node->getName() == STATICTEXT_CLASSNAME) 
    item = new GuiItemStaticText(caption);
  else if(node->getName() == CHECKBOX_CLASSNAME) 
    item = new GuiItemCheckBox(caption);
  else if(node->getName() == "slider") 
    item = new GuiItemSlider(caption);

  if(item)
    item->init(node);

  return item;
}


GuiMenu::GuiItemGroup::GuiItemGroup(const std::wstring & name)
{
  m_name = name;
}



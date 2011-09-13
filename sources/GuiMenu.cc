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
#include "GuiItemListBoxEx.h"
#include "GuiItemStaticText.h"
#include "GuiItemSlider.h"

#include <lunar.h>

#define MANIFEST_NAME "THEME"

GuiTheme::GuiTheme(const char * filename)
{
  irr::io::IFileSystem * fileSystem = ResourceManager::getInstance()->getFileSystem();
  std::string respat=ResourceManager::getInstance()->getResourcePath() + filename;

  irr::io::path mypath(respat.c_str());

  bool res=fileSystem->addFileArchive(mypath);

  GM_LOG("loading theme from '%s'\n",mypath.c_str());

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
  if(!node)
    return;
  std::string value;
  if(node->get("font",value)) {
    if(value == "big") 
      m_font = ResourceManager::getInstance()->getSystemFontBig();
    else if(value == "small") 
      m_font = ResourceManager::getInstance()->getSystemFontSmall();
    else if(value == "normal")
      m_font = ResourceManager::getInstance()->getSystemFont();
  }

  node->get("selectable",m_selectable);

  // create a lua var with name from xml
  if(node->get("name",value)) {
    lua_State * L = ResourceManager::getInstance()->getLuaState();
    lua_pushstring(L,value.c_str());
    lua_pushstring(L, m_luaName);
    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_settable(L, LUA_GLOBALSINDEX);
  }

  // create a bind with config value
  if(node->get("bindConfig",value)) {
    m_boundCfgName=value;
  }
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

  m_focusedItem = 0xffff; // m_items.size();

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

void GuiMenu::selectNext()
{
  if(m_items.size() == 0)
    return;

  if(isItemIndexValid(m_focusedItem) && 
      m_items[m_focusedItem]->retainFocus())
    return;

  unsigned startFrom;
  if(isItemIndexValid(m_focusedItem)) {
    startFrom = m_focusedItem + 1;
    startFrom %= m_items.size();
  } else 
    startFrom=0;

  unsigned i;
  for( i=startFrom; ;  ) {
    if(m_items[i]->isSelectable())
      break;
    i++; i%=m_items.size();
    if( i == startFrom)
      break;
  }

  if(m_items[i]->isSelectable())
    m_focusedItem = i;
}

void GuiMenu::selectPrev()
{
  int startFrom;
  if(m_items.size() == 0)
    return;

  if(isItemIndexValid(m_focusedItem) && 
      m_items[m_focusedItem]->retainFocus())
    return;

  if(isItemIndexValid(m_focusedItem)) {
    if(m_focusedItem == 0) 
      startFrom = m_items.size() - 1;
    else
      startFrom = m_focusedItem - 1;
  } else 
    startFrom=0;

  int i;
  for(i=startFrom; ; )  {
    if(m_items[i]->isSelectable())
      break;
    if(i==0) i=m_items.size()-1; else i--;
  }

  if(m_items[i]->isSelectable()) 
    m_focusedItem = i;
}

void GuiMenu::keyboardEvent(const irr::SEvent::SKeyInput & keyInput)
{
  if(!m_isVisible)
    return;
  switch(keyInput.Key) {
    case irr::KEY_UP:
      if(!keyInput.PressedDown)
        selectPrev();
      break;

    case irr::KEY_DOWN:
      if(!keyInput.PressedDown)
        selectNext();
      break;
    default:
      break;
  }

  //  default:
      if(keyInput.PressedDown) {
        if(isItemIndexValid(m_focusedItem))
          m_items[m_focusedItem]->onKeyDown(keyInput);
      } else {
        if(isItemIndexValid(m_focusedItem)) {
          m_items[m_focusedItem]->onKeyUp(keyInput);
          m_items[m_focusedItem]->onKeyClick(keyInput);
        } 
      }
   //   break;
}

void GuiMenu::mouseEvent(const irr::SEvent::SMouseInput & MouseInput)
{
  if(!m_isVisible)
    return;
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
        unsigned focused=m_focusedItem;
        selectItemByPoint(pnt);

        if(focused != m_focusedItem && isItemIndexValid(focused)) {
          IGuiMenuItem * item = m_items[focused];
          item->onMouseLeave(pnt);
        }

        if(isItemIndexValid(m_focusedItem)) {
          IGuiMenuItem * item = m_items[m_focusedItem];
          if(m_focusedItem != focused)
            item->onMouseEnter(pnt);
          item->onMouseMove(pnt);
        }

#if 0
        unsigned i;
        i = pickupItemByPoint(pnt);

        if(isItemIndexValid(i)) {
          IGuiMenuItem * item = m_items[i];
          item->onMouseMove(pnt);
        }
#endif
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

///////////////////////////////////////

void GuiMenu::setGroup(const std::wstring & name)
{
  m_items.clear();
  for(unsigned i=0; i<m_groups.size(); i++) 
    if(m_groups[i]->getName() == name) {
      m_groups[i]->fillVector(m_items);
    }

  m_focusedItem = m_items.size();

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
  if(node->getName() == LISTBOXEX_CLASSNAME)
    item = new GuiItemListBoxEx(caption);
  else if(node->getName() == STATICTEXT_CLASSNAME) 
    item = new GuiItemStaticText(caption);
  else if(node->getName() == CHECKBOX_CLASSNAME) 
    item = new GuiItemCheckBox(caption);
  else if(node->getName() == SLIDER_CLASSNAME)
    item = new GuiItemSlider(caption);

  if(item)
    item->init(node);

  return item;
}


GuiMenu::GuiItemGroup::GuiItemGroup(const std::wstring & name)
{
  m_name = name;
}



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
#ifndef GUIMENU_H
#define GUIMENU_H
#include <irrlicht.h>

#include "XmlNode.h"
#include "EventReceiver.h"

#include <lunar.h>

class  GuiFrame;
class  GuiItemCheckBox;
class  GuiItemStaticText;
class  GuiItemListBox;

/////////////////////////////////////////////////
// irrlicht 'compatibility'
typedef irr::u32                          GuiU32;
typedef irr::s32                          GuiS32;
typedef irr::core::dimension2d<irr::u32>  GuiDimension;
typedef struct { GuiS32 x,y; }            GuiPoint;
typedef irr::core::rect<irr::s32>         GuiRect;
typedef irr::gui::IGUIFont                GuiFont;
typedef irr::video::ITexture              GuiImage;

#define _X(pnt)   pnt.x
#define _Y(pnt)   pnt.y

#define _W(dim)   dim.Width
#define _H(dim)   dim.Height

#define _RW(rect) ( (rect).LowerRightCorner.X - (rect).UpperLeftCorner.X )
#define _RH(rect) ( (rect).LowerRightCorner.Y - (rect).UpperLeftCorner.Y )
#define _RMINX(rect) (rect).UpperLeftCorner.X
#define _RMINY(rect) (rect).UpperLeftCorner.Y

#define _RMAXX(rect) (rect).LowerRightCorner.X
#define _RMAXY(rect) (rect).LowerRightCorner.Y

// point in rect !
#define _PINR(pnt, rect)  (\
    _X(pnt) >= _RMINX(rect) && _X(pnt) <= _RMAXX(rect) && \
    _Y(pnt) >= _RMINY(rect) && _Y(pnt) <= _RMAXY(rect)\
    )


#define _LOGRECT(rect) do { GM_LOG("rect: %d,%d,%d,%d\n",_RMINX(rect),_RMINY(rect),_RMAXX(rect),_RMAXY(rect)); } while(0)
#define _LOGDIM(dim) do { GM_LOG("dim: %d,%d\n",_W(dim),_H(dim)); } while(0)

/////////////////////////////////////////////////

class GuiTheme
{
  public:
    GuiTheme(const char * filename);

    const XmlNode * getNode(const char * node);

    inline GuiImage * getImage(unsigned index) {
      if(index < m_images.size()) 
        return m_images[index];
      return 0;
    }

  private:
    std::vector<GuiImage*> m_images;
    XmlNode *              m_root;
};

struct ImgElement
{
  GuiImage * image;
  GuiRect    srcRect;
  GuiRect    dstRect;
  GuiPoint   offset;

  ImgElement() { image=0; _X(offset)=0; _Y(offset)=0; }

  inline void draw()
  {
    irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
    if(image) 
      driver->draw2DImage (
          image, dstRect, srcRect,
          0, 0, true);
  }

  inline void init(GuiTheme * theme, const XmlNode * node)
  {
    if(!node)
      return;
    std::string value;
    unsigned idx;
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),srcRect);

    if(node->get("img",idx)) 
      image = theme->getImage(idx);

    if(node->get("offset",value)) {
      Util::parsePoint(value.c_str(),_X(offset),_Y(offset));
    }
  }

  inline void updateDstFromTopLeft(float x, float y)
  {
    _RMINX(dstRect) = x;
    _RMINY(dstRect) = y;
    _RMAXX(dstRect) = _RMINX(dstRect) + _RW(srcRect);
    _RMAXY(dstRect) = _RMINY(dstRect) + _RH(srcRect);
  }

  inline void updateDstFromTopRight(float x, float y)
  {
    _RMAXX(dstRect) = x;
    _RMINY(dstRect) = y;

    _RMINX(dstRect) = _RMAXX(dstRect) - _RW(srcRect);
    _RMAXY(dstRect) = _RMINY(dstRect) + _RH(srcRect);
  }

  inline void updateDstFromBottomLeft(float x, float y)
  {
    _RMINX(dstRect) = x;
    _RMAXY(dstRect) = y;

    _RMAXX(dstRect) = _RMINX(dstRect) + _RW(srcRect);
    _RMINY(dstRect) = _RMAXY(dstRect) - _RH(srcRect);
  }

  inline void updateDstFromBottomRight(float x, float y)
  {
    _RMAXX(dstRect) = x;
    _RMAXY(dstRect) = y;
    _RMINX(dstRect) = _RMAXX(dstRect) - _RW(srcRect);
    _RMINY(dstRect) = _RMAXY(dstRect) - _RH(srcRect);
  }
};

struct FrameElement
{
  ImgElement topLeftCorner;
  ImgElement topRightCorner;
  ImgElement bottomLeftCorner;
  ImgElement bottomRightCorner;
  ImgElement topBorder;
  ImgElement bottomBorder;
  ImgElement leftBorder;
  ImgElement rightBorder;

  GuiRect    internalRect;
  GuiRect    externalRect;

  FrameElement() { }

  inline void init(GuiTheme * theme, const XmlNode * root)
  {
    if(!root)
      return;

    XmlNode * node;
    node = root->getChild("top-left");
    topLeftCorner.init(theme,node);
    node = root->getChild("top-right");
    topRightCorner.init(theme,node);
    node = root->getChild("bottom-left");
    bottomLeftCorner.init(theme,node);
    node = root->getChild("bottom-right");
    bottomRightCorner.init(theme,node);

    node = root->getChild("top");
    topBorder.init(theme,node);
    node = root->getChild("bottom");
    bottomBorder.init(theme,node);
    node = root->getChild("right");
    rightBorder.init(theme,node);
    node = root->getChild("left");
    leftBorder.init(theme,node);
  }

  inline void updateGeometry(const GuiRect & externalRect) 
  {
    topLeftCorner.updateDstFromTopLeft(_RMINX(externalRect),_RMINY(externalRect));
    topRightCorner.updateDstFromTopRight(_RMAXX(externalRect),_RMINY(externalRect));
    bottomLeftCorner.updateDstFromBottomLeft(_RMINX(externalRect),_RMAXY(externalRect));
    bottomRightCorner.updateDstFromBottomRight(_RMAXX(externalRect),_RMAXY(externalRect));
  
    _RMINX(topBorder.dstRect) = _RMAXX(topLeftCorner.dstRect);
    _RMAXX(topBorder.dstRect) = _RMINX(topRightCorner.dstRect);
    _RMINY(topBorder.dstRect) = _RMINY(externalRect);
    _RMAXY(topBorder.dstRect) = _RMINY(externalRect) + _RH(topBorder.srcRect);

    _RMINX(bottomBorder.dstRect) = _RMAXX(bottomLeftCorner.dstRect);
    _RMAXX(bottomBorder.dstRect) = _RMINX(bottomRightCorner.dstRect);
    _RMINY(bottomBorder.dstRect) = _RMAXY(externalRect) - _RH(bottomBorder.srcRect);
    _RMAXY(bottomBorder.dstRect) = _RMAXY(externalRect); 

    _RMINX(leftBorder.dstRect) = _RMINX(externalRect);
    _RMAXX(leftBorder.dstRect) = _RMINX(externalRect) + _RW(leftBorder.srcRect);
    _RMINY(leftBorder.dstRect) = _RMAXY(topLeftCorner.dstRect);
    _RMAXY(leftBorder.dstRect) = _RMINY(bottomLeftCorner.dstRect);

    _RMINX(rightBorder.dstRect) = _RMAXX(externalRect) - _RW(leftBorder.srcRect);
    _RMAXX(rightBorder.dstRect) = _RMAXX(externalRect);
    _RMINY(rightBorder.dstRect) = _RMAXY(topRightCorner.dstRect);
    _RMAXY(rightBorder.dstRect) = _RMINY(bottomRightCorner.dstRect);

    this->externalRect = externalRect;

    _RMINX(internalRect) = _RMINX(externalRect) + _RW(leftBorder.dstRect);
    _RMAXX(internalRect) = _RMAXX(externalRect) - _RW(rightBorder.dstRect);
    _RMINY(internalRect) = _RMINY(externalRect) + _RH(topBorder.dstRect);
    _RMAXY(internalRect) = _RMAXY(externalRect) - _RH(bottomBorder.dstRect);
  }

  inline void draw()
  {
    topLeftCorner.draw();
    topRightCorner.draw();
    bottomLeftCorner.draw();
    bottomRightCorner.draw();

    topBorder.draw();
    bottomBorder.draw();
    rightBorder.draw();
    leftBorder.draw();
  }
};

class IGuiMenuItem 
{
  public:

    IGuiMenuItem(const char * className);

    virtual void init(XmlNode * node);

    virtual GuiDimension getPreferredSize()=0;
    virtual void setSize(const GuiDimension & dim) 
    { 
      m_dimension=dim;
      updateRectangle();
    }

    virtual const GuiDimension & getSize()
    {
      return m_dimension;
    }

    virtual void setPosition(const GuiPoint position) 
    { 
      m_position=position;
      updateRectangle();
    }

    virtual void setTheme(GuiTheme * theme);

    virtual void draw()=0;

    virtual bool selfDrawFocused() { return false; }

    virtual void drawFocus() { }

    virtual bool retainFocusGoingNext() { return false; }
    virtual bool retainFocusGoingPrev() { return false; }

    virtual GuiRect getRectangle() { return m_rectangle; }

    virtual bool isPointInside(const GuiPoint & point) { return _PINR(point,m_rectangle); }

    virtual void  onMouseEnter(const GuiPoint & point) { };
    virtual void  onMouseLeave(const GuiPoint & point) { };
    virtual void  onMouseClick(const GuiPoint & point) { };
    virtual void  onMouseMove(const GuiPoint & point) { };
    virtual bool  onMouseLButton(bool down, const GuiPoint & point) { return false; };
    virtual bool  onMouseRButton(bool down, const GuiPoint & point) { return false; };

    // TODO: to preserve a little of indipendence of the gui 
    //       subsystem, the api interface of the gui items
    //       should not name explicitely the irrlicht
    //       structures.
    //       nevertheless of the 'onKeyClick' the rule is
    //       not accomplished.
    virtual void  onKeyClick(const irr::SEvent::SKeyInput & event) {  };
    virtual void  onKeyUp(const irr::SEvent::SKeyInput & event) {  };
    virtual void  onKeyDown(const irr::SEvent::SKeyInput & event) {  };

    virtual bool  isSelectable() { return m_selectable; }
    virtual void  setSelectable(bool s) { m_selectable=s; }

  private:
    std::string m_className;


  protected:

    inline void executeCode(const char * code)
    {
      if(code[0]) {
        lua_State * L = ResourceManager::getInstance()->getLuaState();
        lua_pushliteral(L, "self");
        lua_pushstring(L, m_luaName);
        lua_gettable(L, LUA_GLOBALSINDEX);
        lua_settable(L, LUA_GLOBALSINDEX);

        ResourceManager::getInstance()->lua_doString(code);
      }
    }

    void updateRectangle()
    {
      m_rectangle.UpperLeftCorner.X = _X(m_position);
      m_rectangle.UpperLeftCorner.Y = _Y(m_position);
      m_rectangle.LowerRightCorner.X = _X(m_position) + _W(m_dimension);
      m_rectangle.LowerRightCorner.Y = _Y(m_position) + _H(m_dimension);
      updateGeometry();
    }

    virtual void updateGeometry()
    {
    }

    
    GuiDimension m_dimension;
    GuiPoint     m_position;
    GuiRect      m_rectangle;
    bool         m_selectable;
    GuiFont *    m_font;

    bool m_captionVCenter;
    bool m_captionHCenter;

    enum { m_luaNameSize=40 };
    char m_luaName[m_luaNameSize];
    std::string m_boundCfgName;
};

class GuiMenuItemFactory 
{
  public:
    static IGuiMenuItem * build(XmlNode * node);
};


class GuiContainerPolicy
{
  public:
    virtual void applyPolicy(GuiPoint & position,
      GuiDimension & dimension,
      std::vector<IGuiMenuItem *> & items)=0;
};

class GuiContainerPolicy_GrowVertical : public GuiContainerPolicy
{
  public:
    virtual void applyPolicy(GuiPoint & position,
      GuiDimension & dimension,
      std::vector<IGuiMenuItem *> & items);
};

class GuiContainerPolicy_GrowHorizontal : public GuiContainerPolicy
{
  public:
    virtual void applyPolicy(GuiPoint & position,
      GuiDimension & dimension,
      std::vector<IGuiMenuItem *> & items);
};

/////////////////////////////////////////////////////////////
 
class GuiMenu : public irr::gui::IGUIElement, public IEventListener
{
  public:
    GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle);

    void load(const std::string & xmlFileName);

    void setGroup(const std::wstring & name);

    // position/size
    void centerOnTheScreen();

    void mouseEvent(const irr::SEvent::SMouseInput & mouseInput);
    void keyboardEvent(const irr::SEvent::SKeyInput & keyInput);

    void draw();

    void loadTheme(const char * filename);

    void selectItemByPoint(const GuiPoint & point);

    unsigned pickupItemByPoint(const GuiPoint & point);

    inline bool isItemIndexValid(unsigned i) { return i < m_items.size(); }

    inline void setVisible(bool visible) { m_isVisible = visible; }
    inline bool getVisible() { return m_isVisible; }

    inline void setHasFrame(bool hasFrame) { m_hasFrame = hasFrame; }
    inline bool getHasFrame() { return m_hasFrame; }

  private:
    void selectNext();
    void selectPrev();

    class GuiItemGroup 
    {
      public:
        GuiItemGroup(XmlNode * xmlFileName);
        GuiItemGroup(const std::wstring & name);

        inline const std::wstring & getName() { return m_name; }

        inline void fillVector(std::vector<IGuiMenuItem*> & items)
        {
          for(unsigned i=0; i<m_items.size(); i++)
              items.push_back(m_items[i]);
        }

        inline void setTheme(GuiTheme * theme) 
        {
          for(unsigned i=0; i<m_items.size(); i++)
            m_items[i]->setTheme(theme);
        }

        inline void onShow()
        {
          if(m_onShow != "") {
            ResourceManager::getInstance()->lua_doString(m_onShow.c_str());
          }
        }


      private:
        std::wstring                m_name;
        std::vector<IGuiMenuItem *> m_items;
        std::string                 m_onShow;
    };

    void refreshSize();

    bool         m_hasFrame;
    bool         m_growSize;
    GuiDimension m_dimension;
    GuiPoint     m_position;
    GuiFont *    m_font;
    GuiFrame *   m_frame;
    GuiContainerPolicy * m_policy;

    std::vector<IGuiMenuItem*> m_items;
    IGuiMenuItem *             m_itemWhichCapturedMouse;

    GuiTheme *   m_theme;

    unsigned     m_focusedItem;

    bool         m_isVisible;

    irr::video::ITexture* m_renderTarget;

    std::vector<GuiItemGroup *> m_groups;

    enum { m_invalidItemIndex=0xffff };
};

#endif

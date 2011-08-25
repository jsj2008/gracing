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

    virtual GuiRect getRectangle() { return m_rectangle; }

    virtual bool isPointInside(const GuiPoint & point) { return _PINR(point,m_rectangle); }

    virtual void  onMouseClick(const GuiPoint & point) { };
    virtual void  onMouseMove(const GuiPoint & point) { };
    virtual bool  onMouseLButton(bool down, const GuiPoint & point) { return false; };
    virtual bool  onMouseRButton(bool down, const GuiPoint & point) { return false; };

    virtual bool  isSelectable() { return m_selectable; }
    virtual void  setSelectable(bool s) { m_selectable=s; }

  private:
    std::string m_className;


  protected:

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

    enum { m_luaNameSize=40 };
    char m_luaName[m_luaNameSize];
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

    void mouseEvent(const irr::SEvent::SMouseInput & MouseInput);

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

      private:
        std::wstring                m_name;
        std::vector<IGuiMenuItem *> m_items;
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

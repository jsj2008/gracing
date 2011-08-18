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
#include "GuiFrame.h"
#include "EventReceiver.h"

/////////////////////////////////////////////////
// irrlicht 'compatibility'
typedef irr::u32                          GuiU32;
typedef irr::s32                          GuiS32;
typedef irr::core::dimension2d<irr::u32>  GuiDimension;
typedef struct { GuiS32 x,y; }            GuiPoint;
typedef irr::core::rect<irr::s32>         GuiRect;
typedef irr::gui::IGUIFont                GuiFont;
typedef irr::video::ITexture                GuiImage;

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

    virtual void setTheme(GuiTheme * theme)=0;

    virtual void draw()=0;

    virtual bool selfDrawFocused() { return false; }

    virtual void drawFocus() { }

    virtual GuiRect getRectangle() { return m_rectangle; }

    virtual bool isPointInside(const GuiPoint & point) { return _PINR(point,m_rectangle); }

    virtual void  onMouseClick(const GuiPoint & point) { };
    virtual void  onMouseMove(const GuiPoint & point) { };
    virtual bool  onMouseLButton(bool down, const GuiPoint & point) { return false; };
    virtual bool  onMouseRButton(bool down, const GuiPoint & point) { return false; };

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
};

class GuiItemCheckBox : public IGuiMenuItem 
{
  public:
    GuiItemCheckBox(const std::wstring & caption);
    GuiItemCheckBox(const wchar_t * caption);

    virtual void setTheme(GuiTheme * theme);

    GuiDimension getPreferredSize();

    void draw();

    virtual void onMouseClick(const GuiPoint & point);

  protected:
    virtual void updateGeometry();
  
  private:
    GuiImage * m_checkerImage;
    GuiRect    m_checkerSrcRect;
    GuiRect    m_checkerDstRect;

    GuiImage * m_boxImage;
    GuiRect    m_boxSrcRect;
    GuiRect    m_boxDstRect;

    std::wstring m_caption;
    bool        m_checked;
    GuiFont *   m_font;
};

class GuiItemStaticText : public IGuiMenuItem
{
  public:
    GuiItemStaticText(const std::wstring & caption);
    GuiItemStaticText(const wchar_t * caption);

    virtual void setTheme(GuiTheme * theme);

    GuiDimension getPreferredSize();

    void draw();

  private:
    std::wstring m_caption;

    GuiFont * m_font;
};

class GuiItemListBox : public IGuiMenuItem
{
  public:
    GuiItemListBox(const std::wstring & caption);

    GuiDimension getPreferredSize();

    virtual void setTheme(GuiTheme * theme);

    void addItem(const std::wstring & item);
    void clearItems();

    void draw();

    virtual bool selfDrawFocused() { return true; }
    virtual void drawFocus();

    virtual void  onMouseMove(const GuiPoint & point);
    virtual void  onMouseClick(const GuiPoint & point);

  protected:
    virtual void updateGeometry();

  private:
    
    unsigned getItemMaxWidth();

    std::wstring               m_caption;
    GuiFont *                  m_font;

    GuiImage * m_riteImage;
    GuiRect    m_riteSrcRect;
    GuiRect    m_riteDstRect;

    GuiImage * m_leftImage;
    GuiRect    m_leftSrcRect;
    GuiRect    m_leftDstRect;

    GuiRect    m_itemDstRect;
    std::vector<std::wstring>  m_items;
    unsigned   m_selectedItem;

    enum {
      mouseOnNothing,
      mouseOnLeftImage,
      mouseOnRiteImage
    } m_mouseOver;
};

class GuiItemSlider : public IGuiMenuItem
{
  public:
    GuiItemSlider(const std::wstring & caption);

    GuiDimension getPreferredSize();

    void draw();

    void setTheme(GuiTheme * theme);

    bool selfDrawFocused() { return true; }

    void drawFocus();

    void  onMouseMove(const GuiPoint & point);
    bool  onMouseLButton(bool down, const GuiPoint & point);


  protected:
    void updateGeometry();

  private:

    void updateHandlePosition();


    std::wstring m_caption;
    GuiFont *    m_font;

    GuiImage * m_leftEdgeImage;
    GuiRect    m_leftEdgeSrcRect;
    GuiRect    m_leftEdgeDstRect;

    GuiImage * m_riteEdgeImage;
    GuiRect    m_riteEdgeSrcRect;
    GuiRect    m_riteEdgeDstRect;

    GuiImage * m_handleImage;
    GuiRect    m_handleSrcRect;
    GuiRect    m_handleDstRect;
    bool       m_handleFocused;

    GuiImage * m_fillerImage;
    unsigned   m_rangeLen;
    GuiRect    m_fillerDstRect;

    GuiPoint   m_lastMousePoint;
    bool       m_draggingHandle;
    unsigned   m_handleValue;
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
 
// this is the container class !!
class GuiMenu : public irr::gui::IGUIElement, public IEventListener
{
  public:
    GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle);

    // elements building
    GuiItemStaticText * addStaticText(const std::wstring & caption);
    GuiItemCheckBox *   addCheckBox(const std::wstring & caption);
    GuiItemListBox *    addListBox(const std::wstring & caption);
    GuiItemSlider *     addSlider(const std::wstring & caption);


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

    enum { m_invalidItemIndex=0xffff };
};

#endif
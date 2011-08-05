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

#define _X(pnt)   pnt.x
#define _Y(pnt)   pnt.y

#define _W(dim)   dim.Width
#define _H(dim)   dim.Height

/////////////////////////////////////////////////

class IGuiMenuItem 
{
  public:

    virtual GuiDimension getPreferredSize()=0;
    virtual void setSize(const GuiDimension & dim) 
    { 
      m_dimension=dim;
      updateRectangle();
    }

    virtual void setPosition(const GuiPoint position) { 
      m_position=position;
      updateRectangle();
    }

    virtual void draw()=0;

  private:

    void updateRectangle()
    {
      GM_LOG("updating rectangle: pos: %d,%d, dim: %d,%d\n",
          _X(m_position),_Y(m_position),
          _W(m_dimension),_H(m_dimension));

      m_rectangle.UpperLeftCorner.X = _X(m_position);
      m_rectangle.UpperLeftCorner.Y = _Y(m_position);
      m_rectangle.LowerRightCorner.X = _X(m_position) + _W(m_dimension);
      m_rectangle.LowerRightCorner.Y = _Y(m_position) + _H(m_dimension);
    }

  protected:
    
    GuiDimension m_dimension;
    GuiPoint     m_position;

    GuiRect      m_rectangle;
};

class GuiItemStaticText : public IGuiMenuItem
{
  public:
    GuiItemStaticText(const std::wstring & caption);
    GuiItemStaticText(const wchar_t * caption);

    GuiDimension getPreferredSize();

    void draw();

  private:
    std::wstring m_caption;

    GuiFont * m_font;
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


    // position/size
    void centerOnTheScreen();

    void mouseEvent(const irr::SEvent::SMouseInput & MouseInput);

    void draw();

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

};

#endif

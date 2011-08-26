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
#ifndef GUIITEMSLIDER_H
#define GUIITEMSLIDER_H

#include "GuiMenu.h"

#define SLIDER_CLASSNAME "slider"

class GuiItemSlider : public IGuiMenuItem
{
  public:
    GuiItemSlider(lua_State *) : IGuiMenuItem(SLIDER_CLASSNAME) { assert(0); }
    GuiItemSlider(const std::wstring & caption);

    GuiDimension getPreferredSize();

    void draw();

    void init(XmlNode *);

    void setTheme(GuiTheme * theme);

    bool selfDrawFocused() { return true; }

    void drawFocus();

    void  onMouseMove(const GuiPoint & point);
    bool  onMouseLButton(bool down, const GuiPoint & point);
    void  onMouseClick(const GuiPoint & point);

    void  onKeyDown(const irr::SEvent::SKeyInput & keyinput);
    void  onKeyUp(const irr::SEvent::SKeyInput & keyinput);

    inline void  getRange(double & min, double & max) { min = m_minValue; max = m_maxValue; }
    void         setRange(double min, double max);

    double       getValue();
    void         setValue(double value);

    // lua bindings
    int          lgetValue(lua_State * L);
    int          lsetValue(lua_State * L);
    int          lsetRange(lua_State * L);


    static const char * className;
    static Lunar<GuiItemSlider>::RegType  methods[];



  protected:
    void updateGeometry();

  private:

    void updateHandlePosition();


    std::wstring m_caption;

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
    GuiRect    m_fillerDstRect;

    GuiPoint   m_lastMousePoint;
    bool       m_draggingHandle;
    int        m_movingHandleWithKeyDirection;
    int        m_rangeLen;
    bool       m_rangeLenFixed;
    int        m_handleValue;

    double     m_minValue, m_maxValue;


    std::string m_onChange;
};


#endif

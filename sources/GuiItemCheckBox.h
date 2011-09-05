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
#ifndef GUIITEM_CHECKBOX_H
#define GUIITEM_CHECKBOX_H

#define CHECKBOX_CLASSNAME "checkbox"

class GuiItemCheckBox : public IGuiMenuItem 
{
  public:
    GuiItemCheckBox(lua_State *) : IGuiMenuItem(CHECKBOX_CLASSNAME) { assert(0); }
    GuiItemCheckBox(const std::wstring & caption);
    GuiItemCheckBox(const wchar_t * caption);

    virtual void setTheme(GuiTheme * theme);

    void init(XmlNode * node);

    GuiDimension getPreferredSize();

    void draw();

    virtual void onMouseClick(const GuiPoint & point);
    void onKeyClick(const irr::SEvent::SKeyInput & event);

    static const char * className;
    static Lunar<GuiItemCheckBox>::RegType  methods[];

    int getValue(lua_State *);
    int setValue(lua_State * L);

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

    std::string m_onChange;

};

#endif


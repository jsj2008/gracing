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
#ifndef GUIITEM_STATICTEXT_H
#define GUIITEM_STATICTEXT_H

#define STATICTEXT_CLASSNAME "static"

class GuiItemStaticText : public IGuiMenuItem
{
  public:
    GuiItemStaticText(lua_State *) : IGuiMenuItem(STATICTEXT_CLASSNAME) { assert(0); }
    GuiItemStaticText(const std::wstring & caption);
    GuiItemStaticText(const wchar_t * caption);

    void init(XmlNode *);

    GuiDimension getPreferredSize();

    void draw();

    static const char * className;
    static Lunar<GuiItemStaticText>::RegType  methods[];

    virtual void onMouseClick(const GuiPoint & point);
    void onKeyClick(const irr::SEvent::SKeyInput & event);

    int lsetcaption(lua_State * L);
  private:
    std::wstring m_caption;
    bool         m_center;


    std::string m_onClick;
};

#endif

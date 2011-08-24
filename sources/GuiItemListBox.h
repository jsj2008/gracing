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
#ifndef GUIITEM_LISTBOX
#define GUIITEM_LISTBOX
#include "GuiMenu.h"

#define LISTBOX_CLASSNAME "listbox"

class GuiItemListBox : public IGuiMenuItem
{
  public:
    GuiItemListBox(const std::wstring & caption);

    GuiDimension getPreferredSize();

    virtual void setTheme(GuiTheme * theme);

    void init(XmlNode *);

    void addItem(const std::wstring & item);
    void addItem(const std::string & item);

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
#endif

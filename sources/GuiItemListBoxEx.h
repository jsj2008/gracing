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
#ifndef GUIITEM_LISTBOXEX
#define GUIITEM_LISTBOXEX
#include "GuiMenu.h"
#include "Util.hh"

#define LISTBOXEX_CLASSNAME "listbox-extended"

class GuiItemListBoxEx : public IGuiMenuItem
{
  public:
    GuiItemListBoxEx(lua_State * L) : IGuiMenuItem(LISTBOXEX_CLASSNAME) { assert(0); }
    GuiItemListBoxEx(const std::wstring & caption);

    GuiDimension getPreferredSize();

    virtual void setTheme(GuiTheme * theme);

    void init(XmlNode *);

    void addItem(const std::wstring & item,bool selected=false);
    void addItem(const std::string & item,bool selected=false);
    void clearItems();

    int getValue();
    void setValue(unsigned value);

    void draw();

    virtual bool selfDrawFocused() { return true; }
    virtual void drawFocus();

    virtual void  onMouseMove(const GuiPoint & point);
    virtual void  onMouseClick(const GuiPoint & point);
    virtual void  onMouseLeave(const GuiPoint & point);
    virtual void  onMouseEnter(const GuiPoint & point);
    virtual void  onKeyClick(const irr::SEvent::SKeyInput & keyinput);
    virtual bool retainFocusGoingNext();
    virtual bool retainFocusGoingPrev();

    static const char * className;
    static Lunar<GuiItemListBoxEx>::RegType  methods[];

    int lgetvalue(lua_State *);
    int lgetitemlabel(lua_State * L);
    int lsetvalue(lua_State *);
    int ladditem(lua_State *);
    int lappenditem(lua_State *);
    int lclearitems(lua_State *);

    void selectItem(unsigned);

  protected:
    virtual void updateGeometry();

  private:
    
    unsigned getItemMaxWidth();

    void hilightNextItem();
    void hilightPrevItem();

    std::wstring               m_caption;

    struct ImgElement
    {
      GuiImage * image;
      GuiRect    srcRect;
      GuiRect    dstRect;

      ImgElement() { image=0; }

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
      }
    };


    GuiRect    m_listDstRect;
    std::vector<std::wstring>  m_items;
    std::vector<GuiRect>       m_itemsRect;
    unsigned   m_selectedItem;
    unsigned   m_hilightItem;
    unsigned   m_firstItemShown;
    unsigned   m_visibleItems;
    bool       m_retainFocus;

    std::string m_onChange;

};
#endif

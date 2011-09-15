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
#include "Util.hh"

#define LISTBOX_CLASSNAME "listbox"

class GuiItemListBox : public IGuiMenuItem
{
  public:
    GuiItemListBox(lua_State * L) : IGuiMenuItem(LISTBOX_CLASSNAME) { assert(0); }
    GuiItemListBox(const std::wstring & caption);

    GuiDimension getPreferredSize();

    virtual void setTheme(GuiTheme * theme);

    void init(XmlNode *);

    void addItem(const std::wstring & item,bool selected=false);
    void addItem(const std::string & item,bool selected=false);

    int getValue();
    void setValue(unsigned value);

    void clearItems();

    void draw();

    virtual bool selfDrawFocused() { return true; }
    virtual void drawFocus();

    virtual void  onMouseMove(const GuiPoint & point);
    virtual void  onMouseClick(const GuiPoint & point);
    virtual void  onKeyClick(const irr::SEvent::SKeyInput & keyinput);

    static const char * className;
    static Lunar<GuiItemListBox>::RegType  methods[];

    int lgetvalue(lua_State *);
    int lsetvalue(lua_State *);
    int lclearItems(lua_State *);
    int laddItem(lua_State *);

  protected:
    virtual void updateGeometry();

  private:
    
    unsigned getItemMaxWidth();
    void     selectNextItem();
    void     selectPrevItem();

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

    ImgElement m_rightButton;
    ImgElement m_leftButton;
    ImgElement m_rightButtonHi;
    ImgElement m_leftButtonHi;
    bool       m_highlightButtons;
    bool       m_mustHighlight;
    bool       m_itemWidthFixed;
    unsigned   m_itemWidth;



    GuiRect    m_itemDstRect;
    std::vector<std::wstring>  m_items;
    unsigned   m_selectedItem;

    std::string m_onChange;


    enum {
      mouseOnNothing,
      mouseOnLeftImage,
      mouseOnRiteImage
    } m_mouseOver;
};
#endif

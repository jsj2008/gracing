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
#ifndef COMPOUND_SCENE_NODE_H
#define COMPOUND_SCENE_NODE_H
#include <irrlicht.h>

#include "gmlog.h"

// an empty scene nodes that only contains other
// SceneNodes (i was not able to find one in irrlicht scene nodes)
class CompoundSceneNode : public irr::scene::ISceneNode
{
  public:
    CompoundSceneNode(irr::scene::ISceneNode * parent, irr::scene::ISceneManager * smgr, irr::s32 id=-1) 
      : irr::scene::ISceneNode(parent,smgr,id)
    {
      // !! nothing here !!
      m_mustUpdateBoundingBox=false;
    }

    virtual void render()
    {
     
    }

		virtual void addChild(ISceneNode* child)
    {
      if(child && child!=this) {
        m_mustUpdateBoundingBox=true;
      }
      irr::scene::ISceneNode::addChild(child);

    }

    virtual const irr::core::aabbox3d<float> & getBoundingBox() const
    {
      return m_boundingBox;
    }

    void recalculateBoundingBox() 
    {
      m_boundingBox.reset(0.,0.,0.);
      irr::scene::ISceneNodeList::ConstIterator it = Children.begin();
      for (; it != Children.end(); ++it) {
        m_boundingBox.addInternalBox((*it)->getBoundingBox());
      }
    }
    

    void dummy() { int a; a=3.; }
  private:
    irr::core::aabbox3d<float> m_boundingBox;
    bool                       m_mustUpdateBoundingBox;
};

#endif

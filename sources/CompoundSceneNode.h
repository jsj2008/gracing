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
    }

    virtual void render()
    {
      static int cnt=0;
      GM_LOG("suka %d\n",cnt++);
#if 0
      // not necessary coz the scene manager is in charge to call
      // render on children??
      irr::scene::ISceneNodeList::Iterator it = Children.begin();
      for (; it != Children.end(); ++it)
        (*it)->render();
#endif
    }

		virtual void addChild(ISceneNode* child)
    {
      static int cnt=0;
      GM_LOG("addchild suka %d\n",cnt++);
      if(child && child!=this) {
        m_boundingBox.addInternalBox(child->getBoundingBox());
#if 0
				// Change scene manager?
				if (SceneManager != child->SceneManager)
					child->setSceneManager(SceneManager);

				child->grab();
				child->remove(); // remove from old parent
				Children.push_back(child);
				child->Parent = this;
#endif
      }
      //irr::scene::ISceneNode::addChild(child);
      GM_LOG("risuka\n");
    }

    virtual const irr::core::aabbox3d<float> & getBoundingBox() const
    {
      return m_boundingBox;
    }

    void dummy() { int a; a=3.; }
  private:
    irr::core::aabbox3d<float> m_boundingBox;
};



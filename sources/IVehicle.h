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

#include "CompoundSceneNode.h"
// !!! interface for a vehicle !!!
class IVehicle : public CompoundSceneNode
{
  public:
  enum {
    USE_PHYSICS=0x1,
    USE_GRAPHICS=0x2,
  };

  IVehicle(irr::scene::ISceneNode * parent, irr::scene::ISceneManager * smgr, irr::s32 id=-1) 
      : CompoundSceneNode(parent,smgr,id)
  {
    // empty here
  }

  virtual void load()=0;

  virtual void unload()=0;

  virtual void use(unsigned int useFlags)=0;

  virtual void unuse(unsigned int useFlags)=0;

  // adding more later...
};
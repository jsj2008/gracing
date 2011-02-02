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

#include "IrrMotionState.h"

IrrMotionState::IrrMotionState(irr::scene::ISceneNode * node, const btTransform & initialTrans) 
{
  m_node=node;
  m_node->grab();
  m_initialTransform=initialTrans;

}

IrrMotionState::~IrrMotionState()
{
  m_node->drop();
}

void IrrMotionState::getWorldTransform (btTransform &worldTrans)
{
  worldTrans=m_initialTransform;
}

void IrrMotionState::setWorldTransform (const btTransform &trans)
{
  // position
  m_node->setPosition(
          irr::core::vector3df(
      float(trans.getOrigin().getX()),
      float(trans.getOrigin().getY()),
      float(trans.getOrigin().getZ())));

  // rotation
  irr::core::vector3df rot;
  btQuaternion quaternion;
  quaternion=trans.getRotation();
  //quaternion.toEuler(rot); where is 
  //m_node->setRotation(quaternion);
  //float(trans.getOrigin().getX()),
  //float(trans.getOrigin().getY()),
  //float(trans.getOrigin().getZ())));
}



//  gracing - a idiot (but physically powered) racing game 
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

#include "IrrDebugDrawer.h"

IrrDebugDrawer::IrrDebugDrawer(irr::video::IVideoDriver * driver)
{
  m_driver=driver;
}

void IrrDebugDrawer::drawLine(const btVector3& from, const btVector3& to,
                              const btVector3& color)
{
  irr::video::SColor c(255, (int)(color.getX()*255), (int)(color.getY()*255),
      (int)(color.getZ()*255));
  m_driver->draw3DLine((const irr::core::vector3df&)from,
      (const irr::core::vector3df&)to, c);
}

void IrrDebugDrawer::drawAabb(const btVector3 & from,
        const btVector3 & to,
        const btVector3 & color)
{
  irr::video::SColor c(255, (int)(color.getX()*255), (int)(color.getY()*255),
      (int)(color.getZ()*255));

  irr::core::aabbox3df  box(irr::core::vector3df(from.getX(), from.getY(), from.getZ()),
        irr::core::vector3df(to.getX(), to.getY(), to.getZ()));
  m_driver->draw3DBox(box,c);

}

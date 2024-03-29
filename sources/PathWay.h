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

#ifndef PATHWAY_H
#define PATHWAY_H

#include <vector>
#include "btBulletDynamicsCommon.h"

class PathWay
{
  public:
    PathWay();

    PathWay(
        const std::vector<btVector3> & controlPoints,
        btVector3                      point,
        const float                    radius);

    void initialize(
        const std::vector<btVector3> & controlPoints,
        btVector3                      point,
        const float                    radius);

    void updateOnPath(btVector3 point);

    void getTarget(btVector3 & target);

    void closestPoint(
        const btVector3 & point,
        btVector3 & tangent,
        btVector3 & pointToFollow,
        double    & distanceToPath,
        unsigned & index);

  private:

    std::vector<float> m_lengths;
    std::vector<btVector3> m_points;
    std::vector<btVector3> m_normals;
    float              m_radius;
    float              m_totalLength;


    unsigned           m_currentIndex;
    float              m_currentPathDistance;
    btVector3          m_targetPoint;
};

#endif

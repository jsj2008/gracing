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

#include "PathWay.h"


PathWay::PathWay(
    const std::vector<btVector3> & controlPoints,
    const float                    radius)
{
  initialize(controlPoints,radius);
}

void PathWay::initialize(
    const std::vector<btVector3> & controlPoints,
    const float                    radius)
{
  m_radius=radius;

  unsigned i_minus_one;

  for(unsigned i=0; i<controlPoints.size(); i++) {
    m_points.push_back(controlPoints[i]);
    if(i==0)
      i_minus_one=controlPoints.size()-1;
    else
      i_minus_one=i-1;
    btVector3 normal=controlPoints[i] - controlPoints[i_minus_one];
    double    length=normal.length();
    m_lengths.push_back(length);
    normal.normalize();
    m_normals.push_back(normal);
    m_totalLength += length;
  }
}

void PathWay::closestPoint(
    const btVector3 & point,
    btVector3 & tangent)
{
  float minDistance=1e10;

  for(unsigned i=0; i < m_points.size(); i++)  {
    double len = m_lengths[i];
    btVector3 norm=m_normals[i];
    
  }

}

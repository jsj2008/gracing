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
#include "gmlog.h"

double pointSegmentDistance(
    const btVector3 & point,
    const btVector3 & ep0, 
    const btVector3 & ep1,
    const btVector3 & segmentNormal,
    const double    & segmentLength,
          btVector3 & onPath)
{
  btVector3 local = point - ep0;

  // find the projection of "local" onto "segmentNormal"
  double segmentProjection = segmentNormal.dot (local);

  // handle boundary cases: when projection is not on segment, the
  // nearest point is one of the endpoints of the segment
  if (segmentProjection < 0)
  {
    onPath = ep0;
    segmentProjection = 0;
    return (point - ep0).length();
  }
  if (segmentProjection > segmentLength)
  {
    onPath = ep1;
    segmentProjection = segmentLength;
    return (point - ep1).length();
  }

  // otherwise nearest point is projection point on segment
  onPath = segmentNormal * segmentProjection;
  onPath +=  ep0;
  return (point - onPath).length();

}

PathWay::PathWay()
{
}

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
  btVector3 onPath, minOnPath;
  unsigned i_minus_one;
  unsigned gi;

  for(unsigned i=0; i < m_points.size(); i++)  {
    if(i==0)
      i_minus_one=m_points.size()-1;
    else
      i_minus_one=i-1;
    double d = pointSegmentDistance(point,
        m_points[i_minus_one], m_points[i],
        m_normals[i],m_lengths[i],
        onPath);
    GM_LOG("index: %d ",i);
    if(d < minDistance) {
      GM_LOG(" good ****\n");
      gi=i;
      minDistance = d;
      minOnPath = onPath;
      tangent=m_normals[i];
    } else {
      GM_LOG(" no good ****\n");
    }
  }
}

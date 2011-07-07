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

#ifndef TWEENING_H
#define TWEENING_H

template<class T>
struct ITween 
{
   virtual void init(double t, const T & startPos, const T & endPos)=0;
   virtual bool doit(double t, T & position);
};


template<class T>
struct TweenVoid : public ITween<T>
{
  virtual void init(double t, T & startPos, const T & endPos)
  {
  
  }

  virtual bool doit(double t, T & position)
  {
    return false;
  }
};

template<class T>
struct TweenQuadIn : public ITween<T>
{
  virtual void init(double t, const T & start, const T & end)
  {
    m_startPos = start;
    m_totalChange = end - start;
    m_startTime = t;
  }

  virtual bool doit(double _t, T & position, double & rotation)
  {
    double t = _t - m_startTime;

    if (t < 1.) {
      position = m_totalChange * t * t + m_startPos;
      return false;
    } else {
      position = m_totalChange + m_startPos;
      return true;
    }
  }

  private:
    double m_startTime;
    T      m_startPos;
    T      m_totalChange;
};

template<class T>
struct TweenBounceOut : public ITween<T>
{
  virtual void init(double t, const T & start, const T & end)
  {
    m_startPos = start;
    m_totalChange = end - start;
    m_startTime = t;
  }

  virtual bool doit(double _t, T & position, double & rotation)
  {
    double t=_t - m_startTime;
    bool   ret;
    rotation = t;
    ret = false;

    if( t < (1/2.75)) {
      position =  m_totalChange * (7.5625*t*t) + m_startPos;
    } else if(t < (2/2.75)) {
      t -= 1.5 / 2.75;
      position =  m_totalChange * (7.5625 * t * t + .75)  + m_startPos;
    } else if(t <(2.5 / 2.75)) {
      t -= 2.25 / 2.75;
      position =  m_totalChange * (7.5625 * t * t + .9375) + m_startPos;
    } else if(t < 1.) {
      t -= 2.625 / 2.75;
      position =  m_totalChange * (7.5625 * t * t + .984375) + m_startPos;
    } else {
      position = m_totalChange + m_startPos;
      ret = true;
    }

    return ret;
  }

  private:
    double m_startTime;
    T      m_startPos;
    T      m_totalChange;
};


template<class T>
struct LinearTween : public ITween<T>
{
  virtual void init(double t, const T & sPos, const T & ePos)
  {
    startTime=t;
    start=sPos;
    end=ePos;
    rotation=0.;
  }

  virtual bool doit(double t, T & position, double  & rot)
  {
    double tf=t-startTime;
    position=start + (end - start) * tf;
    rot=t;

    if(tf >= 1.)
      return true;
    return false;
  }

  private:
    double startTime;
    double rotation;
    T start,end;
};
#endif

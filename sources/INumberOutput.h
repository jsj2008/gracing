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

#ifndef INUMBEROUTPUT_H
#define INUMBEROUTPUT_H

class INumberOutput 
{
  public:
    INumberOutput(bool loBounded=false, bool hiBounded=false,
        double hiBound=0., double loBound=0.)
    {
      m_hiBounded=hiBounded;
      m_loBounded=loBounded;
      m_hiBound=hiBound;
      m_loBound=loBound;
      m_valuesIndex=0;
    };

    virtual double setValue(double v)
    {
      if(m_hiBounded && v > m_hiBound)
        v=m_hiBound;

      if(m_loBounded && v< m_loBound)
        v=m_loBound;

      m_values[m_valuesIndex]=v;
      m_valuesIndex++;
      m_valuesIndex%=4;

      m_value=
        (m_values[0]+m_values[1]+
        m_values[2]+m_values[3])/4.;

      return m_value;
    }

    virtual double getValue() 
    {
      return m_value;
    }

  private:
    bool   m_hiBounded, m_loBounded;
    double m_hiBound, m_loBound;
    double m_value;
    double m_values[4];
    int    m_valuesIndex;

};

#endif

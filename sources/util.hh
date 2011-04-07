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
#ifndef UTIL_HH
#define UTIL_HH

#include "gmlog.h"

inline double rad2deg(double value)
{
  return 57.2957795 * value;
}

class Util 
{
  public:

  enum {
    MARK_VERTICES=0xf100,
    MARK_FACES_ONLY=0xf101,
    MARK_MATERIAL=0xf102,
    MARK_USE_MATERIAL=0xf103,
    MARK_UV_COORD=0xf104,
    MARK_FACES_AND_UV=0xf105
  };

  static inline irr::u32 readU32(irr::io::IReadFile * file)
  {
    irr::u32 val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline int readInt(irr::io::IReadFile * file) 
  {
    int val;
    file->read(&val,sizeof(val));
    return val;
  }

  // NB: the string must be deleted!!!
  static inline char * readString(irr::io::IReadFile * file)
  {
    unsigned len=readShort(file);
    char * str=new char[len+1];
    file->read(str,len);
    str[len]=0;
    return str;
  }

  static inline unsigned readShort(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline void readTriple(irr::io::IReadFile * file, double * tr) 
  {
    tr[0]=readDouble(file);
    tr[1]=readDouble(file);
    tr[2]=readDouble(file);
  }

  static inline unsigned readMark(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline double readDouble(irr::io::IReadFile * file)
  {
    double val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline void readVertex(irr::io::IReadFile * file, irr::core::vector3df & vec)
  {
    vec.X=readDouble(file);
    vec.Y=readDouble(file);
    vec.Z=readDouble(file);
  }

  static inline void readVertex2d(irr::io::IReadFile * file, irr::core::vector2df & vec)
  {
    vec.X=readDouble(file);
    vec.Y=readDouble(file);
  }

  static inline void parseVector(const char * str, irr::core::vector3df & vec)
  {
	  const irr::u32 WORD_BUFFER_LENGTH = 256;
	  char wordBuffer[WORD_BUFFER_LENGTH];
    double c[3];
    char * ptr;
    int n;
    c[0]=c[1]=c[2]=0.f;

    for(n=0, ptr=wordBuffer; *str && n<3;  str++) {
      if( *str && *str != ',') {
        *ptr=*str;
        ptr++;
      } else {
        *ptr=0;
        c[n]=irr::core::fast_atof(wordBuffer);
        n++;
        ptr=wordBuffer;
      }
    }
    if(n<3) {
      *ptr=0;
      c[n]=irr::core::fast_atof(wordBuffer);
    }
    vec.X=c[0];
    vec.Y=c[1];
    vec.Z=c[2];
  }

	static inline double parseFloat(const char * str)
	{
		return irr::core::fast_atof(str);
	}

};

#endif

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

#include <irrlicht.h>
#include "gmlog.h"

#define method(class, name) {#name, &class::name}
#define methodWithName(class, _method, name) {name, &class::_method}

//1 degree = 0.017453293 radiant 
inline double rad2deg(double value)
{
  return 57.2957795 * value;
}

inline double deg2rad(double value)
{
  return 0.017453293 * value;
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


    static void drawRectWithBackgroung(irr::video::IVideoDriver *driver, 
        irr::video::ITexture* backgroud , 
        irr::core::rect<irr::s32> destRect, 
        bool useAlphaChannel,
        unsigned modeHoriz,
        unsigned modeVert);

    static void draw2DImage_v2(irr::video::IVideoDriver *driver, 
        irr::video::ITexture* texture , 
        irr::core::rect<irr::s32> sourceRect, 
        irr::core::position2d<irr::s32> position, 
        irr::core::position2d<irr::s32> rotationPoint, 
        irr::f32 rotation, 
        irr::core::vector2df scale, 
        bool useAlphaChannel, 
        irr::video::SColor color);

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

    static inline void parseVector(const char * str, irr::core::vector2d<irr::s32>& vect) 
    {
      const irr::u32 WORD_BUFFER_LENGTH = 256;
      char wordBuffer[WORD_BUFFER_LENGTH];
      int  c[2];
      char * ptr;
      int n;
      c[0]=c[1]=0;

      for(n=0, ptr=wordBuffer; *str && n<2;  str++) {
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
      if(n<2) {
        *ptr=0;
        c[n]=atoi(wordBuffer);
      }

      vect.X=c[0];
      vect.Y=c[1];
    }

    static inline void parseRect(const char * str, irr::core::rect<irr::s32>& rect) 
    {
      const irr::u32 WORD_BUFFER_LENGTH = 256;
      char wordBuffer[WORD_BUFFER_LENGTH];
      int  c[4];
      char * ptr;
      int n;
      c[0]=c[1]=c[2]=c[3]=0;


      for(n=0, ptr=wordBuffer; *str && n<4;  str++) {
        if( *str && *str != ',') {
          *ptr=*str;
          ptr++;
        } else {
          *ptr=0;
          c[n]=atoi(wordBuffer);
          n++;
          ptr=wordBuffer;
        }
      }
      if(n<4) {
        *ptr=0;
        c[n]=atoi(wordBuffer);
      }

      rect.UpperLeftCorner.X=c[0];
      rect.UpperLeftCorner.Y=c[1];
      rect.LowerRightCorner.X=c[2];
      rect.LowerRightCorner.Y=c[3];
    }

    static inline void parseQuaternion(const char * str, double c[4])
    {
      const irr::u32 WORD_BUFFER_LENGTH = 256;
      char wordBuffer[WORD_BUFFER_LENGTH];
      //double c[3];
      char * ptr;
      int n;
      c[0]=c[1]=c[2]=c[3]=0.f;

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
      if(n<4) {
        *ptr=0;
        c[n]=irr::core::fast_atof(wordBuffer);
      }
    }

    static inline void parseVector(const char * str, double c[3])
    {
      const irr::u32 WORD_BUFFER_LENGTH = 256;
      char wordBuffer[WORD_BUFFER_LENGTH];
      //double c[3];
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
#if 0
      vec.X=c[0];
      vec.Y=c[1];
      vec.Z=c[2];
#endif
    }

    static inline void parseVector(const char * str, irr::core::vector3df & vec)
    {
      double c[3];
      parseVector(str,c);
      vec.X=c[0];
      vec.Y=c[1];
      vec.Z=c[2];
    }

    static inline double parseFloat(const char * str)
    {
      return irr::core::fast_atof(str);
    }

    static inline unsigned parseUnsigned(const char * str)
    {
      return atoi(str);
    }


};


class XmlNode;
XmlNode * loadXml(const char * filename, const char * manifestName);

#endif

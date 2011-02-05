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

#include <stdlib.h>

#ifndef VEHICLES_DIR
#error Please define VEHICLES_DIR preprocessor macro
#endif

class ResourceManager 
{
  public:
    static inline void getVehiclesCompletePath(char * buffer, int buffer_len)
    {
      const char * ddir=VEHICLES_DIR;
      int i;
      --buffer_len;
      for(i=0; i<buffer_len && *ddir; ++i, ++ddir)
        buffer[i]=*ddir;
    }
    static inline void getVehicleCompletePath(const char * vehicleName, char * buffer, int buffer_len)
    {
#ifdef __APPLE__
      //CFUrlRef url=CFBundleCopyResourceURL(bundle, CFSTR(vehicleName), CFSTR("zip"), CFSTR("Resources"));
      const char * ddir=VEHICLES_DIR;
      int i;
      --buffer_len;
      for(i=0; i<buffer_len && *ddir; ++i, ++ddir)
        buffer[i]=*ddir;
      for( ; i<buffer_len && *vehicleName; ++i, ++vehicleName)
        buffer[i]=*vehicleName;
      buffer[i]=0;
#else
      int i;
      for(i=0; i<buffer_len && *vehicleName; ++i)
        buffer[i]=vehicleName[i];
#endif
    }

};


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
#ifndef CONFIG_H
#define CONFIG_H

#define CFG_PARAM_D(p)  double p
#define CFG_INIT_D(p,v) p=v;

#define CFG_PARAM_V3(p)         double p[3]
#define CFG_INIT_V3(p,v1,v2,v3) p[0]=v1; p[1]=v2; p[2]=v3

#define CFG_PARAM_BOOL(p)         bool p
#define CFG_INIT_BOOL(p,v)          p=v

#define CFG_PARAM_UINT(p)           unsigned p
#define CFG_INIT_INT(p,v)          p=v

class ResourceManager;

class ConfigInit 
{
  public:
  static void initGlobVariables(ResourceManager *);
};

#endif

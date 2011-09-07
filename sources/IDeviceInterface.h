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
#ifndef IDEVICEINTERFACE_H
#define IDEVICEINTERFACE_H

#include "XmlNode.h"
#include "IVehicleController.h"

class IDeviceInterface
{
  public:
    virtual std::string          getName()=0;
    virtual unsigned             getNumController()=0;
    virtual IVehicleController * getController(unsigned)=0;

    virtual void setConfiguration(XmlNode * node)=0;
    virtual void getConfiguration(XmlNode * node)=0;

    virtual ~IDeviceInterface()  { }
};
#endif

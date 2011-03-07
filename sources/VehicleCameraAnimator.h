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
#ifndef VEHICLECAMERAANIMATOR_H
#define VEHICLECAMERAANIMATOR_H
#include <irrlicht.h>
#include "IVehicle.h"

class VehicleCameraAnimator : public irr::scene::ISceneNodeAnimator
{
  public:
    VehicleCameraAnimator(IVehicle * vehicle);

    // Animates a scene node. 
    virtual void 	animateNode (irr::scene::ISceneNode *node, irr::u32 timeMs);

    // Creates a clone of this animator. 
    //virtual irr::scene::ISceneNodeAnimator * createClone(irr::sceneISceneNode *node, 
    //    irr::scene::ISceneManager *newManager=0);

    // Returns type of the scene node animator. 
    virtual irr::scene::ESCENE_NODE_ANIMATOR_TYPE getType () const
    {
      return irr::scene::ESNAT_UNKNOWN;
    }

    // Returns if the animator has finished. 
    virtual bool hasFinished (void) const { return false; }

    // Returns true if this animator receives events. 
    virtual bool 	isEventReceiverEnabled () const { return false; };

    // Event receiver, override this function for camera controlling animators. 
    //virtual bool 	OnEvent (const SEvent &event);
  private:
    IVehicle * m_vehicle;

};

#endif

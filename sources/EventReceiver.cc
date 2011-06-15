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
#include "EventReceiver.h"
#include "gmlog.h"

using namespace irr;

bool EventReceiver::OnEvent(const SEvent& event)
{
  // Remember whether each key is down or up
  if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
    if(!KeyIsDown[event.KeyInput.Key] && event.KeyInput.PressedDown) 
      OneShotKeyIsDown[event.KeyInput.Key] = true;
    KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
  }
  return false;
}

    // This is used to check whether a key is being held down
bool EventReceiver::IsKeyDown(EKEY_CODE keyCode) const
{
  return KeyIsDown[keyCode];
}

bool EventReceiver::OneShotKey(EKEY_CODE keyCode) 
{
  bool b;
  b=OneShotKeyIsDown[keyCode];
  OneShotKeyIsDown[keyCode]=false;
  return b;
}

EventReceiver::EventReceiver()
{
  for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i) {
    KeyIsDown[i] = false;
    OneShotKeyIsDown[i]=false;
  }
}


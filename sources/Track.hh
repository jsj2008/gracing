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
#ifndef TRACK_HH
#define TRACK_HH
#include <irrlicht.h>
#include "PhyWorld.h"
#include "CameraDataManager.hh"

class Track
{
  public:
    Track(irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * filename);
    ~Track();

    void load();
    void unload();


    inline const irr::core::vector3df & getStartPosition() { return m_startPosition; }

  private:

    void loadLights( irr::io::IReadFile * file ,
        irr::scene::ISceneManager* smgr );

    irr::io::IFileSystem *                         m_filesystem;
    irr::u32                                       m_archiveIndex;

    // resources loaded
    irr::core::array<irr::scene::ILightSceneNode*>   
                                                   m_lights;
    irr::core::array<irr::scene::IAnimatedMeshSceneNode*>   
                                                   m_sceneNodes;
    irr::scene::ICameraSceneNode *                 m_camera;

    CameraDataManager *                            m_cammgr;

    bool                                           m_loaded;
    std::string                                    m_filename;
    irr::IrrlichtDevice *                          m_device;
    PhyWorld *                                     m_world;

    irr::core::vector3df                           m_startPosition;
};

#endif

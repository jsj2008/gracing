#ifndef TRACK_HH
#define TRACK_HH
#include <irrlicht.h>
#include "PhyWorld.h"



class Track
{
  public:
    Track(irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * filename);
    ~Track();

    void load();
    void unload();

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

    bool                                           m_loaded;
    const char *                                   m_filename;
    irr::IrrlichtDevice *                          m_device;
    PhyWorld *                                     m_world;
};

#endif

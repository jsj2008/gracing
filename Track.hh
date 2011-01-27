#ifndef TRACK_HH
#define TRACK_HH
#include <irrlicht.h>

class Track
{
  public:
    Track( irr::IrrlichtDevice * device, const char * filename);

  private:

    void loadLights( irr::io::IReadFile * file ,
        irr::scene::ISceneManager* smgr );

    irr::io::IFileSystem * m_filesystem;
    irr::u32               m_archiveIndex;


  irr::core::array<irr::scene::ILightSceneNode*> Lights;

};

#endif

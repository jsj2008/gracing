#ifndef TRACK_HH
#define TRACK_HH
#include <irrlicht.h>

class Track
{
  public:
    Track( irr::IrrlichtDevice * device, const char * filename);

  private:
    irr::io::IFileSystem * m_filesystem;
    irr::u32               m_archiveIndex;

};

#endif

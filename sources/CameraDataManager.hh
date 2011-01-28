#ifndef CAMERADATAMANAGER_H
#define CAMERADATAMANAGER_H
#include <irrlicht.h>
#include "util.hh"

class CameraDataManager 
{
  public:
    CameraDataManager (irr::io::IReadFile * file);

    inline void getPositionAndRotation(irr::core::vector3df & position,
                 irr::core::vector3df & rotation)
    {
      position.X=pos[0];
      position.Y=pos[1];
      position.Z=pos[2];

      rotation.X=rad2deg(rot[0]);
      rotation.Y=rad2deg(rot[1]);
      rotation.Z=rad2deg(rot[2]);
    }

  private:
    double pos[3];
    double rot[3];
};
#endif 

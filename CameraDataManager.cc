
#include "CameraDataManager.hh"
#include "gmlog.h"
#include "util.hh"

using namespace irr;

CameraDataManager::CameraDataManager(io::IReadFile * file)
{
   GM_LOG("Loading camera\n");
   double pos[3];
   double rot[3];
   Util::readTriple(file,pos);
   Util::readTriple(file,rot);
   GM_LOG("  camera position: %f,%f,%f\n",
       pos[0],
       pos[1],
       pos[2]);

   GM_LOG("  camera rotation: %f,%f,%f\n",
       rot[0],
       rot[1],
       rot[2]);
}



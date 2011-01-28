
#include "CameraDataManager.hh"
#include "gmlog.h"
#include "util.hh"

using namespace irr;

CameraDataManager::CameraDataManager(io::IReadFile * file)
{
   for(int i=0; i<3; i++) pos[i]=rot[i]=0.;
   Util::readTriple(file,pos);
   Util::readTriple(file,rot);
}



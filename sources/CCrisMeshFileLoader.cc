#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OBJ_LOADER_

#include "CCrisMeshFileLoader.h"
#include "IMeshManipulator.h"
#include "IVideoDriver.h"
#include "SMesh.h"
#include "SMeshBuffer.h"
#include "SAnimatedMesh.h"
#include "IReadFile.h"
#include "IAttributes.h"
#include "fast_atof.h"
#include "coreutil.h"

#include "Util.hh"
//#include "os.h"
//
#include <assert.h>

using namespace irr;

static const u32 WORD_BUFFER_LENGTH = 512;

/////////////////////////////////////////////////////////////////
static void logMaterial(irr::video::SMaterial & mat) {
  GM_LOG(
    " - material: diffuse (%d,%d,%d),"
    "dmbient (%d,%d,%d),"
    " emissive (%d,%d,%d),"
    " specular (%d,%d,%d), "
    " shininess %f\n",
      mat.DiffuseColor.getRed(), mat.DiffuseColor.getGreen(), mat.DiffuseColor.getBlue(),
      mat.AmbientColor.getRed(), mat.AmbientColor.getGreen(), mat.AmbientColor.getBlue(),
      mat.EmissiveColor.getRed(), mat.EmissiveColor.getGreen(), mat.EmissiveColor.getBlue(),
      mat.SpecularColor.getRed(), mat.SpecularColor.getGreen(), mat.SpecularColor.getBlue(),
      mat.Shininess);
}

/////////////////////////////////////////////////////////////////

CCrisMeshFileLoader::CCrisMeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs)
: SceneManager(smgr), FileSystem(fs)
{
	if (FileSystem)
		FileSystem->grab();
}

CCrisMeshFileLoader::~CCrisMeshFileLoader()
{
	if (FileSystem)
		FileSystem->drop();
}

bool CCrisMeshFileLoader::isALoadableFileExtension(const io::path& filename) const
{
	return core::hasFileExtension ( filename, "mesh" );
}

scene::IAnimatedMesh* CCrisMeshFileLoader::createMesh(io::IReadFile* file)
{
  const long filesize = file->getSize();
  if (!filesize)
    return 0;

  const u32 WORD_BUFFER_LENGTH = 512;

  core::array<core::vector3df> vertexBuffer;
  core::array<core::vector3df> normalsBuffer;
  core::array<core::vector2df> textureCoordBuffer;

  SObjMtl * currMtl = new SObjMtl();
  SObjMtl * defaultMtl = currMtl;
  Materials.push_back(currMtl);
  u32 smoothingGroup=0;

  const io::path fullName = file->getFileName();
  const io::path relPath = FileSystem->getFileDir(fullName)+"/";

  u16 mark;
  bool done=false;
  core::vector3df vec;
  video::S3DVertex v;
  int n_vertices,n_faces;
  char * name;
  char * matName=0;
  bool matChanged=false;
  float f[3];
  double ka[3];
  double kd[3];
  double ks[3];
  u32    flags;
  core::array<int> faceCorners;

  while(!done && file->getPos()<filesize) {
    mark=Util::readMark(file);

    switch(mark) {
      case Util::MARK_USE_MATERIAL:
        if(matName)
          delete matName;
        matName=Util::readString(file);
        matChanged=true;
        GM_LOG(" - using material: '%s'\n",matName);
        break;

      case Util::MARK_MATERIAL:
        name=Util::readString(file);
        flags=Util::readU32(file);
        Util::readTriple(file,ka);
        Util::readTriple(file,kd);
        Util::readTriple(file,ks);
        currMtl=new SObjMtl;
        currMtl->Name=name;
        Materials.push_back(currMtl);

        // TODO: handle emissive color
        currMtl->Meshbuffer->Material.DiffuseColor.setRed(kd[0]*255.0);
        currMtl->Meshbuffer->Material.DiffuseColor.setGreen(kd[1]*255.0);
        currMtl->Meshbuffer->Material.DiffuseColor.setBlue(kd[2]*255.0);
        currMtl->Meshbuffer->Material.SpecularColor.setRed(ks[0]*255.0);
        currMtl->Meshbuffer->Material.SpecularColor.setGreen(ks[1]*255.0);
        currMtl->Meshbuffer->Material.SpecularColor.setBlue(ks[2]*255.0);
        currMtl->Meshbuffer->Material.AmbientColor.setRed(ka[0]*255.0);
        currMtl->Meshbuffer->Material.AmbientColor.setGreen(ka[1]*255.0);
        currMtl->Meshbuffer->Material.AmbientColor.setBlue(ka[2]*255.0);
        GM_LOG(" - material: '%s' (flags: %X)\n",name,flags);
        break;

      case Util::MARK_VERTICES:
        n_vertices=Util::readInt(file);
        GM_LOG(" - vertices: %d\n",n_vertices);
        for(int vi=0; vi<n_vertices; vi++) {
          Util::readVertex(file,vec);
          //GM_LOG("Vertex: %f,%f,%f\n",vec.X,vec.Y,vec.Z);
          vertexBuffer.push_back(vec);
        }
        break;

      case Util::MARK_FACES_ONLY:
        n_faces=Util::readInt(file);
        GM_LOG(" - faces: %d\n",n_faces);
        if(matChanged) {
          SObjMtl * mtl = findMtl(matName);
          if(mtl)  {
            currMtl=mtl;
          } else {
            GM_LOG("Cannot find color\n");
          }
          matChanged=false;
        }

        faceCorners.reallocate(32); // should be large enough

        for(int f=0; f<n_faces; f++) {
          n_vertices=Util::readInt(file);
          for(int vi=0; vi<n_vertices; vi++) {
            int nn=Util::readInt(file);

            if(currMtl) 
              v.Color = currMtl->Meshbuffer->Material.DiffuseColor;
            v.Pos = vertexBuffer[nn];
            v.TCoords.set(0.0f,0.0f);
            v.Normal.set(0.0f,0.0f,0.0f);
            currMtl->RecalculateNormals=true;
            int vertLocation;
            core::map<video::S3DVertex, int>::Node* n = currMtl->VertMap.find(v);
            if (n) {
              vertLocation = n->getValue();
            } else {
              currMtl->Meshbuffer->Vertices.push_back(v);
              vertLocation = currMtl->Meshbuffer->Vertices.size() -1;
              currMtl->VertMap.insert(v, vertLocation);
            }
            faceCorners.push_back(vertLocation);
          }
          for ( u32 i = 1; i < faceCorners.size() - 1; ++i )
          {
            // Add a triangle
            currMtl->Meshbuffer->Indices.push_back( faceCorners[i+1] );
            currMtl->Meshbuffer->Indices.push_back( faceCorners[i] );
            currMtl->Meshbuffer->Indices.push_back( faceCorners[0] );
          }
          faceCorners.set_used(0); // fast clear
          faceCorners.reallocate(32);
        }
        break;
      default:
        //GM_LOG("Unknow mark '%X', abort loading\n",mark);
        done=true;
        break;
    }
  }

  scene::SMesh* mesh = new scene::SMesh();

  // Combine all the groups (meshbuffers) into the mesh
  for ( u32 m = 0; m < Materials.size(); ++m ) {
    if ( Materials[m]->Meshbuffer->getIndexCount() > 0 ) {
      Materials[m]->Meshbuffer->recalculateBoundingBox();
      if (Materials[m]->RecalculateNormals) {
        GM_LOG(" - recalculating normals\n");
        SceneManager->getMeshManipulator()->recalculateNormals(Materials[m]->Meshbuffer,false);
      }
      assert(Materials[m]->Meshbuffer->Material.MaterialType != video::EMT_PARALLAX_MAP_SOLID);
      irr::video::SMaterial & mat=Materials[m]->Meshbuffer->getMaterial();
      logMaterial(mat);
      mesh->addMeshBuffer( Materials[m]->Meshbuffer );
    }
  }

  // Create the Animated mesh if there's anything in the mesh
  scene::SAnimatedMesh* animMesh = 0;
  if ( 0 != mesh->getMeshBufferCount() )
  {
    mesh->recalculateBoundingBox();
    animMesh = new scene::SAnimatedMesh();
    animMesh->Type = scene::EAMT_OBJ;
    animMesh->addMesh(mesh);
    animMesh->recalculateBoundingBox();
  }

  cleanUp();
  mesh->drop();

  return animMesh;
}

CCrisMeshFileLoader::SObjMtl* CCrisMeshFileLoader::findMtl(const core::stringc& mtlName)
{
	CCrisMeshFileLoader::SObjMtl* defMaterial = 0;
	for (u32 i = 0; i < Materials.size(); ++i) {
		if ( Materials[i]->Name == mtlName ) {
				return Materials[i];
		}
	}
	return 0;
}

void CCrisMeshFileLoader::cleanUp()
{
	for (u32 i=0; i < Materials.size(); ++i )
	{
		Materials[i]->Meshbuffer->drop();
		delete Materials[i];
	}

	Materials.clear();
}


#endif // _IRR_COMPILE_WITH_OBJ_LOADER_

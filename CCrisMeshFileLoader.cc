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
//#include "os.h"
//
#include <assert.h>

using namespace irr;

static const u32 WORD_BUFFER_LENGTH = 512;

/////////////////////////////////////////////////////////////////
static void logMaterial(irr::video::SMaterial & mat) {
#if 0
  mat.AmbientColor.setRed(255);
  mat.AmbientColor.setGreen(0);
  mat.AmbientColor.setBlue(0);

  mat.EmissiveColor.setRed(255);
  mat.EmissiveColor.setGreen(0);
  mat.EmissiveColor.setBlue(0);

  mat.SpecularColor.setRed(255);
  mat.SpecularColor.setGreen(0);
  mat.SpecularColor.setBlue(0);
  
  mat.Shininess=0.;
#endif


  GM_LOG("Diffuse %d,%d,%d\n",
      mat.DiffuseColor.getRed(),
      mat.DiffuseColor.getGreen(),
      mat.DiffuseColor.getBlue());

  GM_LOG("Ambient %d,%d,%d\n",
      mat.AmbientColor.getRed(),
      mat.AmbientColor.getGreen(),
      mat.AmbientColor.getBlue());

  GM_LOG("Emissive %d,%d,%d\n",
      mat.EmissiveColor.getRed(),
      mat.EmissiveColor.getGreen(),
      mat.EmissiveColor.getBlue());

  GM_LOG("Specular %d,%d,%d\n",
      mat.SpecularColor.getRed(),
      mat.SpecularColor.getGreen(),
      mat.SpecularColor.getBlue());

  GM_LOG("Shininess %f\n",
      mat.Shininess);
}
/////////////////////////////////////////////////////////////////

//! Constructor
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
	return core::hasFileExtension ( filename, "track" );
}


scene::IAnimatedMesh* CCrisMeshFileLoader::createMesh(io::IReadFile* file)
{
  const long filesize = file->getSize();
  if (!filesize)
    return 0;

  const u32 WORD_BUFFER_LENGTH = 512;

  GM_LOG("starting\n");

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
  core::array<int> faceCorners;

  while(!done && file->getPos()<filesize) {
    mark=readMark(file);


    switch(mark) {
      case MARK_USE_MATERIAL:
        if(matName)
          delete matName;
        matName=readString(file);
        matChanged=true;
        GM_LOG("Using material: '%s'\n",matName);
        break;

      case MARK_MATERIAL:
        name=readString(file);
        //if(currMtl && currMtl != defaultMtl) {
        //  Materials.push_back(currMtl);
        //}
        currMtl=new SObjMtl;
        currMtl->Name=name;
        Materials.push_back(currMtl);
        readTriple(file,ka);
        readTriple(file,kd);
        readTriple(file,ks);

        currMtl->Meshbuffer->Material.EmissiveColor.setRed(kd[0]*255.0);
        currMtl->Meshbuffer->Material.EmissiveColor.setGreen(kd[1]*255.0);
        currMtl->Meshbuffer->Material.EmissiveColor.setBlue(kd[2]*255.0);

        currMtl->Meshbuffer->Material.DiffuseColor.setRed(kd[0]*255.0);
        currMtl->Meshbuffer->Material.DiffuseColor.setGreen(kd[1]*255.0);
        currMtl->Meshbuffer->Material.DiffuseColor.setBlue(kd[2]*255.0);

        currMtl->Meshbuffer->Material.SpecularColor.setRed(ks[0]*255.0);
        currMtl->Meshbuffer->Material.SpecularColor.setGreen(ks[1]*255.0);
        currMtl->Meshbuffer->Material.SpecularColor.setBlue(ks[2]*255.0);

        currMtl->Meshbuffer->Material.AmbientColor.setRed(ka[0]*255.0);
        currMtl->Meshbuffer->Material.AmbientColor.setGreen(ka[1]*255.0);
        currMtl->Meshbuffer->Material.AmbientColor.setBlue(ka[2]*255.0);

        GM_LOG("Read material: '%s'\n",name);
        //GM_LOG("--->Ka=%f,%f,%f",ka[0],ka[1],ka[2]);
        //GM_LOG(",Kd=%f,%f,%f",kd[0],kd[1],kd[2]);
        //GM_LOG(",Ks=%f,%f,%f\n",ks[0],ks[1],ks[2]);
        break;

      case MARK_VERTICES:
        n_vertices=readInt(file);
        GM_LOG("Loading vertices: %d\n",n_vertices);
        for(int vi=0; vi<n_vertices; vi++) {
          readVertex(file,vec);
          //GM_LOG("Vertex: %f,%f,%f\n",vec.X,vec.Y,vec.Z);
          vertexBuffer.push_back(vec);
        }
        break;

      case MARK_FACES_ONLY:
        n_faces=readInt(file);
        GM_LOG("Faces: %d (material: '%s')\n",n_faces,matName);
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
          n_vertices=readInt(file);
          for(int vi=0; vi<n_vertices; vi++) {
            int nn=readInt(file);

            if(currMtl) 
              v.Color = currMtl->Meshbuffer->Material.DiffuseColor;


            v.Color.setRed(255);
            v.Color.setBlue(0);
            v.Color.setGreen(0);
            v.Pos = vertexBuffer[nn];
            v.TCoords.set(0.0f,0.0f);
            v.Normal.set(0.0f,0.0f,0.0f);
            currMtl->RecalculateNormals=true;

            int vertLocation;
            core::map<video::S3DVertex, int>::Node* n = currMtl->VertMap.find(v);
            if (n)
            {
              vertLocation = n->getValue();
            }
            else
            {
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
        GM_LOG("Unknow mark '%X', abort loading\n",mark);
        done=true;
        break;
    }
  }

  scene::SMesh* mesh = new scene::SMesh();

  // Combine all the groups (meshbuffers) into the mesh
  for ( u32 m = 0; m < Materials.size(); ++m )
  {
    if ( Materials[m]->Meshbuffer->getIndexCount() > 0 )
    {
      Materials[m]->Meshbuffer->recalculateBoundingBox();
      if (Materials[m]->RecalculateNormals)
        SceneManager->getMeshManipulator()->recalculateNormals(Materials[m]->Meshbuffer);
      if (Materials[m]->Meshbuffer->Material.MaterialType == video::EMT_PARALLAX_MAP_SOLID)
      {
        scene::SMesh tmp;
        tmp.addMeshBuffer(Materials[m]->Meshbuffer);
        scene::IMesh* tangentMesh = SceneManager->getMeshManipulator()->createMeshWithTangents(&tmp);
        mesh->addMeshBuffer(tangentMesh->getMeshBuffer(0));
        tangentMesh->drop();
      }
      else {
        irr::video::SMaterial & mat=Materials[m]->Meshbuffer->getMaterial();
        if(mat.MaterialType==irr::video::EMT_SOLID) {
          GM_LOG("--------------------------------\n");
          logMaterial(mat);
        }
        mesh->addMeshBuffer( Materials[m]->Meshbuffer );
      }
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
	// search existing Materials for best match
	// exact match does return immediately, only name match means a new group
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


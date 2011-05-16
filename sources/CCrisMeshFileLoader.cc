//  gracing - a idiot (but physically powered) racing game 
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
#if 0
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
#endif
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

  core::array<core::vector3df> vertexBuffer;
  core::array<core::vector3df> normalsBuffer;
  core::array<core::vector2df> textureCoordBuffer;

  SObjMtl * currMtl = new SObjMtl();
  Materials.push_back(currMtl);

  const io::path fullName = file->getFileName();
  const io::path relPath = FileSystem->getFileDir(fullName)+"/";

  u16 mark;
  bool done=false;
  core::vector3df vec;
  core::vector2df vec2d;
  video::S3DVertex v;
  int n_vertices,n_faces;
  char * name;
  char * matName=0;
  char * imageName=0;
  bool matChanged=false;
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
        break;

      case Util::MARK_MATERIAL:
        name=Util::readString(file);
        flags=Util::readU32(file);
        Util::readTriple(file,ka);
        Util::readTriple(file,kd);
        Util::readTriple(file,ks);
        if(imageName)
          delete imageName;
        imageName=Util::readString(file);
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

        currMtl->Meshbuffer->Material.setFlag(irr::video::EMF_BACK_FACE_CULLING,false);
        currMtl->Meshbuffer->Material.setFlag(irr::video::EMF_LIGHTING,false);

        if(imageName[0]) {
          video::ITexture * texture = 0;
          //GM_LOG("Loading texture: '%s' on material %p\n",imageName,currMtl);
          if (FileSystem->existFile(imageName)) {
            texture = SceneManager->getVideoDriver()->getTexture(imageName);
            if(texture) {
              currMtl->Meshbuffer->Material.setTexture(0, texture);
              //currMtl->Meshbuffer->Material.MaterialType=video::EMT_TRANSPARENT_ADD_COLOR;
              //currMtl->Meshbuffer->Material.DiffuseColor.set(
              //  currMtl->Meshbuffer->Material.DiffuseColor.getAlpha(), 255, 255, 255 );
              //GM_LOG("Loaded texture\n");
            } else {
              GM_LOG("  --> cannot load texture\n");
            }
          } else {
              GM_LOG("  --> cannot find texture\n");
          }
        }
        break;

      case Util::MARK_VERTICES:
        n_vertices=Util::readInt(file);
        for(int vi=0; vi<n_vertices; vi++) {
          Util::readVertex(file,vec);
          vertexBuffer.push_back(vec);
        }
        break;

      case Util::MARK_UV_COORD:
        n_vertices=Util::readInt(file);
        for(int vi=0; vi<n_vertices; vi++) {
          Util::readVertex2d(file,vec2d);
          //GM_LOG("Texture coords: %f,%f\n",vec2d.X,vec2d.Y);
          textureCoordBuffer.push_back(vec2d);
        }
        break;


      case Util::MARK_FACES_AND_UV:
      case Util::MARK_FACES_ONLY:
        n_faces=Util::readInt(file);
        if(matChanged) {
          SObjMtl * mtl = findMtl(matName);
          if(mtl)  {
            currMtl=mtl;
          } else {
            GM_LOG("Cannot find material '%s'\n",matName);
          }
          matChanged=false;
        }

        faceCorners.reallocate(32); // should be large enough

        for(int f=0; f<n_faces; f++) {
          n_vertices=Util::readInt(file);
          for(int vi=0; vi<n_vertices; vi++) {
            int nn;
            int uvn;

            nn=Util::readInt(file);

            if(mark == Util::MARK_FACES_AND_UV) 
              uvn=Util::readInt(file);
            else
              uvn=-1;

            if(currMtl) 
              v.Color = currMtl->Meshbuffer->Material.DiffuseColor;
            v.Pos = vertexBuffer[nn];
            if(uvn==-1) 
              v.TCoords.set(0.0f,0.0f);
            else {
              v.TCoords = textureCoordBuffer[uvn];
            }
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
        SceneManager->getMeshManipulator()->recalculateNormals(Materials[m]->Meshbuffer,false);
      }
      assert(Materials[m]->Meshbuffer->Material.MaterialType != video::EMT_PARALLAX_MAP_SOLID);
      //irr::video::SMaterial & mat=Materials[m]->Meshbuffer->getMaterial();
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


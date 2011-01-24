// Copyright (C) 2002-2010 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__
#define __C_OBJ_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "irrString.h"
#include "SMeshBuffer.h"
#include "irrMap.h"

#include "gmlog.h"

//! Meshloader capable of loading obj meshes.
class CCrisMeshFileLoader : public irr::scene::IMeshLoader
{
public:

	CCrisMeshFileLoader(irr::scene::ISceneManager* smgr, irr::io::IFileSystem* fs);

	virtual ~CCrisMeshFileLoader();

	virtual bool isALoadableFileExtension(const irr::io::path& filename) const;

	virtual irr::scene::IAnimatedMesh* createMesh(irr::io::IReadFile* file);

private:

  enum {
    MARK_VERTICES=0xf100,
    MARK_FACES_ONLY=0xf101,
    MARK_MATERIAL=0xf102,
    MARK_USE_MATERIAL=0xf103,
  };

  inline int readInt(irr::io::IReadFile * file) 
  {
    int val;
    file->read(&val,sizeof(val));
    return val;
  }

  // NB: the string must be deleted!!!
  inline char * readString(irr::io::IReadFile * file)
  {
    unsigned len=readShort(file);
    char * str=new char[len+1];
    file->read(str,len);
    str[len]=0;
    return str;
  }

  inline unsigned readShort(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  inline void readTriple(irr::io::IReadFile * file, double * tr) 
  {
    tr[0]=readDouble(file);
    tr[1]=readDouble(file);
    tr[2]=readDouble(file);
  }

  inline unsigned readMark(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  inline double readDouble(irr::io::IReadFile * file)
  {
    double val;
    file->read(&val,sizeof(val));
    return val;
  }

  inline void readVertex(irr::io::IReadFile * file, irr::core::vector3df & vec)
  {
    vec.X=readDouble(file);
    vec.Y=readDouble(file);
    vec.Z=readDouble(file);
  }

	struct SObjMtl
	{
		SObjMtl() : Meshbuffer(0), Bumpiness (1.0f), Illumination(0),
			RecalculateNormals(false)
		{
			Meshbuffer = new irr::scene::SMeshBuffer();
			Meshbuffer->Material.Shininess = 0.0f;
			Meshbuffer->Material.AmbientColor = irr::video::SColorf(0.2f, 0.2f, 0.2f, 1.0f).toSColor();
			Meshbuffer->Material.DiffuseColor = irr::video::SColorf(0.8f, 0.8f, 0.8f, 1.0f).toSColor();
			Meshbuffer->Material.SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f).toSColor();
		}

		SObjMtl(const SObjMtl& o)
			: Name(o.Name), Group(o.Group),
			Bumpiness(o.Bumpiness), Illumination(o.Illumination),
			RecalculateNormals(false)
		{
			Meshbuffer = new irr::scene::SMeshBuffer();
			Meshbuffer->Material = o.Meshbuffer->Material;
		}

    irr::core::map<irr::video::S3DVertex, int> VertMap;
    irr::scene::SMeshBuffer *Meshbuffer;
    irr::core::stringc Name;
    irr::core::stringc Group;
    irr::f32 Bumpiness;
    irr::c8 Illumination;
		bool RecalculateNormals;
	};

	// helper method for material reading
	const irr::c8* readTextures(const irr::c8* bufPtr, const irr::c8* const bufEnd, 
      SObjMtl* currMaterial, const irr::io::path& relPath);

	// returns a pointer to the first printable character available in the buffer
	const irr::c8* goFirstWord(const irr::c8* buf, const irr::c8* const bufEnd, bool acrossNewlines=true);
	// returns a pointer to the first printable character after the first non-printable
	const irr::c8* goNextWord(const irr::c8* buf, const irr::c8* const bufEnd, bool acrossNewlines=true);
	// returns a pointer to the next printable character after the first line break
	const irr::c8* goNextLine(const irr::c8* buf, const irr::c8* const bufEnd);
	// copies the current word from the inBuf to the outBuf
  irr::u32 copyWord(irr::c8* outBuf, const irr::c8* inBuf, irr::u32 outBufLength, const irr::c8* const pBufEnd);
	// copies the current line from the inBuf to the outBuf
  irr::core::stringc copyLine(const irr::c8* inBuf, const irr::c8* const bufEnd);

	// combination of goNextWord followed by copyWord
	const irr::c8* goAndCopyNextWord(irr::c8* outBuf, const irr::c8* inBuf, irr::u32 outBufLength, const irr::c8* const pBufEnd);

	//! Read the material from the given file
	void readMTL(const irr::c8* fileName, const irr::io::path& relPath);

	//! Find and return the material with the given name
	SObjMtl* findMtl(const irr::core::stringc& mtlName, const irr::core::stringc& grpName);

	//! Read RGB color
	const irr::c8* readColor(const irr::c8* bufPtr, irr::video::SColor& color, const irr::c8* const pBufEnd);
	//! Read 3d vector of floats
	const irr::c8* readVec3(const irr::c8* bufPtr, irr::core::vector3df& vec, const irr::c8* const pBufEnd);
	//! Read 2d vector of floats
	const irr::c8* readUV(const irr::c8* bufPtr, irr::core::vector2df& vec, const irr::c8* const pBufEnd);
	//! Read boolean value represented as 'on' or 'off'
	const irr::c8* readBool(const irr::c8* bufPtr, bool& tf, const irr::c8* const bufEnd);

	// reads and convert to integer the vertex indices in a line of obj file's face statement
	// -1 for the index if it doesn't exist
	// indices are changed to 0-based index instead of 1-based from the obj file
	bool retrieveVertexIndices(irr::c8* vertexData, irr::s32* idx, const irr::c8* bufEnd, irr::u32 vbsize, irr::u32 vtsize, irr::u32 vnsize);

	void cleanUp();

  irr::scene::ISceneManager* SceneManager;
  irr::io::IFileSystem* FileSystem;

  irr::core::array<SObjMtl*> Materials;
};

#endif


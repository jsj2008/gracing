// Copyright (C) 2002-2010 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef CRIS_MESH_LOADER_H
#define CRIS_MESH_LOADER_H

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

	SObjMtl* findMtl(const irr::core::stringc& mtlName);

	void cleanUp();

  irr::scene::ISceneManager* SceneManager;
  irr::io::IFileSystem*      FileSystem;
  irr::core::array<SObjMtl*> Materials;
};

#endif


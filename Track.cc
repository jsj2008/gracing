#include <assert.h>
#include "gmlog.h"


#include "Track.hh"
#include "CameraDataManager.hh"

#ifndef BASE_DIR
#define BASE_DIR "."
#endif
#define MANIFEST_NAME "TRACK"

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

Track::Track(
    irr::IrrlichtDevice * device,
    const char * filename
    )
{
  m_filesystem=device->getFileSystem();

  irr::u32 cnt;

  cnt=m_filesystem->getFileArchiveCount();

  bool res=m_filesystem->addFileArchive(filename);

  if(!res) {
    return ;
  }
  assert(res);

  m_archiveIndex=cnt;

  irr::io::IFileArchive* archive=m_filesystem->
    getFileArchive(m_archiveIndex);
  assert(archive);

  
  const irr::io::IFileList * fileList=archive->getFileList();
  /*
  cnt=fileList->getFileCount();
  GM_LOG("Files in the archive: %u\n",cnt);
  */

  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);
  
  if(manifestIndex<0) {
    GM_LOG("Not a valid track file\n");
    return;
  }

  irr::io::IReadFile *  manifestFile=
    archive->createAndOpenFile(manifestIndex);

  assert(manifestFile);

  irr::io::IXMLReaderUTF8 * xmlReader=m_filesystem->createXMLReaderUTF8 (manifestFile);

  enum { MAX_DEPTH=128 };

  int i,level,ot;
  int nodeStack[MAX_DEPTH];

  int nodeStackPtr;
  irr::scene::ISceneManager* smgr = device->getSceneManager();

  irr::io::EXML_NODE nodeType;
  bool inElement;
  enum {
    ot_none,
    ot_mesh,
    ot_camera,
    ot_unknown
  };
  irr::io::IReadFile * rfile;
  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_NONE:
        GM_LOG("%d None '%s'\n",i,
            xmlReader->getNodeName(),
            xmlReader->getNodeType());
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("mesh",xmlReader->getNodeName())==0) {
          ot=ot_mesh;
        } else if(strcmp("camera",xmlReader->getNodeName())==0) {
          ot=ot_camera;
        } else {
          ot=ot_unknown;
        }
        nodeStack[++nodeStackPtr]=ot;
        inElement=true;
      break;
      case irr::io::EXN_ELEMENT_END:
        inElement=false;
        nodeStackPtr --;
      break;
      case irr::io::EXN_TEXT:
        if(inElement) {
          switch(nodeStack[nodeStackPtr]) {
            case ot_mesh:
              GM_LOG("Loading mesh: %s\n",xmlReader->getNodeName());
              rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                irr::scene::IAnimatedMesh* mesh = smgr->getMesh(rfile);
                irr::scene::IAnimatedMeshSceneNode* node=0;
                node=smgr->addAnimatedMeshSceneNode( mesh );
                if(node) {
                  node->setMaterialFlag(irr::video::EMF_LIGHTING, true);
                  //node->setMD2Animation(irr::scene::EMAT_STAND);

                  GM_LOG("Done, with %d materials:\n",node->getMaterialCount());
                  for(irr::u32 i=0; i<node->getMaterialCount(); i++) {
                    irr::video::SMaterial & mat=node->getMaterial(i);
                    logMaterial(mat);
                  }

                } else {
                  GM_LOG("Cannot load mesh\n");
                }
                rfile->drop();
              } else {
                GM_LOG("  cannot find file\n");
              }
              GM_LOG("------------------------------------------------\n");
              break;
            case ot_camera:
              GM_LOG("Loading camera data from: '%s'\n",xmlReader->getNodeName());
              irr::io::IReadFile * rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              if(rfile) {
                CameraDataManager * cammgr=new CameraDataManager(rfile);
                irr::scene::ICameraSceneNode * camera=smgr->addCameraSceneNodeFPS();
                irr::core::vector3df p,r;
                cammgr->getPositionAndRotation(p,r);
                camera->setPosition(p);
                camera->setRotation(r);
                rfile->drop();
              } else {
                GM_LOG("Cannot load camera data, using default camera\n");
                smgr->addCameraSceneNodeFPS();
              }

          }
        }
      break;
      case irr::io::EXN_COMMENT:
        GM_LOG("%d Comment '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_CDATA:
        GM_LOG("%d Cdata '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_UNKNOWN:
        GM_LOG("%d Unknown '%s'\n",i,
            xmlReader->getNodeName());
      break;
      default:
      GM_LOG("%d '%s', type: %d\n",i,
          xmlReader->getNodeName(),
          xmlReader->getNodeType());
      break;
    }
  }
  
  // TODO: drop references
}

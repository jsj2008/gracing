#include <assert.h>

#include "Track.hh"

#ifndef BASE_DIR
#define BASE_DIR "."
#endif
#define MANIFEST_NAME "TRACK"

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
  printf("Files in the archive: %u\n",cnt);
  */

  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);
  
  if(manifestIndex<0) {
    printf("Not a valid track file\n");
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
    ot_unknown
  };
  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_NONE:
        printf("%d None '%s'\n",i,
            xmlReader->getNodeName(),
            xmlReader->getNodeType());
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("mesh",xmlReader->getNodeName())==0) {
          ot=ot_mesh;
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
              printf("Loading mesh: %s\n",xmlReader->getNodeName());
              irr::io::IReadFile * rfile=archive->
                  createAndOpenFile (xmlReader->getNodeName());
              assert(rfile);
              irr::scene::IAnimatedMesh* mesh = smgr->getMesh(rfile);
              irr::scene::IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

              node=smgr->addAnimatedMeshSceneNode( mesh );
              node->setMaterialFlag(irr::video::EMF_LIGHTING, false);
              node->setMD2Animation(irr::scene::EMAT_STAND);
              rfile->drop();
              break;
          }
        }
      break;
      case irr::io::EXN_COMMENT:
        printf("%d Comment '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_CDATA:
        printf("%d Cdata '%s'\n",i,
            xmlReader->getNodeName());
      break;
      case irr::io::EXN_UNKNOWN:
        printf("%d Unknown '%s'\n",i,
            xmlReader->getNodeName());
      break;
      default:
      printf("%d '%s', type: %d\n",i,
          xmlReader->getNodeName(),
          xmlReader->getNodeType());
      break;
    }
  }
  
  // TODO: drop references
}

#include <string.h>
#include <assert.h>

#include "Vehicle.h"
#include "gmlog.h"

#define NOT_A_VALID_VEHICLE_IF_NOT(cond) assert(cond)
#define NOT_A_VALID_VEHICLE_IF(cond) assert(!(cond))

#define MANIFEST_NAME "VEHICLE"

Vehicle::Vehicle(
        irr::IrrlichtDevice * device, 
        PhyWorld * world,
        const char * source)
{
   m_device=device;
   m_world=world;
   m_filesystem=device->getFileSystem();
   m_sourceName=strdup(source);
   m_loaded=false;
}


void Vehicle::load()
{
  if(m_loaded)
    return;
  irr::u32 cnt=m_filesystem->getFileArchiveCount();

  bool res=m_filesystem->addFileArchive(m_sourceName);

  GM_LOG("--->%d\n",res);

  NOT_A_VALID_VEHICLE_IF(!res);

  irr::u32 archiveIndex=cnt;

  irr::io::IFileArchive* archive=m_filesystem->
    getFileArchive(archiveIndex);

  NOT_A_VALID_VEHICLE_IF_NOT(archive);

  const irr::io::IFileList * fileList=archive->getFileList();
  irr::s32 manifestIndex;

  manifestIndex=fileList->findFile(MANIFEST_NAME);

  NOT_A_VALID_VEHICLE_IF(manifestIndex<0);

  irr::io::IReadFile *  manifestFile=
    archive->createAndOpenFile(manifestIndex);

  assert(manifestFile);

  irr::io::IXMLReaderUTF8 * xmlReader=
    m_filesystem->createXMLReaderUTF8 (manifestFile);

  enum { MAX_DEPTH=128 };

  int i,level,ot;
  int nodeStack[MAX_DEPTH];
  enum {
    ot_none,
    ot_chassis,
    ot_wfr,
    ot_wfl,
    ot_wrr,
    ot_wrl
  };

  bool inElement;
  irr::io::EXML_NODE nodeType;

  for(int i=0, inElement=false, nodeStackPtr=0; 
      xmlReader->read(); 
      i++)
  {
    nodeType=xmlReader->getNodeType();
    switch(nodeType) {
      case irr::io::EXN_NONE:
      break;
      case irr::io::EXN_ELEMENT:
        if(strcmp("chassis",xmlReader->getNodeName())==0) {
          ot=ot_chassis;
        } else if(strcmp("wfr",xmlReader->getNodeName())==0) {
          ot=ot_wfr;
        } else if(strcmp("wfl",xmlReader->getNodeName())==0) {
          ot=ot_wfl;
        } else if(strcmp("wrr",xmlReader->getNodeName())==0) {
          ot=ot_wrr;
        } else if(strcmp("wrl",xmlReader->getNodeName())==0) {
          ot=ot_wrl;
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
            case ot_chassis:
              GM_LOG("loading chassis from '%s'\n",
                  xmlReader->getNodeName());
              break;

            case ot_wfl:
              GM_LOG("loading front left wheel from '%s'\n",
                  xmlReader->getNodeName());
              break;

            case ot_wfr:
              GM_LOG("loading front right wheel from '%s'\n",
                  xmlReader->getNodeName());
              break;

            case ot_wrl:
              GM_LOG("loading rear left wheel from '%s'\n",
                  xmlReader->getNodeName());
              break;

            case ot_wrr:
              GM_LOG("loading rear right wheel from '%s'\n",
                  xmlReader->getNodeName());
              break;
          }
        }
    }
  }
  xmlReader->drop();
  m_loaded=true;
}


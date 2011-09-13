#include "XmlNode.h"
#include "ResourceManager.h"
#include "gmlog.h"
#include "util.hh"

using namespace irr;

 
XmlNode::XmlNode(io::IXMLReaderUTF8 *xml)
{
    while(xml->getNodeType()!=io::EXN_ELEMENT && xml->read());
    readXML(xml);
}   // XMLNode

XmlNode::XmlNode(const char * name)
{
  m_name=name;
}


XmlNode::XmlNode(const std::string &filename, ResourceManager * resmanager)
{
  if(resmanager==0) 
    resmanager=ResourceManager::getInstance();

  io::IXMLReaderUTF8 * xml=resmanager->createXMLReaderUTF8(filename);

  if(!xml) {
    GM_LOG("Cannot load file %s\n",filename.c_str());
    return ;
  }

  bool is_first_element = true;

  while(xml->read()) {
    switch (xml->getNodeType()) 
    {
      case io::EXN_ELEMENT:
        {
          if(!is_first_element)
          {
            fprintf(stderr, 
                "More than one root element in '%s' - ignored.\n",
                filename.c_str());
          }
          readXML(xml);
          is_first_element = false;
          break;
        }
      case io::EXN_ELEMENT_END:  break;   // Ignore all other types
      case io::EXN_UNKNOWN:      break;
      case io::EXN_COMMENT:      break;
      case io::EXN_TEXT:         break;
      default:                   break;
    }   // switch
  }   // while

  //xml->drop();
  delete xml;
}   // XMLNode

void XmlNode::readXML(io::IXMLReaderUTF8 *xml)
{
  m_name = std::string(core::stringc(xml->getNodeName()).c_str());
  m_text = "";

  for(unsigned int i=0; i<xml->getAttributeCount(); i++) {
    std::string   name  = core::stringc(xml->getAttributeName(i)).c_str();
    core::stringw value = xml->getAttributeValue(i);
    m_attributes[name] = value;
  }   // for i

  // If no children, we are done
  if(xml->isEmptyElement())  
    return;


  /** Read all children elements. */
  while(xml->read())
  {
    switch (xml->getNodeType()) 
    {
      case io::EXN_ELEMENT:
        m_nodes.push_back(new XmlNode(xml));
        break;
      case io::EXN_ELEMENT_END:
        // End of this element found.
        return;
        break;
      case io::EXN_UNKNOWN:            break;
      case io::EXN_COMMENT:            break;
      case io::EXN_TEXT:               
        m_text=xml->getNodeName();
      break;
      default:                         break;
    }   // switch
  }   // while
}   // readXML

const void XmlNode::getChildren(const std::string &s, std::vector<XmlNode*>& out) const
{
  for(unsigned int i=0; i<m_nodes.size(); i++) {
    if(m_nodes[i]->getName()==s) {
      out.push_back(m_nodes[i]);
    }
  }
}   

XmlNode * XmlNode::getChildByAttr(const std::string & attrName, const std::string & attrValue)
{
  for(unsigned i=0; i<m_nodes.size(); i++) {
    XmlNode * node=m_nodes[i];
    std::string value;
    if(node->get(attrName,value)) {
      GM_LOG("node with '%s'='%s'\n",attrName.c_str(),value.c_str());
    }
    if(node->get(attrName,value) && value == attrValue) {
      return node;
    }
  }
  return 0;
}

/*const*/ XmlNode *XmlNode::getChild(const std::string &s) const
{
  for(unsigned int i=0; i<m_nodes.size(); i++) {
    if(m_nodes[i]->getName()==s) return m_nodes[i];
  }
  return NULL;
}  

const void XmlNode::getChildren(std::vector<XmlNode*>& out) const
{
  for(unsigned int i=0; i<m_nodes.size(); i++)
      out.push_back(m_nodes[i]);
}

XmlNode::~XmlNode()
{
  for(unsigned int i=0; i<m_nodes.size(); i++) 
    delete m_nodes[i];
  m_nodes.clear();
}


int XmlNode::get(const std::string &attribute, std::wstring & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;

  std::string vv=
    std::string(core::stringc(o->second).c_str());

  value=std::wstring(vv.begin(),vv.end());
  return 1;
}

int XmlNode::get(const std::string &attribute, std::string & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  //*value=core::stringc(o->second).c_str();
  value=core::stringc(o->second).c_str();
  return 1;
}

int XmlNode::get(const std::string &attribute, bool & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  //*value=core::stringc(o->second).c_str();
  core::stringc v=core::stringc(o->second);
  if(v == "true" || v == "yes")
    value=true;
  else
    value=false;
  return 1;
}


int XmlNode::get(const std::string &attribute, btQuaternion & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  double v[4];
  Util::parseQuaternion(core::stringc(o->second).c_str(),v);
  value.setX(v[0]);
  value.setY(v[1]);
  value.setZ(v[2]);
  value.setW(v[3]);
  return 1;
}

int XmlNode::get(const std::string &attribute, irr::core::rect<irr::s32>& rectangle) const
{
#if 0
  {
    std::map<std::string, core::stringw>::const_iterator i;
    for(i=m_attributes.begin(); i != m_attributes.end(); i++) {
      GM_LOG("-->'%s'\n",i->first.c_str());
    }
  }
#endif
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  Util::parseRect(core::stringc(o->second).c_str(),rectangle);
  return 1;
}

int XmlNode::get(const std::string &attribute, btVector3 & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  double v[3];
  Util::parseVector(core::stringc(o->second).c_str(),v);
  value.setX(v[0]);
  value.setY(v[1]);
  value.setZ(v[2]);
  return 1;
}

int XmlNode::get(const std::string &attribute, int & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  value=atoi(core::stringc(o->second).c_str());
  //*value=core::stringc(o->second).c_str();
  return 1;
}

int XmlNode::get(const std::string &attribute, unsigned & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  value=atoi(core::stringc(o->second).c_str());
  return 1;
}

int XmlNode::get(const std::string &attribute, double & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  value=irr::core::fast_atof(core::stringc(o->second).c_str());
  return 1;
}

int XmlNode::get(const std::string &attribute, irr::core::vector2d<irr::s32> &value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  Util::parseVector(core::stringc(o->second).c_str(),value);
  //*value=core::stringc(o->second).c_str();
  return 1;
}

// dont know why can write an xml with IXMLWriter and read it back
// IXMLReader.
// so i'm using a plain "FILE *" to write the xml.
//void XmlNode::save(io::IXMLWriter * xmlWriter)  const
void XmlNode::save(FILE * xml,unsigned indentWidth) const
{
  bool empty;

  empty=m_nodes.size()==0 && m_text == "";

  for(unsigned i=0; i<indentWidth; i++) 
    fprintf(xml,"  ");
  
  fprintf(xml,"<%s",m_name.c_str());
  // TODO: write attributes

  std::map<std::string, irr::core::stringw>::const_iterator it;
  if(m_attributes.size()) fprintf(xml," ");
  for(it=m_attributes.begin(); it != m_attributes.end(); it++) 
    fprintf(xml,"%s=\"%s\" ",it->first.c_str(),core::stringc(it->second).c_str());

  if(empty) 
    fprintf(xml,"/>\n");
  else if(m_nodes.size()) 
    fprintf(xml,">\n");
  else
    fprintf(xml,">");

  for(unsigned i=0; i < m_nodes.size(); i++) 
    m_nodes[i]->save(xml,indentWidth+1);

  // write the 'text' only if the node has no children
  if(m_nodes.size() == 0) 
    fprintf(xml,"%s",m_text.c_str());

  if(!empty) {
    if(m_nodes.size()) 
      for(unsigned i=0; i<indentWidth; i++) 
        fprintf(xml,"  ");
    fprintf(xml,"</%s>\n",m_name.c_str());
  }
}

void XmlNode::save(const std::string & filename) const {
  FILE * xml=fopen(filename.c_str(),"w");
  if(!xml)
    return;
  save(xml);
  fclose(xml);
}

XmlNode * XmlNode::addChild(const char * name)
{
  XmlNode * child=new XmlNode(name);

  m_nodes.push_back(child);

  return child;
}

void XmlNode::deleteAllChildren()
{
  for(unsigned i=0; i<m_nodes.size(); i++) 
    delete m_nodes[i];
  m_nodes.clear();
}

void XmlNode::set(const std::string & attribute, unsigned v)
{
  char buffer[64];
  snprintf(buffer,64,"%d",v);

  m_attributes[attribute]=buffer;
}

void XmlNode::set(const std::string & attribute, const std::string & value)
{
  m_attributes[attribute]=value.c_str();
}


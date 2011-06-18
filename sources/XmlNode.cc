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

  xml->drop();
}   // XMLNode

void XmlNode::readXML(io::IXMLReaderUTF8 *xml)
{
  m_name = std::string(core::stringc(xml->getNodeName()).c_str());
  m_text = "";

  for(unsigned int i=0; i<xml->getAttributeCount(); i++)
  {
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


const XmlNode *XmlNode::getChild(const std::string &s) const
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
  //*value=core::stringc(o->second).c_str();
  return 1;
}

int XmlNode::get(const std::string &attribute, double & value) const
{
  if(m_attributes.size()==0) return 0;
  std::map<std::string, core::stringw>::const_iterator o;
  o = m_attributes.find(attribute);
  if(o==m_attributes.end()) return 0;
  value=irr::core::fast_atof(core::stringc(o->second).c_str());
  //*value=core::stringc(o->second).c_str();
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

#if 0
int XmlNode::get(const std::string &attribute, core::vector2df *value) const
{
    std::string s = "";
    if(!get(attribute, &s)) return 0;

    //std::vector<std::string> v = StringUtils::split(s,' ');
    //Util::parseVector2d(s.c_str(), *value);
#if 0
    if(v.size()!=2) return 0;
    value->X = (float)atof(v[0].c_str());
    value->Y = (float)atof(v[1].c_str());
#endif
    return 1;
}   // get(vector2df)


int XmlNode::get(const std::string &attribute, core::vector3df *value) const
{
#if 0
    Vec3 xyz;
    if(!get(attribute, &xyz)) return 0;

    *value = xyz.toIrrVector();
#endif
    return 1;
}   // get(vector3df)
#endif


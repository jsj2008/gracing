//  gracing - an idiot (but physically powered) racing game 
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

#ifndef XMLNODE_H
#define XMLNODE_H
#include <string>
#include <vector>
#include <map>
#include <irrlicht.h>
#include "btBulletDynamicsCommon.h"

#include "ResourceManager.h"

class XmlNode
{
  public:
    XmlNode(const std::string & filename, ResourceManager * resmanager=0);
    XmlNode(irr::io::IXMLReaderUTF8 *xml);

    ~XmlNode();

    /* return all children with a given name*/
    const void getChildren(const std::string &s, std::vector<XmlNode*>& out) const;

    /* return all children */
    const void getChildren(std::vector<XmlNode*>& out) const;

    /* get first child with a given name */
    const XmlNode * getChild(const std::string &s) const;

    /* get name of this node */
    const std::string &getName() const {return m_name; }

    /* get text of this node */
    const std::string &getText() const {return m_text; }

    // get value as string
    int get(const std::string &attribute,std::string & value) const;

    // get value as double
    int get(const std::string &attribute, double & value) const;

    int get(const std::string &attribute, btQuaternion & value) const;

    // get value as 2d vector
    int get(const std::string &attribute, irr::core::vector2df *value) const;

    // get value as 3d vector
    int get(const std::string &attribute, irr::core::vector3df *value) const;

    // get value as a bullet 3d vector
    int get(const std::string &attribute, btVector3 & value) const;


  private:

    void readXML(irr::io::IXMLReaderUTF8 * xml);
    std::string m_name;
    std::string m_text;
    std::map<std::string, irr::core::stringw> m_attributes;
    std::vector<XmlNode *> m_nodes;
};
#endif

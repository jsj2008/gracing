#!BPY
"""
Name: 'Crisalide track/vehicle exporter...'
Blender: 253
Group: 'Export'
Tooltip: 'Exports meshes to crisalide'
"""
__author__ = ['Gianni Masullo']
__version__ = '0.4'
__url__ = [ ]
__bpydoc__ = """\
Crisalide xml exporter

"""
import bpy,os,struct,zipfile,math,time,mathutils,shutil,string

from bpy.props import *
from io_utils import ExportHelper, ImportHelper
from io_utils import create_derived_objects, free_derived_objects
import re

"""
cris binary file format

- MARK_VERTICES, n_vertices, ...<vertices>...

- MARK_MATERIALS

- MARK_USE_MATERIAL, material_name

- MARK_FACES_ONLY 
"""

# TODO: get "custom property" from mat to
#       set flags: uptonow they are set to default
#       in getMaterialFlagsWord

# TODO: change naming convention of the xmlnodes for using "-"

# LOG configuration
LOG_ON_STDOUT=1
LOG_ON_FILE=0
LOG_FILENAME="/tmp/log.txt"
EPSILON=0.0001

BIG_NUMBER=10000
NEG_BIG_NUMBER=-BIG_NUMBER

#########################################
EXPORT_XML_MESH=False

# markers 
MARK_VERTICES=0xf100
MARK_FACES_ONLY=0xf101
MARK_MATERIAL=0xf102
MARK_USE_MATERIAL=0xf103
MARK_UVCOORDS=0xf104
MARK_FACES_AND_UV=0xf105

# light types
LAMP_TYPE_POINT=0
LAMP_TYPE_SUN=1
LAMP_TYPE_SPOT=2
LAMP_TYPE_HEMI=3
LAMP_TYPE_AREA=4

# material flags
MATERIA_FLAGS_LIST=[
  { 'name': 'EMF_WIREFRAME',          'value': 0x1, "default": False },
  { 'name': 'EMF_POINTCLOUD',         'value': 0x2, "default": False },
  { 'name': 'EMF_GOURAUD_SHADING',    'value': 0x4, "default": True },
  { 'name': 'EMF_LIGHTING',           'value': 0x8, "default": False }, #True },
  { 'name': 'EMF_ZBUFFER',            'value': 0x10, "default": True },
  { 'name': 'EMF_ZWRITE_ENABLE',      'value': 0x20, "default": True },
  { 'name': 'EMF_BACK_FACE_CULLING',  'value': 0x40, "default": True },
  { 'name': 'EMF_FRONT_FACE_CULLING', 'value': 0x80, "default": False },
  { 'name': 'EMF_BILINEAR_FILTER',    'value': 0x100, "default": True },
  { 'name': 'EMF_TRILINEAR_FILTER',   'value': 0x200, "default": False },
  { 'name': 'EMF_ANISOTROPIC_FILTER', 'value': 0x400, "default": False },
  { 'name': 'EMF_FOG_ENABLE',         'value': 0x800, "default": False },
  { 'name': 'EMF_NORMALIZE_NORMALS',  'value': 0x1000, "default": False },
  { 'name': 'EMF_TEXTURE_WRAP',       'value': 0x2000, "default": False },
  { 'name': 'EMF_ANTI_ALIASING',      'value': 0x4000, "default": False },
  { 'name': 'EMF_COLOR_MASK',         'value': 0x8000, "default": False },
  { 'name': 'EMF_COLOR_MATERIAL',     'value': 0x10000, "default": False }
]

WORLD_PROPERTIES=[
  { 'name': 'gravity', 'default': [ 0., -10., 0.] },
  { 'name': 'skydome', 'default': "" }
]

WIDX_FRONT_LEFT=0
WIDX_FRONT_RIGHT=1
WIDX_REAR_LEFT=2
WIDX_REAR_RIGHT=3

WHEEL_PREFIX=[ 'wfl_', 'wfr_', 'wrl_', 'wrr_' ]


def buildPath(rel):
  b=os.path.dirname(bpy.data.filepath)
  return b + "/" + rel

class BBound:
  def __init__(self):
    self.minX = BIG_NUMBER
    self.maxX = NEG_BIG_NUMBER
    self.minY = BIG_NUMBER
    self.maxY = NEG_BIG_NUMBER
    self.minZ = BIG_NUMBER
    self.maxZ = NEG_BIG_NUMBER

  def reset(self):
    self.minX = BIG_NUMBER
    self.maxX = NEG_BIG_NUMBER
    self.minY = BIG_NUMBER
    self.maxY = NEG_BIG_NUMBER
    self.minZ = BIG_NUMBER
    self.maxZ = NEG_BIG_NUMBER

  def update(self,point):
    if point[0] < self.minX:
      self.minX=point[0]

    if point[0] > self.maxX:
      self.maxX=point[0]

    if point[1] > self.maxY:
      self.maxY=point[1]

    if point[1] < self.minY:
      self.minY=point[1]

    if point[2] > self.maxZ:
      self.maxZ=point[2]

    if point[2] < self.minZ:
      self.minZ=point[2]

  def updateFromAABB(self,aabb):
    for v in aabb:
      self.update(v)

  def getOffsets(self):
    point=[ 0., 0., 0. ]
    point[0]=-(self.minX + self.maxX) / 2.0;
    point[1]=-(self.minY + self.maxY) / 2.0;
    point[2]=-(self.minZ + self.maxZ) / 2.0;
    return point

  def getCrisOffsets(self):
    point=[ 0., 0., 0. ]
    point[0]=-(self.minX + self.maxX) / 2.0;
    point[1]=-(self.minZ + self.maxZ) / 2.0;
    point[2]=-(self.minY + self.maxY) / 2.0;
    return point

  def logDim(self):
    minY=self.minY + (self.minZ + self.maxZ) / 2.0
    maxY=self.maxY + (self.minZ + self.maxZ) / 2.0
    log("-->%f,%f"%(minY,maxY))
    log("BBound dim (%f,%f) Y:(%f,%f) Z:(%f,%f)"%(
          self.minX,self.maxX,
          self.minZ,self.maxZ,
          self.minY,self.maxY))


def copy_image(iname,dest_dir):
  fn = bpy.path.abspath(iname)
  fn = os.path.normpath(fn)
  fn_strip = os.path.basename(fn)

  rel = fn_strip
  fn_abs_dest = os.path.join(dest_dir, fn_strip)
  if not os.path.exists(fn_abs_dest):
    log("COPYING '%s' to '%s'"%(fn,fn_abs_dest))
    try:
      shutil.copy(fn, fn_abs_dest)
    except:
      # have mercy on me!
      # still dont understand how blender handles
      # image files.
      log("WARNING: cannot copy image file (src: '%s', dst: '%s')"%(fn,fn_abs_dest))
      src_name=os.path.basename(fn)
      log("         trying with base name (src: '%s', dst: '%s'"%(src_name,fn_abs_dest))
      try:
        shutil.copy(src_name, fn_abs_dest)
      except:
        log("         cannot even copy image file with basename (src: '%s', dst: '%s')"%(src_name,fn_abs_dest))
        

  return fn_abs_dest
    
def getMaterialFlagsWord(mat):
  word=0
  for v in MATERIA_FLAGS_LIST:
    if v["default"]:
      word = word | v["value"]
  return word
      

def fixName(name):
  if name is None:
    return 'None'
  else:
    return name.replace(' ', '_')

def log(str):
  if LOG_ON_STDOUT==1:
#print(str,end="")
    print(str)
  if LOG_ON_FILE==1:
    f=open(LOG_FILENAME,"a")
    f.write(str)
    f.write("\n")
    f.close()

def binWrite_mark(fp,value):
  fp.write(struct.pack("H",value))

def binWrite_u32(fp,value):
  fp.write(struct.pack("I",value))

def binWrite_float(fp,value):
  v=struct.pack("d",value)
  fp.write(v)

def binWrite_int(fp,value):
  v=struct.pack("i",value)
  fp.write(v)

def binWrite_pointVect(fp,point):
  v=struct.pack("ddd",point[0],point[2],point[1])
  fp.write(v)

def binWrite_pointUV(fp,point):
  v=struct.pack("dd",point[0],point[1])
  fp.write(v)

def binWrite_color(fp,color):
  v=struct.pack("ddd",color[0],color[1],color[2])
  fp.write(v)
 
def binWrite_colorNode(fp,color):
  r=float(color.getProp("R"))
  g=float(color.getProp("G"))
  b=float(color.getProp("B"))
  v=struct.pack("ddd",r,g,b)
  fp.write(v)

def binWrite_faceNode(fp,face,writeUV=False):
  t0=int(face.getProp("V0"))
  t1=int(face.getProp("V1"))
  t2=int(face.getProp("V2"))
  binWrite_int(fp,3)
  binWrite_int(fp,t0)
  if writeUV:
    binWrite_int(fp,t0)
  binWrite_int(fp,t1)
  if writeUV:
    binWrite_int(fp,t1)
  binWrite_int(fp,t2)
  if writeUV:
    binWrite_int(fp,t2)


def binWrite_string(fp,str):
  l=len(str)
  fp.write(struct.pack("H",l))
  for c in str:
    fp.write(struct.pack("c",c))

def binWrite_face(fp,face):
  if len(face) == 4:
    v=struct.pack("iiiii",4,face[0]+1, face[1]+1, face[2]+1,face[3]+1)
  else:
    v=struct.pack("iiii",3,face[0]+1, face[1]+1, face[2]+1)
  fp.write(v)

def binWrite_textureCoords(fp,n_vertices,data):
  if n_vertices==4:
    v=struct.pack("ffffffff" , data.uv1[1], -data.uv1[0],\
      data.uv2[1], -data.uv2[0],\
      data.uv3[1], -data.uv3[0],\
      data.uv4[1], -data.uv4[0])
  else:
    v=struct.pack("ffffff" , data.uv1[1], -data.uv1[0],\
      data.uv2[1], -data.uv2[0],\
      data.uv3[1], -data.uv3[0])
  fp.write(v)

def binWrite_faceWithTexture(fp,face,textIndex):
  if len(f.vertices) == 4:
    v=struct.pack("iiii",face[0]+1, face[1]+1, face[2]+1,face[3]+1)
    fp.write("iiiiiiii" ,vs[0]+1, vt, vs[1]+1, vt+1, vs[2]+1, vt+2, vs[3], vt)
  else:
    fp.write("iiiiii",vs[0]+1, vt, vs[1]+1, vt+1, vs[2]+1, vt+2)

  fp.write(v)

###################################################################


class XmlNode: 
  def __init__(self,name):
    self.x_name=name
    self.x_children=[ ]
    self.x_props={ }
    self.x_defaultProps=None
    self.x_text=""
    self.x_internalProps={ }
    self.x_defaultInternalProps=None

  def setProp(self,pname,pvalue):
    self.x_props[pname]=pvalue 

  def setInternalProp(self,pname,pvalue):
    self.x_internalProps[pname]=pvalue

  def getInternalProp(self,pname):
    if pname in self.x_internalProps:
      return self.x_internalProps[pname]
    return self.x_defaultInternalProps

  def getProp(self,pname):
    if pname in self.x_props:
      return self.x_props[pname]
    return self.x_defaultProps
  
  def forceType(self,type):
    self.x_name=type

  def getChild(self,childName):
    for child in self.x_children:
      if child.x_name == childName:
        return child
    return None
 
  def getChildrenNumber(self):
    return len(self.x_children)

  def addChild(self,child):
    if child == None:
      ErrorError()
    if child == self:
      RaiseError("Recursive addChils")
    self.x_children.append(child)

  def getChildrenList(self):
    return self.x_children

  def setText(self,text):
    self.x_text=text

  def getText(self):
    return self.x_text

  def getName(self):
    return self.x_name

  def export(self):
    for child in self.x_children:
      child.export()

  def cleanup(self):
    for child in self.x_children:
      child.cleanup()

  def getRecurse(self):
    return True

  def writeSubTree(self,outFile,forceRecurse=False):
    outFile.write("<%s"%(self.x_name))
    if len(self.x_props.keys()):
      for prop in self.x_props:
        outFile.write(" %s=\"%s\""%(prop,self.x_props[prop]))

    mustRecurse=self.getRecurse() or forceRecurse
    
    if (mustRecurse and len(self.x_children)) or self.x_text!="":
      outFile.write(">");
      if mustRecurse and len(self.x_children) > 0:
        outFile.write("\n")
        for child in self.x_children:
          child.writeSubTree(outFile)
      if self.x_text != "":
        outFile.write(self.x_text)
      outFile.write("</%s>\n"%(self.x_name))
    else:
      outFile.write("/>\n")


class XmlMeshNode(XmlNode):
  def __init__(self,ob,mesh,materials,dest_dir):
    XmlNode.__init__(self,"mesh")
    self.x_dest_dir=dest_dir
    self.x_ofilename=dest_dir+"/"+ob.name+".mesh"
    self.x_exportXmlMesh = False

    self.setText(os.path.basename(self.x_ofilename))

    tri_list = extract_triangles(mesh)

    if len(mesh.uv_textures):
      vert_array, uv_array, tri_list = remove_face_uv(mesh.vertices,tri_list)
    else:
      vert_array = [ ]
      for vert in mesh.vertices:
        vert_array.append(vert.co)
      # If the mesh has vertex UVs, create an array of UVs:
      if len(mesh.sticky):
        uv_array = [ ]
        for uv in mesh.sticky:
          uv_array.append(uv.co)
      else:
        # no UV at all:
        uv_array = None

    mats=XmlNode("materials")
    self.addChild(mats)
    for mat_and_image in materials.values():
      node=createMaterialNode(mat_and_image[0], mat_and_image[1])
      mats.addChild(node)

    idx=0
    vertices=XmlNode("vertices")
    self.addChild(vertices)
    for v in vert_array:
      node=XmlNode("vertex")
      #node.setText("%f,%f,%f"%(v[0],v[1],v[2]))
      node.setProp("index",idx)
      node.setProp("X",v[0])
      node.setProp("Y",v[1])
      node.setProp("Z",v[2])
      idx=idx+1
      vertices.addChild(node)

    if uv_array:
      idx=0
      uvcoords=XmlNode("uvcoords")
      self.addChild(uvcoords)
      for uvc in uv_array:
        node=XmlNode("uvcord")
        node.setProp("index",idx)
        node.setProp("U",uvc[0])
        node.setProp("V",-uvc[1])
#node.setProp("U",uvc[0])
#node.setProp("V",uvc[1])

        idx=idx+1
        uvcoords.addChild(node)

    node=self.produceFacesChildren(tri_list, mesh)
    self.addChild(node)
  
  def getRecurse(self):
    return False

  def getMeshFileName(self):
    return self.x_ofilename
 
  def cleanup(self):
    try:
      os.remove(self.x_ofilename)
    except:
      pass
    try:
      os.remove(self.x_ofilename+".xml")
    except:
      pass
    XmlNode.cleanup(self)

  def applyTranslationToVertices(self,trasl):
    vertices=self.getChild("vertices")
    for node in vertices.getChildrenList():
      co=[0., 0., 0.]
      co[0]=float(node.getProp("X"))+trasl[0]
      co[1]=float(node.getProp("Y"))+trasl[1]
      co[2]=float(node.getProp("Z"))+trasl[2]
      node.setProp("X",co[0])
      node.setProp("Y",co[1])
      node.setProp("Z",co[2])

  def exportBinary(self,filename):
    vertices=self.getChild("vertices")
    uvcoords=self.getChild("uvcoords")
    materials=self.getChild("materials")
    faceGroups=self.getChild("faces")
    fp=open(filename,"wb")

    # export vertices #
    binWrite_mark(fp,MARK_VERTICES)
    binWrite_int(fp,vertices.getChildrenNumber())
    for node in vertices.getChildrenList():
      co=[0., 0., 0.]
      co[0]=float(node.getProp("X"))
      co[1]=float(node.getProp("Y"))
      co[2]=float(node.getProp("Z"))
      binWrite_pointVect(fp,co)

    # export uvcoords #
    if uvcoords:
      binWrite_mark(fp,MARK_UVCOORDS)
      binWrite_int(fp,uvcoords.getChildrenNumber())

      for node in uvcoords.getChildrenList():
        uv=[ float(node.getProp("U")) , float(node.getProp("V")) ]
        binWrite_pointUV(fp,uv)

    # export materials and copy image(s) #
    for mat in materials.getChildrenList():
      word=getMaterialFlagsWord(mat)
      binWrite_mark(fp,MARK_MATERIAL)

      binWrite_string(fp,mat.getProp("name"))
      binWrite_u32(fp,word)
      binWrite_colorNode(fp,mat.getChild("ambient_color"))
      binWrite_colorNode(fp,mat.getChild("diffuse_color"))
      binWrite_colorNode(fp,mat.getChild("specular_color"))

      # there should only an image !! #
      iname=""
      for inode in mat.getChildrenList():
        if inode.getName() != 'image':
          continue
        iname = inode.getInternalProp("filepath")
        copy_image(iname,self.x_dest_dir)

      binWrite_string(fp,iname)

    # export faces #
    for fgroup in faceGroups.getChildrenList():
      binWrite_mark(fp,MARK_USE_MATERIAL)
      binWrite_string(fp,fgroup.getProp("name"))

      if uvcoords != None:
        binWrite_mark(fp,MARK_FACES_AND_UV)
      else:
        binWrite_mark(fp,MARK_FACES_ONLY)
      binWrite_int(fp,fgroup.getChildrenNumber())
      for face in fgroup.getChildrenList():
        binWrite_faceNode(fp,face,uvcoords != None)
      
    fp.close()
 
  def setExportXmlMesh(self,value):
    self.x_exportXmlMesh=value

  def export(self):
    self.exportBinary(self.x_ofilename)
    if self.x_exportXmlMesh:
      f=open(self.x_ofilename+".xml","w")
      self.writeSubTree(f,True)
      f.close()

  def produceFacesChildren(self, tri_list, mesh):
    root=XmlNode("faces")
    faces_list=[ ]
    materials = mesh.materials
    if not materials:
      mat = None
    if len(mesh.uv_textures):
      unique_mats={}
      for i, tri in enumerate(tri_list):
        faces_list.append(tri.vertex_index)
        if materials:
          mat = materials[tri.mat]
          if mat: mat=mat.name

        img = tri.image

        try:
          context_mat_face_array = unique_mats[mat, img][1]
        except:
          if mat: name_str = mat
          else: name_str = "None"
          if img: name_str += img

          context_mat_face_array = [ ]
          unique_mats[mat, img] = name_str, context_mat_face_array

        context_mat_face_array.append(i)
        
      for mat_name, mat_faces in unique_mats.values():
        mnode=XmlNode("use_material")
        mnode.setProp("name",mat_name)
        for ff in mat_faces:
          node=XmlNode("face")
          tri=faces_list[ff]
          node.setProp("V0",tri[0])
          node.setProp("V1",tri[1])
          node.setProp("V2",tri[2])
          #node.setText("%d,%d,%d"%(tri[0],
          #  tri[1],
          #  tri[2]))
          mnode.addChild(node)
        root.addChild(mnode)

    else:
      obj_material_faces=[]
      obj_material_names=[]

      for m in materials:
        if m:
          obj_material_names.append(m.name)
          obj_material_faces.append([])
       
      n_materials=len(obj_material_names)

      for i,tri in enumerate(tri_list):
        faces_list.append(tri.vertex_index)
        if (tri.mat < n_materials):
          obj_material_faces[tri.mat].append(i)

      for  i in range(n_materials):
        findex=obj_material_faces[i]
        mnode=XmlNode("use_material")
        mnode.setProp("name",obj_material_names[i])
        root.addChild(mnode)

        faces=obj_material_faces[i]
        for ff in faces:
          node=XmlNode("face")
          tri=faces_list[ff]
          node.setProp("V0",tri[0])
          node.setProp("V1",tri[1])
          node.setProp("V2",tri[2])
          node.setText("%d,%d,%d"%(tri[0],
            tri[1],
            tri[2]))
          mnode.addChild(node)

    return root

    

class tri_wrapper(object):
    '''Class representing a triangle.

    Used when converting faces to triangles'''

    __slots__ = 'vertex_index', 'mat', 'image', 'faceuvs', 'offset'
    def __init__(self, vindex=(0,0,0), mat=None, image=None, faceuvs=None):
        self.vertex_index= vindex
        self.mat= mat
        self.image= image
        self.faceuvs= faceuvs
        self.offset= [0, 0, 0] # offset indicies

def uv_key(uv):
    return round(uv[0], 6), round(uv[1], 6)


def extract_triangles(mesh):
    '''Extract triangles from a mesh.

    If the mesh contains quads, they will be split into triangles.'''
    tri_list = []
    do_uv = len(mesh.uv_textures)
    img = None
    for i, face in enumerate(mesh.faces):
        f_v = face.vertices

        uf = mesh.uv_textures.active.data[i] if do_uv else None

        if do_uv:
            f_uv = uf.uv
            img = uf.image if uf else None
            if img: img = img.name

        if len(f_v)==3:
            new_tri = tri_wrapper((f_v[0], f_v[1], f_v[2]), face.material_index, img)
            if (do_uv): new_tri.faceuvs= uv_key(f_uv[0]), uv_key(f_uv[1]), uv_key(f_uv[2])
            tri_list.append(new_tri)

        else: #it's a quad
            new_tri = tri_wrapper((f_v[0], f_v[1], f_v[2]), face.material_index, img)
            new_tri_2 = tri_wrapper((f_v[0], f_v[2], f_v[3]), face.material_index, img)

            if (do_uv):
                new_tri.faceuvs= uv_key(f_uv[0]), uv_key(f_uv[1]), uv_key(f_uv[2])
                new_tri_2.faceuvs= uv_key(f_uv[0]), uv_key(f_uv[2]), uv_key(f_uv[3])

            tri_list.append( new_tri )
            tri_list.append( new_tri_2 )

    return tri_list

def remove_face_uv(verts, tri_list):
    '''Remove face UV coordinates from a list of triangles.

    Since 3ds files only support one pair of uv coordinates 
    for each vertex, face uv coordinates
    need to be converted to vertex uv coordinates. 
    That means that vertices need to be duplicated when
    there are multiple uv coordinates per vertex.'''

    # initialize a list of UniqueLists, one per vertex:
    unique_uvs= [{} for i in range(len(verts))]

    # for each face uv coordinate, add it to the UniqueList of the vertex
    for tri in tri_list:
        for i in range(3):
            context_uv_vert= unique_uvs[tri.vertex_index[i]]
            uvkey= tri.faceuvs[i]

            offset_index__uv_3ds = context_uv_vert.get(uvkey)

            if not offset_index__uv_3ds:
                offset_index__uv_3ds = context_uv_vert[uvkey] = len(context_uv_vert), uvkey

            tri.offset[i] = offset_index__uv_3ds[0]

    # At this point, each vertex has a UniqueList containing every 
    # uv coordinate that is associated with it
    # only once.

    # Now we need to duplicate every vertex as many times as 
    # it has uv coordinates and make sure the
    # faces refer to the new face indices:
    vert_index = 0
    vert_array = [ ] # _3ds_array()
    uv_array = [ ]  # _3ds_array()
    index_list = []
    for i,vert in enumerate(verts):
        index_list.append(vert_index)

        pt = vert.co # reuse, should be ok
        uvmap = [None] * len(unique_uvs[i])
        for ii, uv_co in unique_uvs[i].values():
            # add a vertex duplicate to the vertex_array for every uv associated with this vertex:
            #vert_array.add(pt)
            vert_array.append(pt)

            # add the uv coordinate to the uv array:
            # This for loop does not give uv's ordered by ii, so we create a new map
            # and add the uv's later
            # uv_array.add(uv_co)
            uvmap[ii] = uv_co

        # Add the uv's in the correct order
        for uv_co in uvmap:
            # add the uv coordinate to the uv array:
            uv_array.append(uv_co)

        vert_index += len(unique_uvs[i])

    # Make sure the triangle vertex indices now refer to the new vertex list:
    for tri in tri_list:
        for i in range(3):
            tri.offset[i]+=index_list[tri.vertex_index[i]]
        tri.vertex_index= tri.offset

    return vert_array, uv_array, tri_list

def createColorNode(name,color):
  node=XmlNode(name)
  node.setText("%f,%f,%f"%(color[0],color[1],color[2]))
  node.setProp("R",color[0])
  node.setProp("G",color[1])
  node.setProp("B",color[2])
  return node

def get_material_images(material):
  # blender utility func.
  if material:
    return [s.texture.image for s in material.texture_slots if s and s.texture.type == 'IMAGE' and s.texture.image]
  return []
  
def createMaterialNode(material, image):
  if material:	
    name_str = material.name
  else:
    name_str = 'None'
  if image:
    name_str += image.name

  matNode=XmlNode("material")
  matNode.setProp("name",name_str)

  if not material:
    matNode.addChild(createColorNode("ambient_color",(0.,0.,0.)))
    matNode.addChild(createColorNode("diffuse_color",(0.8,0.8,0.8)))
    matNode.addChild(createColorNode("specular_color",(1.,1.,1.)))
  else:
    matNode.addChild(createColorNode("ambient_color", [a*material.ambient for a in material.diffuse_color]))
    matNode.addChild(createColorNode("diffuse_color", material.diffuse_color))
    matNode.addChild(createColorNode("specular_color", material.specular_color))

  images = get_material_images(material) # can be None
  if image: images.append(image)

  if images:
    for image in images:
      node=XmlNode("image")
      node.setText(os.path.basename(image.filepath))
      node.setInternalProp("filepath",image.filepath)
      matNode.addChild(node)

  return matNode


inserted_images={ }
def createZipFile(root,xmlfile,path,srcdir):
  global inserted_images
  zf=zipfile.ZipFile(path,"w")
  for node in root.getChildrenList():
    if isinstance(node,XmlMeshNode):
      fn=node.getMeshFileName()
      p=os.path.basename(fn)
      log("Zipping '%s' as '%s'"%(fn,p))
      try:
        zf.write(fn,p)
      except:
        log("Cannot zip the file!")
        continue
      mats=node.getChild("materials")
      for mat in mats.getChildrenList():
        for inode in mat.getChildrenList():
          if inode.getName() != 'image':
            continue
          iname = os.path.basename(inode.getText())
          if iname not in inserted_images.keys():
            inserted_images[iname]=True
            dname = iname
            iname = srcdir + "/" + iname
            if os.path.exists(iname):
              log("Zipping '%s' as '%s'"%(iname,dname))
              zf.write(iname,dname)
            else:
              log("skipping image (src: '%s', dst: '%s')"%(iname,dname))
    if node.x_name == "skydome":
      for t in node.getChildrenList():
        iname=os.path.basename(t.getProp("src"))
        if iname in inserted_images.keys():
          continue
        inserted_images[iname]=True
        dname = iname
        iname = srcdir + "/" + iname
        if os.path.exists(iname):
          log("Zipping skydome '%s' as '%s'"%(iname,dname))
          zf.write(iname,dname)
        else:
          log("skipping skydome image (src: '%s', dst: '%s')"%(iname,dname))

  log("Zipping '%s' as '%s'"%(xmlfile,os.path.basename(xmlfile)))
  zf.write(xmlfile,os.path.basename(xmlfile))
  zf.close()

def createMaterialDictAndMeshList2(scene,applyTransform=True):
    materialDict = {}
    mesh_objects = []
    for ob in [ob for ob in scene.objects if ob.is_visible(scene)]:

      free, derived = create_derived_objects(scene, ob)

      if derived is None:
        continue
    
      for ob_derived, mat in derived:
        if ob.type not in ('MESH', 'CURVE', 'SURFACE', 'FONT', 'META'):
          continue

        data = ob_derived.create_mesh(scene, True, 'PREVIEW')
        if data:
          if applyTransform:
            data.transform(mat)

          mesh_objects.append((ob_derived, data))
          mat_ls = data.materials
          mat_ls_len = len(mat_ls)

          # get material/image tuples.
          if len(data.uv_textures):
            if not mat_ls:
              mat = mat_name = None

            for f, uf in zip(data.faces, data.uv_textures.active.data):
              if mat_ls:
                mat_index = f.material_index
                if mat_index >= mat_ls_len:
                  mat_index = f.mat = 0
                mat = mat_ls[mat_index]
                if mat:	mat_name = mat.name
                else:	mat_name = None
              # else there already set to none

              img = uf.image
              if img:	img_name = img.name
              else:	img_name = None

              materialDict.setdefault((mat_name, img_name), (mat, img) )

          else:
            for mat in mat_ls:
              if mat: # material may be None so check its not.
                materialDict.setdefault((mat.name, None), (mat, None) )

            for f in data.faces:
              if f.material_index >= mat_ls_len:
                f.material_index = 0

        if free:
            free_derived_objects(ob)
    return materialDict, mesh_objects

def createMaterialDictAndMeshList(scene,object,applyTransform=True):
    materialDict = {}
    mesh_objects = []

    for ob in [object]:

      free, derived = create_derived_objects(scene, ob)

      if derived is None:
        continue
    
      for ob_derived, mat in derived:
        if ob.type not in ('MESH', 'CURVE', 'SURFACE', 'FONT', 'META'):
          continue

        data = ob_derived.create_mesh(scene, True, 'PREVIEW')
        if data:
          if applyTransform:
            data.transform(mat)

          mesh_objects.append((ob_derived, data))
          mat_ls = data.materials
          mat_ls_len = len(mat_ls)

          # get material/image tuples.
          if len(data.uv_textures):
            if not mat_ls:
              mat = mat_name = None

            for f, uf in zip(data.faces, data.uv_textures.active.data):
              if mat_ls:
                mat_index = f.material_index
                if mat_index >= mat_ls_len:
                  mat_index = f.mat = 0
                mat = mat_ls[mat_index]
                if mat:	mat_name = mat.name
                else:	mat_name = None
              # else there already set to none

              img = uf.image
              if img:	img_name = img.name
              else:	img_name = None

              materialDict.setdefault((mat_name, img_name), (mat, img) )

          else:
            for mat in mat_ls:
              if mat: # material may be None so check its not.
                materialDict.setdefault((mat.name, None), (mat, None) )

            for f in data.faces:
              if f.material_index >= mat_ls_len:
                f.material_index = 0

        if free:
            free_derived_objects(ob)
    return materialDict, mesh_objects

class export_OT_vehicle(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_vehicle_exporter"
  bl_label = "Export crisalide file formats"
  
  filename_ext = ".zip"
  filter_glob = StringProperty(default="*.zip", options={'HIDDEN'})

  filepath = bpy.props.StringProperty(
    name="File Path", 
    description="File path used for exporting the crisalide file", 
    maxlen= 1024, default= "")

  exportXML = bpy.props.BoolProperty(
    name="Export xml", 
    description="Export also xml files of the meshes (debug purposes)",
    default=EXPORT_XML_MESH)

  exportEXP = bpy.props.BoolProperty(
    name="Export exp", 
    description="Export only object with name starting with 'exp.' (debug purposes",
    default=False)
  
  chassis_bbound =  BBound()

  def extractTextInfo(self):
    root=XmlNode("info")
    for textFile in bpy.data.texts:
      if textFile.name != "data":
        continue
      for line in textFile.lines:
        body=line.body
        couple=re.split('=',body)
        if len(couple) != 2:
          continue
        key=couple[0]
        value=couple[1]
        node=XmlNode(key)
        node.setText(value)
        root.addChild(node)
    return root


  def getWheelInfo(self,root,prefix,ob):
    dimX=ob.dimensions[0]
    dimY=ob.dimensions[1]
    dimZ=ob.dimensions[2]
    if (dimX - dimZ) < -EPSILON or (dimX - dimZ) > EPSILON:
      log("Wheel %s %f,%f\n"%(prefix,dimX,dimZ));
      RaiseError("Wheel is not 'squared'")

    radius=dimX / 2.;
    name=prefix + "radius"
    node=XmlNode(name)
    node.setText("%f"%radius)
    root.addChild(node)

    width=dimY;
    name=prefix + "width"
    node=XmlNode(name)
    node.setText("%f"%width)
    root.addChild(node)

    name=prefix + "position"
    node=XmlNode(name)
    node.setText("%f,%f,%f"%(ob.location[0], ob.location[2], ob.location[1]))
    node.setProp("X",ob.location[0])
    node.setProp("Y",ob.location[2])
    node.setProp("Z",ob.location[1])
    root.addChild(node)
  
  def invoke(self, context, event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

  def execute(self, context):
    global inserted_images
    filepath=self.properties.filepath
    log("filepath: %s"%filepath)
    tmpdir = "/tmp/tmp"

    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        pass
      else:
        RaiseError("horrorororor!")

    xmlname=tmpdir+"/"+"VEHICLE"

    root=XmlNode("vehicle")

    infos=self.extractTextInfo()
    root.addChild(infos)

    materialDict, mesh_objects = createMaterialDictAndMeshList2(context.scene,False)

    chassisNodes=[ ]
    wheels=[ None, None, None, None ]

    self.chassis_bbound.reset()

    for ob, blender_mesh in mesh_objects:
      if ob.type=="MESH":
        if ob.name == 'wheel.fr': 
          wheels[WIDX_FRONT_RIGHT]=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          wheels[WIDX_FRONT_RIGHT].forceType("wfr")
          wheels[WIDX_FRONT_RIGHT].setProp("name",ob.name)
          root.addChild(wheels[WIDX_FRONT_RIGHT])
          self.getWheelInfo(root,"wfr_",ob)

        elif ob.name == 'wheel.fl': 
          wheels[WIDX_FRONT_LEFT]=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          wheels[WIDX_FRONT_LEFT].forceType("wfl")
          wheels[WIDX_FRONT_LEFT].setProp("name",ob.name)
          root.addChild(wheels[WIDX_FRONT_LEFT])
          self.getWheelInfo(root,"wfl_",ob)

        elif ob.name == 'wheel.rr': 
          wheels[WIDX_REAR_RIGHT]=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          wheels[WIDX_REAR_RIGHT].forceType("wrr")
          wheels[WIDX_REAR_RIGHT].setProp("name",ob.name)
          root.addChild(wheels[WIDX_REAR_RIGHT])
          self.getWheelInfo(root,"wrr_",ob)

        elif ob.name == 'wheel.rl': 
          wheels[WIDX_REAR_LEFT]=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          wheels[WIDX_REAR_LEFT].forceType("wrl")
          wheels[WIDX_REAR_LEFT].setProp("name",ob.name)
          root.addChild(wheels[WIDX_REAR_LEFT])
          self.getWheelInfo(root,"wrl_",ob)
        
        else:
          if ob.name.startswith("ignore."):
            log("Ignoring: %s"%ob.name)
            continue

          if self.properties.exportEXP and not ob.name.startswith("exp."):
            log("Ignoring: %s"%ob.name)
            continue

          log("exporting: '%s'"%ob.name)

          node=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          node.forceType("chassis")
          chassisNodes.append(node)
          root.addChild(node)
          log("updating from mesh '%s'"%ob.name)
          bb=ob.bound_box
          self.chassis_bbound.updateFromAABB(bb)
          for v in bb:
            self.chassis_bbound.update(v)


    offs=self.chassis_bbound.getOffsets()
    self.chassis_bbound.logDim()
    log("Applying offset: %f,%f,%f"%(offs[0],offs[1],offs[2]))
    for n in chassisNodes:
      n.applyTranslationToVertices(offs)
      n.setExportXmlMesh(self.properties.exportXML)
      n.export()

   
    offs=self.chassis_bbound.getCrisOffsets()
    if True:
      for node_name in [ "wrl_position", "wrr_position", "wfl_position", "wfr_position" ]:
        node=root.getChild(node_name)
        log("1- Position: %f,%f,%f"%
          (float(node.getProp("X")),
          float(node.getProp("Y")),
          float(node.getProp("Z"))))
        X=float(node.getProp("X"))+offs[0]
        Y=float(node.getProp("Y"))+offs[1]
        Z=float(node.getProp("Z"))+offs[2]
        node.setProp("X",X)
        node.setProp("Y",Y)
        node.setProp("Z",Z)
        log("2- Position: %f,%f,%f"%(
          float(node.getProp("X")),
          float(node.getProp("Y")),
          float(node.getProp("Z"))))
        node.setText("%f,%f,%f"%(X,Y,Z))


    for n in wheels:
      if n != None:
        n.setExportXmlMesh(self.properties.exportXML)
        n.export()
    
    xmlname=tmpdir + "/VEHICLE"
    f=open(xmlname,"w")
    root.writeSubTree(f,True)
    f.close()

    createZipFile(root,xmlname,filepath,tmpdir)
    
    return {'FINISHED'}

class export_OT_track(bpy.types.Operator):
  
  bl_idname = "io_export_scene.crisalide_track_exporter"
  bl_label = "Export crisalide file formats"
  
  filename_ext = ".zip"
  filter_glob = StringProperty(default="*.zip", options={'HIDDEN'})

  filepath = bpy.props.StringProperty(
    name="File Path", 
    description="File path used for exporting the crisalide file", 
    maxlen= 1024, default= "")


  exportXML = bpy.props.BoolProperty(
    name="Export xml", 
    description="Export also xml files of the meshes (debug purposes)",
    default=EXPORT_XML_MESH)

  exportEXP = bpy.props.BoolProperty(
    name="Export exp", 
    description="Export only object with name starting with 'exp.' (debug purposes",
    default=False)

  def extractTextInfo(self,tmpdir,root):
# 0: means none
# 1: means box
# 2: means sphere
    skydomeType=0  
    skydomeTextures=[ "", "", "", "", "", "" ]
    skydomeRe=re.compile("skydome(.+)")
    for textFile in bpy.data.texts:
      if textFile.name != "data":
        continue
      for line in textFile.lines:
        body=line.body
        couple=re.split('=',body)
        if len(couple) != 2:
          continue
        key=couple[0]
        value=couple[1]
        if key == "skydomeType":
          if value == "box" or value == "cube":
            skydomeType=1
          elif value == "sphere":
            skydomeType=2
          else:
            RaiseError("Unsupported skydome type")
        elif key.startswith("skydome"):
          m=skydomeRe.match(key)
          if m and len(m.groups()) == 1:
            n=int(m.group(1))-1
            skydomeTextures[n]=value
        else:
          node=XmlNode(key)
          node.setText(value)
          root.addChild(node)

    if skydomeType == 1:
      log("Skydome box")
      skydomeNode = XmlNode("skydome")
      skydomeNode.setProp("type","box")
      for i in range(0,6):
        tNode=XmlNode("texture")
        tNode.setProp("idx","%d"%(i+1))
        src=skydomeTextures[i]
        tNode.setProp("src",src)
        absSrc=buildPath(src)
        copy_image(absSrc,tmpdir)

        skydomeNode.addChild(tNode)
      root.addChild(skydomeNode)
    elif skydomeType == 2:
      log("Skydome sphere")
      skydomeNode = XmlNode("skydome")
      skydomeNode.setProp("type","sphere")

      tNode=XmlNode("texture")
      src=skydomeTextures[0]
      tNode.setProp("src",src)
      skydomeNode.addChild(tNode)
      absSrc=buildPath(src)
      copy_image(absSrc,tmpdir)
      root.addChild(skydomeNode)

    return root

  def handleTrackProfile(self, obj):
    if obj.type != 'CURVE':
      return


    ctrlPoints=XmlNode("control-points")
    
    # TODO: should do more 'type' checking on 
    #       type of spline !!!
    #       may should also check the number
    #       of spline
    spline=obj.data.splines[0]
    for bpoint  in spline.bezier_points:
      co = bpoint.co
      node=XmlNode("point")
      node.setProp("X",co[0])
      node.setProp("Y",co[2])
      node.setProp("Z",co[1])
      ctrlPoints.addChild(node)

    return ctrlPoints
    

  def execute(self, context):
    global inserted_images
    filepath=self.properties.filepath
    log("filepath: %s"%filepath)
    tmpdir = "/tmp/tmp"

    #########################
    #xmlname=tmpdir+"/"+"track.xml"
    xmlname=tmpdir+"/"+"TRACK"

    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        pass
      else:
        RaiseError("horrorororor!")

    root=XmlNode("track")
    triggers=XmlNode("triggers")
    root.addChild(triggers)
    scene = context.scene
    ctrlPoint=None

    self.extractTextInfo(tmpdir,root)
    
    for ob in [ ob for ob in scene.objects if ob.is_visible(scene) ]:
      if ob.name.startswith("trig."):
        triggerType = ob.name.replace("trig.","")
        pos=ob.location
        dim=ob.dimensions
        quat=ob.rotation_quaternion
        node=XmlNode("trigger")
        node.setProp("type",triggerType)
        node.setProp("pos","%f,%f,%f"%(pos[0],pos[2],pos[1]))
        node.setProp("rot","%f,%f,%f,%f"%(quat[0],quat[2],quat[1],quat[3]))
        node.setProp("halfDim","%f,%f,%f"%(dim[0]/2., dim[2]/2., dim[1]/2.))
        triggers.addChild(node)
        continue

      if ob.type == "CURVE" and ob.name == "track.profile":
        ctrlPoints=self.handleTrackProfile(ob)
        continue

      materialDict, mesh_objects = createMaterialDictAndMeshList(scene,ob)

      for ob, blender_mesh in mesh_objects:
        if ob.type=="MESH":
          if ob.name.startswith("ignore."):
            log("Ignoring: %s"%ob.name)
            continue

          if self.properties.exportEXP and not ob.name.startswith("exp."):
            log("Ignoring: %s"%ob.name)
            continue

          if len(materialDict) == 0:
            log("Ignoring: '%s' (zero materials!)"%ob.name)
            continue

          log("Exporting: '%s', material #: %d"%(ob.name,len(materialDict)))

          node=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          node.setExportXmlMesh(self.properties.exportXML)
          node.setProp("name",ob.name)
          root.addChild(node)

    # gather track information #
    for ob in bpy.data.objects:
      if ob.type == "EMPTY" and ob.name == "track.start":
        val="%f,%f,%f"%( ob.location[0], ob.location[2], ob.location[1])
        node=XmlNode("track_start_pos")
        node.setText(val)
        root.addChild(node)

        val="%f"%(-1.*ob.rotation_euler[2])
        node=XmlNode("track_start_rot")
        node.setText(val)
        root.addChild(node)

    if ctrlPoints != None:
      root.addChild(ctrlPoints)

    file=open(xmlname,"w")
    root.writeSubTree(file)
    file.close()

    root.export()

    inserted_images={ }
    createZipFile(root,xmlname,filepath,tmpdir)

    #root.cleanup()
    #os.remove(xmlname)

    return {'FINISHED'}

  def invoke(self, context, event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

class export_OT_roombox(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_roombox_exporter"
  bl_label = "Export crisalide file formats"
  
  filename_ext = ".zip"
  filter_glob = StringProperty(default="*.zip", options={'HIDDEN'})

  filepath = bpy.props.StringProperty(
    name="File Path", 
    description="File path used for exporting the crisalide file", 
    maxlen= 1024, default= "")

  exportXML = bpy.props.BoolProperty(
    name="Export xml", 
    description="Export also xml files of the meshes (debug purposes)",
    default=EXPORT_XML_MESH)

  exportEXP = bpy.props.BoolProperty(
    name="Export exp", 
    description="Export only object with name starting with 'exp.' (debug purposes",
    default=False)

  def invoke(self, context, event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

  def execute(self, context):
    tmpdir = "/tmp/tmp"

    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        pass
      else:
        RaiseError("horrorororor!")

    xmlname=tmpdir+"/"+"ROOMBOX"
    root=XmlNode("roombox")

    scene = context.scene

    for ob in [ ob for ob in scene.objects if ob.is_visible(scene) ]:
      materialDict, mesh_objects = createMaterialDictAndMeshList(scene,ob)

      for ob, blender_mesh in mesh_objects:
        if ob.type=="MESH":
          if ob.name.startswith("ignore."):
            log("Ignoring: %s"%ob.name)
            continue

          if self.properties.exportEXP and not ob.name.startswith("exp."):
            log("Ignoring: %s"%ob.name)
            continue

          if len(materialDict) == 0:
            log("Ignoring: '%s' (zero materials!)"%ob.name)
            continue

          log("Exporting: '%s', material #: %d"%(ob.name,len(materialDict)))

          node=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
          node.setExportXmlMesh(self.properties.exportXML)
          node.setProp("name",ob.name)
          root.addChild(node)

    root.export()

    file=open(xmlname,"w")
    root.writeSubTree(file)
    file.close()

    filepath=self.properties.filepath
    createZipFile(root,xmlname,filepath,tmpdir)
    return {'FINISHED'}


def menu_func_track_export(self, context):
    self.layout.operator(export_OT_track.bl_idname, text="Crisalide track exporter (*.zip)")

def menu_func_vehicle_export(self, context):
    self.layout.operator(export_OT_vehicle.bl_idname, text="Crisalide vehicle exporter (*.zip)")

def menu_func_roombox_export(self, context):
    self.layout.operator(export_OT_roombox.bl_idname, text="Crisalide room box export (*zip)") 

def register():
    bpy.types.INFO_MT_file_export.append(menu_func_track_export)
    bpy.types.INFO_MT_file_export.append(menu_func_vehicle_export)
    bpy.types.INFO_MT_file_export.append(menu_func_roombox_export)

def unregister():
    bpy.types.INFO_MT_file_export.remove(menu_func_track_export)
    bpy.types.INFO_MT_file_export.remove(menu_func_vehicle_export)
    bpy.types.INFO_MT_file_export.remove(menu_func_roombox_export)

if __name__ == "__main__":
  log("*** crisalide exporter ***\n")
  register()



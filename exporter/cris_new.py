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
import bpy,os,struct,zipfile,math,time,mathutils,shutil

from bpy.props import *
from io_utils import ExportHelper, ImportHelper
from io_utils import create_derived_objects, free_derived_objects

"""
cris binary file format

- MARK_VERTICES, n_vertices, ...<vertices>...

- MARK_MATERIALS

- MARK_USE_MATERIAL, material_name

- MARK_FACES_ONLY 
"""

# LOG configuration
LOG_ON_STDOUT=0
LOG_ON_FILE=1
LOG_FILENAME="/tmp/log.txt"
EPSILON=0.0001

#########################################

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

class BBound:
  def __init__(self):
    self.minX = 0.
    self.maxX = 0.
    self.minY = 0.
    self.maxY = 0.
    self.minZ = 0.
    self.maxZ = 0.

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

  def getOffsets(self):
    point=[ 0., 0., 0. ]
    point[0]=-(self.minX + self.maxX) / 2.0;
    point[1]=-(self.minY + self.maxY) / 2.0;
    point[2]=-(self.minZ + self.maxZ) / 2.0;
    return point


def copy_image(image,dest_dir):
  fn = bpy.path.abspath(image.filepath)
  fn = os.path.normpath(fn)
  fn_strip = os.path.basename(fn)

  rel = fn_strip
  fn_abs_dest = os.path.join(dest_dir, fn_strip)
  if not os.path.exists(fn_abs_dest):
    log("Copying '%s' to '%s'\n"%(fn, fn_abs_dest))
    shutil.copy(fn, fn_abs_dest)
    log("done\n");

  return fn_abs_dest
    
      

def fixName(name):
  if name is None:
    return 'None'
  else:
    return name.replace(' ', '_')

def log(str):
  if LOG_ON_STDOUT==1:
    print(str,end="")
  if LOG_ON_FILE==1:
    f=open(LOG_FILENAME,"a")
    f.write(str)
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
    v=struct.pack("ffffffff" , data.uv1[0], data.uv1[1],\
      data.uv2[0], data.uv2[1],\
      data.uv3[0], data.uv3[1],\
      data.uv4[0], data.uv4[1])
  else:
    v=struct.pack("ffffff" , data.uv1[0], data.uv1[1],\
      data.uv2[0], data.uv2[1],\
      data.uv3[0], data.uv3[1])
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
    self.x_text=""

  def setProp(self,pname,pvalue):
    self.x_props[pname]=pvalue 

  def getProp(self,pname):
    if pname in self.x_prop:
      return self.x_prop[pname]
    return None

  def addChild(self,child):
    if child == None:
      ErrorError()
    self.x_children.append(child)

  def setText(self,text):
    self.x_text=text

  def export(self):
    pass

  def cleanup(self):
    for child in self.children:
      child.cleanup()

  def getRecurse(self):
    return True

  def writeSubTree(self,outFile):
    outFile.write("<%s"%(self.x_name))
    if len(self.x_props.keys()):
      for prop in self.x_props:
        outFile.write(" %s=\"%s\""%(prop,self.x_props[prop]))
    
    if (self.getRecurse() and len(self.x_children)) or self.x_text!="":
      outFile.write(">");
      if len(self.x_children) > 0:
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
    ofilename=dest_dir+"/"+ob.name+".mesh"

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

    for mat_and_image in materials.values():
      node=createMaterialNode(mat_and_image[0], mat_and_image[1])
      self.addChild(node)

    for v in vert_array:
      node=XmlNode("vertex")
      node.setText("%f,%f,%f"%(v[0],v[1],v[2]))
      self.addChild(node)

    if uv_array:
      for uvc in uv_array:
        node=XmlNode("uvcord")
        node.setText("%f,%f"%(uvc[0],uvc[1]))
        self.addChild(node)
  
  def getRecurse(self):
    return False
 
  def cleanup(self):
    XmlNode.cleanup()



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

def make_faces_chunk(tri_list, mesh, materialDict):

    materials = mesh.materials
    if not materials:
        mat = None

    face_list = [ ]

    if len(mesh.uv_textures):
        unique_mats = {}
        for i,tri in enumerate(tri_list):
            face_list.append(tri.vertex_index)

            if materials:
                mat = materials[tri.mat]
                if mat: mat = mat.name

            img = tri.image

            try:
                context_mat_face_array = unique_mats[mat, img][1]
            except:
                if mat:	name_str = mat
                else:	name_str = 'None'
                if img: name_str += img

                context_mat_face_array = [ ]
                unique_mats[mat, img] = _3ds_string(sane_name(name_str)), context_mat_face_array


            context_mat_face_array.add(_3ds_short(i))
            # obj_material_faces[tri.mat].add(_3ds_short(i))

        face_chunk.add_variable("faces", face_list)
        for mat_name, mat_faces in unique_mats.values():
            obj_material_chunk=_3ds_chunk(OBJECT_MATERIAL)
            obj_material_chunk.add_variable("name", mat_name)
            obj_material_chunk.add_variable("face_list", mat_faces)
            face_chunk.add_subchunk(obj_material_chunk)

    else:

        obj_material_faces=[]
        obj_material_names=[]
        for m in materials:
            if m:
                obj_material_names.append(_3ds_string(sane_name(m.name)))
                obj_material_faces.append(_3ds_array())
        n_materials = len(obj_material_names)

        for i,tri in enumerate(tri_list):
            face_list.add(_3ds_face(tri.vertex_index))
            if (tri.mat < n_materials):
                obj_material_faces[tri.mat].add(_3ds_short(i))

        face_chunk.add_variable("faces", face_list)
        for i in range(n_materials):
            obj_material_chunk=_3ds_chunk(OBJECT_MATERIAL)
            obj_material_chunk.add_variable("name", obj_material_names[i])
            obj_material_chunk.add_variable("face_list", obj_material_faces[i])
            face_chunk.add_subchunk(obj_material_chunk)

    return face_chunk


def createColorNode(name,color):
  node=XmlNode(name)
  node.setText("%f,%f,%f"%(color[0],color[1],color[2]))
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
      node.setText(image.filepath)
      matNode.addChild(node)

  return matNode
  

def createMeshNode(ob,mesh,materials,dest_dir):
  ofilename=dest_dir+"/"+ob.name+".mesh"

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


  root=XmlNode("mesh")

  for mat_and_image in materials.values():
    node=createMaterialNode(mat_and_image[0], mat_and_image[1])
    root.addChild(node)

  for v in vert_array:
    node=XmlNode("vertex")
    node.setText("%f,%f,%f"%(v[0],v[1],v[2]))
    root.addChild(node)

  if uv_array:
    for uvc in uv_array:
      node=XmlNode("uvcord")
      node.setText("%f,%f"%(uvc[0],uvc[1]))
      root.addChild(node)

  return root


class export_OT_track(bpy.types.Operator):
  
  bl_idname = "io_export_scene.crisalide_track_exporter"
  bl_label = "Export crisalide file formats"
  
  filename_ext = ".zip"
  filter_glob = StringProperty(default="*.zip", options={'HIDDEN'})

  def execute(self, context):
    tmpdir = "/tmp/tmp"

    #########################
    xmlname=tmpdir+"/"+"track.xml"

    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        pass
      else:
        RaiseError("horrorororor!")

    root=XmlNode("track")
    scene = context.scene

    materialDict = {}
    mesh_objects = []
    scene = context.scene
    for ob in [ob for ob in scene.objects if ob.is_visible(scene)]:

      free, derived = create_derived_objects(scene, ob)

      if derived is None:
        continue
    
      for ob_derived, mat in derived:
        if ob.type not in ('MESH', 'CURVE', 'SURFACE', 'FONT', 'META'):
          continue

        data = ob_derived.create_mesh(scene, True, 'PREVIEW')
        if data:
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

    for ob, blender_mesh in mesh_objects:
#node=createMeshNode(ob,blender_mesh,materialDict,tmpdir)
      node=XmlMeshNode(ob,blender_mesh,materialDict,tmpdir)
      node.setProp("name",ob.name)
      root.addChild(node)

    file=open(xmlname,"w")
    root.writeSubTree(file)
    file.close()

    root.cleanup()

    return {'FINISHED'}

#def invoke(self, context, event):
#context.window_manager.fileselect_add(self)
#return {'RUNNING_MODAL'}

def menu_func_track_export(self, context):
    self.layout.operator(export_OT_track.bl_idname, text="Crisalide track exporter (*.zip)")


def register():
    bpy.types.INFO_MT_file_export.append(menu_func_track_export)

def unregister():
    bpy.types.INFO_MT_file_export.remove(menu_func_track_export)

if __name__ == "__main__":
  log("*** crisalide exporter ***\n")
  register()



#!BPY
"""
Name: 'Crisalide track exporter (.cris,)...'
Blender: 253
Group: 'Export'
Tooltip: 'Exports meshes to Irrlicht mesh & scene files'
"""
__author__ = ['Gianni Masullo']
__version__ = '0.4'
__url__ = [ ]
__bpydoc__ = """\
Crisalide xml exporter

"""
import bpy,os,struct,zipfile,math


# LOG configuration
LOG_ON_STDOUT=1
LOG_ON_FILE=0
LOG_FILENAME="/tmp/log.txt"

# EXPORT configuration
EXP_APPLY_OBJ_TRANSFORM=1

#########################################

# markers 
MARK_VERTICES=0xf100
MARK_FACES_ONLY=0xf101
MARK_MATERIAL=0xf102
MARK_USE_MATERIAL=0xf103
MARK_UVCOORD=0xf104

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
  #TODO: use a transform matrix
  #v=struct.pack("ddd",point[0],point[2],-point[1])
  v=struct.pack("ddd",point[0],point[2],point[1])
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


def getMaterialFlagsWord(mat):
  # TODO: get "custom property" from mat to
  #       set flags: uptonow they are set to default
  word=0
  for v in MATERIA_FLAGS_LIST:
    if v["default"]:
      word = word | v["value"]
  return word

class export_OT_track(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_exporter"
  bl_description = 'Export to track crisalide file format (.zip)'
  bl_label = "Export crisalide file formats"
  bl_space_type = "PROPERTIES"
  bl_region_type = "WINDOW"

  filepath = bpy.props.StringProperty(
    name="File Path", 
    description="File path used for exporting the crisalide file", 
    maxlen= 1024, default= "")

  rot90 = bpy.props.BoolProperty(
    name = "Rotate 90 degrees",
    description="Rotate mesh to Y up",
    default = True)

  scale = bpy.props.FloatProperty(
    name = "Scale", 
    description="Scale mesh", 
    default = 0.1, min = 0.001, max = 1000.0)

  def createObjTransformMatrix(self,object):
    Mx = []
    My = []
    Mz = []
    T1 = []
    transform = []

    angle = object.rotation_euler[0]
    Mx.append([1, 0, 0])
    y = math.cos(angle)
    z = -math.sin(angle)
    Mx.append([0, y, z])
    y = math.sin(angle)
    z = math.cos(angle)
    Mx.append([0, y, z])

    angle = object.rotation_euler[1]
    x = math.cos(angle)
    z = math.sin(angle)
    My.append([x, 0, z])
    My.append([0, 1, 0])
    x = -math.sin(angle)
    z = math.cos(angle)
    My.append([x, 0, z])

    angle = object.rotation_euler[2]
    x = math.cos(angle)
    y = -math.sin(angle)
    Mz.append([x, y, 0])
    x = math.sin(angle)
    y = math.cos(angle)
    Mz.append([x, y, 0])
    Mz.append([0, 0, 1])

    m0 = Mx[0]
    m1 = Mx[1]
    m2 = Mx[2]
    for row in My:
      x, y, z = row
      nx = x*m0[0] + y*m1[0] + z*m2[0]
      ny = x*m0[1] + y*m1[1] + z*m2[1]
      nz = x*m0[2] + y*m1[2] + z*m2[2]
      T1.append([nx, ny, nz])

    m0 = T1[0]
    m1 = T1[1]
    m2 = T1[2]
    for row in Mz:
      x, y, z = row
      nx = x*m0[0] + y*m1[0] + z*m2[0]
      ny = x*m0[1] + y*m1[1] + z*m2[1]
      nz = x*m0[2] + y*m1[2] + z*m2[2]
      transform.append([nx, ny, nz])

    transform.append([object.scale[0], object.scale[1], object.scale[2]])
    transform.append([object.location[0], object.location[1], object.location[2]])
  
    return transform

  def applyTransform(self,v,matrix):
    nv=[0., 0., 0.]
    for c in range(0,3):
      for r in range(0,3):
        nv[c] = nv[c] + v[r] * matrix[r][c]
      nv[c] = nv[c] + matrix[3][c]
    return nv

  def applyTransformOld(self,vertex,matrix):
    v2 = []
    nv = []

    x, y, z = vertex
    sx, sy, sz = matrix[3]
    lx, ly, lz = matrix[4]

    v2.append(x*sx)
    v2.append(y*sy)
    v2.append(z*sz)

    for index in range(len(vertex)):
      t = matrix[index]
      nv.append(v2[0]*t[0] + v2[1]*t[1] +v2[2]*t[2])

    nv[0] = nv[0]+lx
    nv[1] = nv[1]+ly
    nv[2] = nv[2]+lz

    return nv

  def exportMesh3(self, fp, ob):
    materials=ob.data.materials
    faces=ob.data.faces
    vertices=ob.data.vertices

    if len(materials)==0:
      log("Discarding object '%s' coz have no material set\n"%ob.name);
      return False

    #transform=self.createObjTransformMatrix(ob)
    transform=ob.matrix_world

    # export vertices
    binWrite_mark(fp,MARK_VERTICES)
    binWrite_int(fp,len(vertices))
    log("vertices: %d\n"%len(vertices))
    for v in vertices:
      if EXP_APPLY_OBJ_TRANSFORM == 1:
        co=self.applyTransform(v.co,transform)
      else:
        co=v.co
      binWrite_pointVect(fp,co)

    used_material={}
    for face in faces:
      name=materials[face.material_index].name
      if not name in used_material:
        used_material[name]=face.material_index

    for key in used_material:
      idx=used_material[key]
      self.exportMaterial(fp,materials[idx])

    # export faces
    for mat_idx in range(len(materials)):
      n_faces=0
      for face in faces:
        if face.material_index!=mat_idx:
          continue
        n_faces=n_faces+1
      log("material: '%s'\n"%materials[mat_idx].name)
      binWrite_mark(fp,MARK_USE_MATERIAL)
      binWrite_string(fp,materials[mat_idx].name)
      binWrite_mark(fp,MARK_FACES_ONLY)
      binWrite_int(fp,n_faces)
      for face in faces:
        if face.material_index!=mat_idx:
          continue
        #log("face:")
        binWrite_int(fp,len(face.vertices))
        for v_idx in face.vertices:
          #log("%d "%v_idx)
          binWrite_int(fp,v_idx)
          #log("\n")
      log("done with material '%s'\n"%materials[mat_idx].name);
    return True

  def exportMesh2(self, fp, ob):
    materials=ob.data.materials
    faces=ob.data.faces
    vertices=ob.data.vertices
    log("exporting '%s'\n"%ob.name)

    vert_flag=[ ]
    for i in range(len(vertices)):
      vert_flag.append(-1)

    for mat_idx in range(len(materials)):
      tot_vert=0
      tot_faces=0
      new_verts=[ ]

      binWrite_mark(fp,MARK_USE_MATERIAL)
      binWrite_string(fp,materials[mat_idx].name)

      for face in faces:
        if face.material_index!=mat_idx:
          continue
        new_face=[ ]
        for v_idx in face.vertices:
          if vert_flag[v_idx] == -1:
            new_verts.append(v_idx)
            vert_flag[v_idx]=tot_vert
            tot_vert=tot_vert+1
        tot_faces=tot_faces+1 

      binWrite_mark(fp,MARK_VERTICES)
      binWrite_int(fp,len(new_verts))
      for v_idx in new_verts:
        v=vertices[v_idx]
        binWrite_pointVect(fp,v.co)
        log("vertex: %f,%f,%f\n"%(v.co[0],v.co[1],v.co[2]))

      binWrite_mark(fp,MARK_FACES_ONLY)
      binWrite_int(fp,tot_faces)
      for face in faces:
        if face.material_index!=mat_idx:
          continue
        log("face:")
        binWrite_int(fp,len(face.vertices))
        for v_idx in face.vertices:
          log("%d "%vert_flag[v_idx])
          binWrite_int(fp,vert_flag[v_idx])
        log("\n")

  def exportMesh(self, fp, ob):

    if not ob or ob.type != 'MESH':
        raise NameError('Cannot export: active object %s is not a mesh.' % ob)

    me = ob.data

    # export vertices
    print("Vertices: %d"%len(me.vertices))
    binWrite_mark(fp,MARK_VERTICES)
    binWrite_int(fp,len(me.vertices))
    for v in me.vertices:
      x=v.co
      binWrite_pointVect(fp,x)

    # export faces
    if len(me.uv_textures) > 0:
      uvtex = me.uv_textures[0]
      for f in me.faces:
        data = uvtex.data[f.index]
        binWrite_textureCoords(fp,len(f.vertices),data)

      vt = 1
      for f in me.faces:
        vs = f.vertices
        binWrite_faceWithTexture(fp,vs,vt)
        vt += 3
        if len(f.vertices) == 4:
          vt += 1        

    else:
      binWrite_mark(fp,MARK_FACES_ONLY)
      binWrite_int(fp,len(me.faces))
      for f in me.faces:
        vs = f.vertices
        binWrite_face(fp,vs)

  def exportMaterial(self,fp,ma):
    alpha=ma.alpha
    specular=ma.specular_color
    diffuse=ma.diffuse_color
    word=getMaterialFlagsWord(ma)
    Ka=[ alpha, alpha, alpha ]

    log("Exporting material: '%s' (flags: %u)\n"%(ma.name,word))

    binWrite_mark(fp,MARK_MATERIAL)

    binWrite_string(fp,ma.name)
    binWrite_u32(fp,word)
    binWrite_color(fp,diffuse)
    binWrite_color(fp,diffuse)
    binWrite_color(fp,specular)

  def exportCameras(self,tmp_dir_name):
    objects=bpy.data.objects
    names=[]
    for camera in objects:
      if camera.type != "CAMERA":
        continue
      log("Exporting camera: '%s'\n"%camera.name)
      filename=tmp_dir_name+"/"+camera.name+".camera"
      loc=camera.location
      rot=camera.rotation_euler
      log("camera rotation: %f,%f,%f\n"%(rot[0],rot[1],rot[2]))
      fp = open(filename,"wb")
      binWrite_pointVect(fp,loc)
      binWrite_pointVect(fp,rot)
      fp.close()
      names.append([ 'camera', filename ])
    return names

  def exportLamp(self,fp,ob):
    lamp=ob.data
    if lamp.type == "POINT":
      log("exporting lamp '%s', type: '%s'\n"%\
        (ob.name,lamp.type))
      binWrite_mark(fp,LAMP_TYPE_POINT)
      binWrite_pointVect(fp,ob.location)
      binWrite_pointVect(fp,lamp.color)
      binWrite_pointVect(fp,lamp.color)
      binWrite_float(fp,lamp.energy)
    else:
      log("skipping export lamp '%s', type: '%s'\n"%\
        (ob.name,lamp.type))

  def exportObject(self,tmp_dir_name,obj):
    if obj.type == 'MESH':
      filename=tmp_dir_name+"/"+obj.name+".mesh"
      fp = open(filename, 'wb')    
      ret=self.exportMesh3(fp,obj)
      fp.close()
      if ret:
        return [ 'mesh' , filename ]
      else:
        return None
    if obj.type == "LAMP":
      filename=tmp_dir_name+"/"+obj.name+".lamp"
      fp = open(filename, 'wb')    
      self.exportLamp(fp,obj)
      fp.close()
      return [ 'lamp', filename ]
    return None

  def execute(self, context):
    filepath=self.properties.filepath
    realpath = os.path.realpath(os.path.expanduser(filepath))
    name = os.path.basename(filepath)
    dirname = os.path.dirname(filepath)
    tmpdir = dirname + "/tmp";

    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        log("presente directory\n")
      else:
        RaiseError("herrorororor!")
    log("Exporing track (dir %s): %s\n"%(dirname,filepath))
    elements=[]

    for ob in bpy.data.objects:
      name=self.exportObject(tmpdir,ob)
      if name != None:
        log("%s,%s\n"%(name[0],name[1]))
        elements.append(name)

    ns=self.exportCameras(tmpdir)
    for n in ns:
      elements.append(n)

    #### create xml file
    filename=tmpdir + "/TRACK";
    fp=open(filename,"w")
    fp.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<track>\n")
    for name in elements:
      p=os.path.basename(name[1])
      fp.write("  <%s>%s</%s>\n"%(name[0],p,name[0]))
    fp.write("</track>")
    fp.close()

    #### create zip file
    zf=zipfile.ZipFile(realpath,"w")
    for name in elements:
      p=os.path.basename(name[1])
      zf.write(name[1],p)
    zf.write(filename,"TRACK")
    zf.close()

    #### remove temp files
    os.remove(filename)
    for name in elements:
      fname=name[1]
      os.remove(fname)
      log("removing '%s'\n"%fname)
    os.removedirs(tmpdir)
    log("removing '%s'\n"%tmpdir)

    return {'FINISHED'}

  def invoke(self, context, event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

class export_OT_vehicle(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_exporter"
  bl_description = 'Export vehicle to crisalide file format (.track)'
  bl_label = "Export crisalide file formats"
  bl_space_type = "PROPERTIES"
  bl_region_type = "WINDOW"

def menu_func_track_export(self, context):
    self.layout.operator(export_OT_track.bl_idname, text="Crisalide track exporter")

def menu_func_vehicle_export(self, context):
    self.layout.operator(export_OT_vehicle.bl_idname, text="Crisalide vehicle exporter")

def register():
    bpy.types.INFO_MT_file_export.append(menu_func_track_export)
    bpy.types.INFO_MT_file_export.append(menu_func_vehicle_export)

def unregister():
    bpy.types.INFO_MT_file_export.remove(menu_func_track_export)
    bpy.types.INFO_MT_file_export.remove(menu_func_vehicle_export)

if __name__ == "__main__":
  print("*** crisalide exporter ***")
  register()



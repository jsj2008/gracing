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
import bpy,os,struct,zipfile,math


# LOG configuration
LOG_ON_STDOUT=0
LOG_ON_FILE=1
LOG_FILENAME="/tmp/log.txt"

# EXPORT configuration
EXP_APPLY_OBJ_TRANSFORM=1

EPSILON=0.0001

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

WIDX_FRONT_LEFT=0
WIDX_FRONT_RIGHT=1
WIDX_REAR_LEFT=2
WIDX_REAR_RIGHT=3

WHEEL_PREFIX=[ 'wfl_', 'wfr_', 'wrl_', 'wrr_' ]

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

def applyTransform(v,matrix):
  nv=[0., 0., 0.]
  for c in range(0,3):
    for r in range(0,3):
      nv[c] = nv[c] + v[r] * matrix[r][c]
    nv[c] = nv[c] + matrix[3][c]
  return nv

def exportMesh(fp, ob):
  materials=ob.data.materials
  faces=ob.data.faces
  vertices=ob.data.vertices

  if len(materials)==0:
    log("Discarding object '%s' coz have no material set\n"%ob.name);
    return False

  transform=ob.matrix_world

  # export vertices
  binWrite_mark(fp,MARK_VERTICES)
  binWrite_int(fp,len(vertices))
  log("vertices: %d\n"%len(vertices))
  for v in vertices:
    if EXP_APPLY_OBJ_TRANSFORM == 1:
      co=applyTransform(v.co,transform)
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
    exportMaterial(fp,materials[idx])

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

def exportMaterial(fp,ma):
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

def exportCameras(tmp_dir_name):
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

def exportLamp(fp,ob):
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

def exportObject(tmp_dir_name,obj):
  if obj.type == 'MESH':
    filename=tmp_dir_name+"/"+obj.name+".mesh"
    fp = open(filename, 'wb')    
    ret=exportMesh(fp,obj)
    fp.close()
    if ret:
      return [ 'mesh' , filename ]
    else:
      return None
  if obj.type == "LAMP":
    filename=tmp_dir_name+"/"+obj.name+".lamp"
    fp = open(filename, 'wb')    
    exportLamp(fp,obj)
    fp.close()
    return [ 'lamp', filename ]
  if obj.type == "EMTPY":
    return [ "pippo", "pluto", 1 ]

  return None

def exportXml(root_name,basedir,name,elements):
  filename=basedir + "/" + name;
  fp=open(filename,"w")
  fp.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<%s>\n"%root_name)
  for name in elements:
    log("'%s','%s',%d\n"%(name[0],name[1],len(name)))
    if len(name) == 2:
      p=os.path.basename(name[1])
    else:
      p=name[1]
    fp.write("  <%s>%s</%s>\n"%(name[0],p,name[0]))
  fp.write("</%s>"%root_name)
  fp.close()

def createZip(path,index_file,index_name,elements):
    zf=zipfile.ZipFile(path,"w")
    for name in elements:
      p=os.path.basename(name[1])
      zf.write(name[1],p)
    zf.write(index_file,index_name)
    zf.close()

class export_OT_track(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_track_exporter"
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

  def getTrackInfo(self, elements):
    pass

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
      name=exportObject(tmpdir,ob)
      if name != None:
        log("%s,%s\n"%(name[0],name[1]))
        elements.append(name)

    ns=exportCameras(tmpdir)
    for n in ns:
      elements.append(n)

    xmlElements=[ ]

    for e in elements:
      xmlElements.append(e)

    #### retrieve track info
    self.getTrackInfo(xmlElements)

    #### create xml file
    exportXml("track",tmpdir,"TRACK",xmlElements)

    #### create zip file
    index_filename=tmpdir+"/TRACK"
    createZip(realpath,index_filename,"TRACK",elements)

    #### remove temp files
    os.remove(index_filename)
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
  bl_idname = "io_export_scene.crisalide_vehicle_exporter"
  bl_description = 'Export vehicle to crisalide file format (.zip)'
  bl_label = "Export crisalide file formats"
  bl_space_type = "PROPERTIES"
  bl_region_type = "WINDOW"

  wheel_radius=[ 1., 1., 1., 1.  ]
  wheel_width=[ 1., 1., 1., 1.  ]

  wheel_position=[\
    [ 1., 0., 0. ],\
    [ 1., 0., 0. ],\
    [ 1., 0., 0. ],\
    [ 1., 0., 0. ]\
  ]

  vehicle_parm_default=[
   [ "vehicleSteering", 0.0 ],\
   [ "steeringIncrement", 0.04 ],\
   [ "steeringClamp", 0.5 ],\
   [ "suspensionStiffness", 20.0 ],\
   [ "suspensionDamping", 2.3 ],\
   [ "suspensionCompression", 4.4 ],\
   [ "rollInfluence", 0.1 ],\
  ]
  
  filepath = bpy.props.StringProperty(
    name="File Path", 
    description="File path used for exporting the crisalide file", 
    maxlen= 1024, default= "")

  def updateWheelInfo(self, ob):

    if ob.name == "wheel.fr":
      idx=WIDX_FRONT_RIGHT
    elif ob.name == "wheel.fl":
      idx=WIDX_FRONT_LEFT
    elif ob.name == "wheel.rr":
      idx=WIDX_REAR_RIGHT
    elif ob.name == "wheel.rl":
      idx=WIDX_REAR_LEFT
    else:
      idx=-1

    if idx == -1:
      RaiseError("Internal incosistence")

    dimX=ob.dimensions[0]
    dimY=ob.dimensions[1]
    dimZ=ob.dimensions[2]

    if (dimX - dimZ) < -EPSILON or (dimX - dimZ) > EPSILON:
      log("Wheel %s %f,%f\n"%(WHEEL_PREFIX[idx],dimX,dimZ));
      RaiseError("Wheel is not 'squared'")

    self.wheel_radius[idx]=dimX / 2.;
    self.wheel_width[idx]=dimY / 2.;
    self.wheel_position[idx][0]=ob.location[0]
    self.wheel_position[idx][1]=ob.location[1]
    self.wheel_position[idx][2]=ob.location[2]


  def execute(self, context):
    log("executing\n");

#####################

#####################

    filepath=self.properties.filepath
    realpath = os.path.realpath(os.path.expanduser(filepath))
    name = os.path.basename(filepath)
    dirname = os.path.dirname(filepath)
    tmpdir = dirname + "/tmp";

    chassis_objs=[ ]
    wheel_fl_objs=[ ]
    wheel_fr_objs=[ ]
    wheel_rl_objs=[ ]
    wheel_rr_objs=[ ]
    for ob in bpy.data.objects:
      if ob.name == "wheel.fr":
        self.updateWheelInfo(ob)
        wheel_fr_objs.append(ob)
      elif ob.name == "wheel.fl":
        self.updateWheelInfo(ob)
        wheel_fl_objs.append(ob)
      elif ob.name == "wheel.rr":
        self.updateWheelInfo(ob)
        wheel_rr_objs.append(ob)
      elif ob.name == "wheel.rl":
        self.updateWheelInfo(ob)
        wheel_rl_objs.append(ob)
      elif ob.type == 'MESH':
        chassis_objs.append(ob)
      elif ob.type == "EMPTY" and ob.name == "VehicleData":
        log("getting vehicle data from '%s'\n"%ob.name)

    if len(wheel_rl_objs) == 0:
      RaiseError("Wheel rear left is missing")

    if len(wheel_rr_objs) == 0:
      RaiseError("Wheel rear right is missing")

    if len(wheel_fr_objs) == 0:
      RaiseError("Wheel front right is missing")

    if len(wheel_fl_objs) == 0:
      RaiseError("Wheel front left is missing")
    
    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        log("presente directory\n")
      else:
        RaiseError("herrorororor!")

    log("Exporing vehicle (dir %s): %s\n"%(dirname,filepath))
    elements=[ ]

    log("Exporting chassis\n")
    for ob in chassis_objs: 
      el=exportObject(tmpdir,ob)
      if el != None:
        el[0]="chassis"
        log("%s,%s\n"%(el[0],el[1]))
        elements.append(el)
      log("  -exporting obj '%s'\n"%ob.name)

    log("Exporting wheels\n");
    log("  - front left\n")
    for ob in wheel_fl_objs:
      el=exportObject(tmpdir,ob)
      if el != None: 
        el[0]="wfl";
        elements.append(el)

    log("  - front right\n")
    for ob in wheel_fr_objs:
      el=exportObject(tmpdir,ob)
      if el != None: 
        el[0]="wfr";
        elements.append(el)

    log("  - rear left\n")
    for ob in wheel_rl_objs:
      el=exportObject(tmpdir,ob)
      if el != None: 
        el[0]="wrl";
        elements.append(el)

    log("  - rear right\n")
    for ob in wheel_rr_objs:
      el=exportObject(tmpdir,ob)
      if el != None: 
        el[0]="wrr";
        elements.append(el)

    xmlElements=[ ]
    for e in elements:
      xmlElements.append(e)

    #### add wheels data to xml elements
    for i in range(0,4):
      name=WHEEL_PREFIX[i] + "radius"
      value=self.wheel_radius[i]
      xmlElements.append([ name, value, 1])
      name=WHEEL_PREFIX[i] + "width"
      value=self.wheel_width[i]
      xmlElements.append([ name, value, 1])
      name=WHEEL_PREFIX[i] + "position"
      value="%f,%f,%f" % \
        (self.wheel_position[i][0],\
        self.wheel_position[i][1],\
        self.wheel_position[i][2])
      xmlElements.append([ name, value, 1 ])

    #### add vehicle data to xml elements
    for p in self.vehicle_parm_default:
      name=p[0]
      value="%f"%p[1]
      xmlElements.append([ name, value, 1 ])

    #### create xml file
    exportXml("vehicle",tmpdir,"VEHICLE",xmlElements)

    #### create zip file
    index_filename=tmpdir+"/VEHICLE"
    createZip(realpath,index_filename,"VEHICLE",elements)

    #### remove temp files
    os.remove(index_filename)
    for name in elements:
      fname=name[1]
      os.remove(fname)
      log("removing '%s'\n"%fname)
    os.removedirs(tmpdir)
    log("removing '%s'\n"%tmpdir)

    return {'FINISHED'}

  def invoke(self,context,event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

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
  log("*** crisalide exporter ***\n")
  register()



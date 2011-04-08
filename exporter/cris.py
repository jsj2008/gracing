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

# ENABLE 'BOUNDING BOX ADJUST'
# i.e. apply translation to exported meshes
#      so that the chassis is centered on origin
#
# set to 0 the following to disable it it
EXP_ENABLE_BBA=1

# EXPORT configuration
EXP_APPLY_OBJ_TRANSFORM=False
EXP_APPLY_MODIFIERS=False

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


def getMaterialFlagsWord(mat):
  # TODO: get "custom property" from mat to
  #       set flags: uptonow they are set to default
  word=0
  for v in MATERIA_FLAGS_LIST:
    if v["default"]:
      word = word | v["value"]
  return word

#def write_file(fp, objects, scene,
def exportMesh(fp, ob, scene,translation=None,
          dest_dir=None,
          addElements=None,
          EXPORT_EDGES=False,
          EXPORT_NORMALS_HQ=False,
          EXPORT_UV=True,
          EXPORT_COPY_IMAGES=False,
          EXPORT_APPLY_MODIFIERS=True,
          EXPORT_ROTX90=True,
          EXPORT_BLEN_OBS=True,
          EXPORT_CURVE_AS_NURBS=True):
    '''
    Basic write function. The context and options must be already set
    This can be accessed externaly
    eg.
    write( 'c:\\test\\foobar.obj', Blender.Object.GetSelected() ) # Using default options.
    '''

    # XXX
    import math

    objects=[ ob ]

    def veckey2d(v):
        return round(v[0], 6), round(v[1], 6)

    log('------------------------ start\n')

    #fp = open(filepath, "wb")

    # Initialize totals, these are updated each object
    totverts = totuvco = totno = 1

    face_vert_index = 1

    globalNormals = {}

    # A Dict of Materials
    # (material.name, image.name):matname_imagename # matname_imagename has gaps removed.
    mtl_dict = {}

    # Get all meshes
    for ob_main in objects:

        # ignore dupli children
        if ob_main.parent and ob_main.parent.dupli_type != 'NONE':
            # XXX
            log(ob_main.name, 'is a dupli child - ignoring')
            continue

        obs = []
        if ob_main.dupli_type != 'NONE':
            # XXX
            log('creating dupli_list on', ob_main.name)
            ob_main.create_dupli_list(scene)

            obs = [(dob.object, dob.matrix) for dob in ob_main.dupli_list]

            # XXX debug print
            log(ob_main.name, 'has', len(obs), 'dupli children')
        else:
            obs = [(ob_main, ob_main.matrix_world)]

        for ob, ob_mat in obs:

            if ob.type != 'MESH':
                continue

            me = ob.create_mesh(scene, EXPORT_APPLY_MODIFIERS, 'PREVIEW')

            if EXPORT_UV:
                faceuv = len(me.uv_textures) > 0
                if faceuv:
                    uv_layer = me.uv_textures.active.data[:]
            else:
                faceuv = False

            me_verts = me.vertices[:]

            # Make our own list so it can be sorted to reduce context switching
            face_index_pairs = [ (face, index) for index, face in enumerate(me.faces)]
            # faces = [ f for f in me.faces ]

            if EXPORT_EDGES:
                edges = me.edges
            else:
                edges = []

            if not (len(face_index_pairs)+len(edges)+len(me.vertices)): # Make sure there is somthing to write

                # clean up
                bpy.data.meshes.remove(me)

                continue # dont bother with this mesh.

            materials = me.materials

            materialNames = []
            materialItems = [m for m in materials]
            if materials:
                for mat in materials:
                    if mat:
                        materialNames.append(mat.name)
                    else:
                        materialNames.append(None)

            # Possible there null materials, will mess up indicies
            # but at least it will export, wait until Blender gets fixed.
            materialNames.extend((16-len(materialNames)) * [None])
            materialItems.extend((16-len(materialItems)) * [None])

            # Sort by Material, then images
            # so we dont over context switch in the obj file.
            if faceuv:
                face_index_pairs.sort(key=lambda a: (a[0].material_index, hash(uv_layer[a[1]].image), a[0].use_smooth))
            elif len(materials) > 1:
                face_index_pairs.sort(key = lambda a: (a[0].material_index, a[0].use_smooth))
            else:
                # no materials
                face_index_pairs.sort(key = lambda a: a[0].use_smooth)

            # Set the default mat to no material and no image.
            contextMat = (0, 0) # Can never be this, so we will label a new material teh first chance we get.
            contextSmooth = None # Will either be true or false,  set bad to force initialization switch.

            if EXPORT_BLEN_OBS:
                name1 = ob.name
                name2 = ob.data.name
                if name1 == name2:
                    obnamestring = fixName(name1)
                else:
                    obnamestring = '%s_%s' % (fixName(name1), fixName(name2))

                log('exporting object %s\n' % obnamestring) # Write Object name


            # export vertices
            binWrite_mark(fp,MARK_VERTICES)
            binWrite_int(fp,len(me_verts))
            for v in me_verts:
              if EXP_ENABLE_BBA and translation != None:
                co=applyTranslation(v.co,translation)
              else:
                co=v.co
              binWrite_pointVect(fp,co)

            # prepare UV coordinates
            if faceuv:
                uv_face_mapping = [[0,0,0,0] for i in range(len(face_index_pairs))] # a bit of a waste for tri's :/
                uv_cords = [ ]

                uv_dict = {} # could use a set() here
                uv_layer = me.uv_textures.active.data

                for f, f_index in face_index_pairs:
                    for uv_index, uv in enumerate(uv_layer[f_index].uv):
                        uvkey = veckey2d(uv)
                        try:
                            uv_face_mapping[f_index][uv_index] = uv_dict[uvkey]
                        except:
                            uv_face_mapping[f_index][uv_index] = uv_dict[uvkey] = len(uv_dict)
                            uv_cords.append(uv)
                uv_unique_count = len(uv_dict)
                binWrite_mark(fp,MARK_UVCOORDS)
                binWrite_int(fp,uv_unique_count)

                for uv in uv_cords:
                  binWrite_pointUV(fp,uv)


            # NORMAL, Smooth/Non smoothed.
            if not faceuv:
                f_image = None

            matImgFaces={}

            for f, f_index in face_index_pairs:
              f_mat = min(f.material_index, len(materialNames)-1)
              if faceuv:
                tface = uv_layer[f_index]
                f_image = tface.image
              if faceuv and f_image: # Object is always true.
                key = materialNames[f_mat],  f_image.name
              else:
                key = materialNames[f_mat],  None # No image, use None instead.
              
              try:
                matImgFaces[key].append(f)
              except:
                matImgFaces[key]=[ f ]

            for mat in materialItems:
              if mat != None:
                exportMaterial(fp,mat,dest_dir,addElements)

            for matName, imageName in matImgFaces:
              faces=matImgFaces[(matName,imageName)]
              n_faces=len(faces)
              binWrite_mark(fp,MARK_USE_MATERIAL)
              binWrite_string(fp,matName)

              if faceuv:  
                binWrite_mark(fp,MARK_FACES_AND_UV)
              else:
                binWrite_mark(fp,MARK_FACES_ONLY)
              binWrite_int(fp,n_faces)
              log("Mat: '%s', img: '%s' has %d faces\n"%(matName,imageName,n_faces))
              for f in faces:
                f_smooth= f.use_smooth
                f_mat = min(f.material_index, len(materialNames)-1)
                f_index = f.index
                log("  face %d - "%f_index)
                f_v_orig = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]
                f_v_iter = (f_v_orig, )
                for f_v in f_v_iter:
                  binWrite_int(fp,len(f_v))
                  if faceuv:
                    for vi, v in f_v:
                      binWrite_int(fp,v.index + totverts - 1)
                      binWrite_int(fp,totuvco + uv_face_mapping[f_index][vi]) 
                      log( ' %d/%d' % (\
                        v.index + totverts,\
                        totuvco + uv_face_mapping[f_index][vi])) # vert, uv
                    face_vert_index += len(f_v)
                  else: # No UV's
                    for vi, v in f_v:
                      binWrite_int(fp,v.index + totverts - 1)
                      log( ' %d' % (v.index + totverts) )
                log("\n")       

            log("---------------- done\n")

            totverts += len(me_verts)
            if faceuv:
                totuvco += uv_unique_count

            # clean up
            bpy.data.meshes.remove(me)

        if ob_main.dupli_type != 'NONE':
            ob_main.free_dupli_list()

    return True


def applyTransform(v,matrix):
  nv=[0., 0., 0.]
  for c in range(0,3):
    for r in range(0,3):
      nv[c] = nv[c] + v[r] * matrix[r][c]
    nv[c] = nv[c] + matrix[3][c]
  return nv

def applyTranslation(v,translation):
  nv=[0., 0., 0.]
  nv[0] = v[0] + translation[0]
  nv[1] = v[1] + translation[1]
  nv[2] = v[2] + translation[2]
  return nv

def exportMaterial(fp,ma,dest_dir,addElements):
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

  imageName=""
  for mtex in ma.texture_slots:
    if mtex and mtex.texture.type == 'IMAGE' and dest_dir != None:
      try:
        filepath = copy_image(mtex.texture.image,dest_dir)
        addElements.append([ "image", filepath])
        imageName=filepath
        break
      except:
        log("hwat??\n")
        # Texture has no image though its an image type, best ignore.
        pass
  binWrite_string(fp,imageName)


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

def exportObject(tmp_dir_name,context,obj,translation=None,addElements=None):
  if obj.type == 'MESH':
    filename=tmp_dir_name+"/"+obj.name+".mesh"
    fp = open(filename, 'wb')    
    ret=exportMesh(fp,obj,context.scene,translation,tmp_dir_name,addElements)
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
  if obj.type == "EMPTY":
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
  fp.write("</%s>\n"%root_name)
  fp.close()

def createZip(path,index_file,index_name,elements):
    zf=zipfile.ZipFile(path,"w")
    for name in elements:
      if len(name) == 3:
        continue
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

  def getTrackInfo(self, elements,es):
    self.getWorldProperties(elements)
    for e in es:
      if e.name == 'track.start':
        val="%f,%f,%f"%( e.location[0], e.location[2], e.location[1])
        elements.append([ 'track_start_pos', val, 1])
        val="%f,%f,%f"%( e.rotation_euler[0], e.rotation_euler[1], e.rotation_euler[2])
        elements.append([ 'track_start_rot', val, 1])

  def getWorldProperties(self,elements):
    for p in WORLD_PROPERTIES:
      name=p["name"]
      value=p["default"]
      if type(value) == type([]):
        svalue="%f,%f,%f"%( value[0], value[2], value[1])
      elif type(value) == type(""):
        svalue=value
      else:
        svalue="unknowwn"
      elements.append([ name, svalue, 1])
      

  def execute(self, context):
    filepath=self.properties.filepath
    realpath = os.path.realpath(os.path.expanduser(filepath))
    name = os.path.basename(filepath)
    dirname = os.path.dirname(filepath)
    tmpdir = dirname + "/tmp";

    empties=[ ]
    if not os.path.exists(tmpdir):
      os.mkdir(tmpdir)
    else:
      if os.path.isdir(tmpdir):
        pass
      else:
        RaiseError("horrorororor!")
    log("Exporing track (dir %s): %s\n"%(dirname,filepath))
    elements=[]

    for ob in bpy.data.objects:
      if ob.type == "EMPTY":
        empties.append(ob)
        continue
      addElements=[ ]
      name=exportObject(tmpdir,context,ob,None,addElements)
      if name != None:
        elements.append(name)
        for el in addElements:
          elements.append(el)
   

    ns=exportCameras(tmpdir)
    for n in ns:
      elements.append(n)

    xmlElements=[ ]

    for e in elements:
      xmlElements.append(e)

    #### retrieve track info
    self.getTrackInfo(xmlElements,empties)

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
    #shutil.rmtree(tmpdir)
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

  chassis_bounds=BBound()

  vehicle_parm_default=[
   [ "vehicleSteering", 0.0 ],\
   [ "steeringIncrement", 0.04 ],\
   [ "steeringClamp", 0.5 ],\
   [ "suspensionStiffness", 20.0 ],\
   [ "suspensionDamping", 2.3 ],\
   [ "suspensionCompression", 4.4 ],\
   [ "rollInfluence", 0.1 ],\
   [ "wheelFriction",  1000. ],\
   [ "suspensionRestLength",  0.6 ]\
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
    self.wheel_width[idx]=dimY;
    self.wheel_position[idx][0]=ob.location[0]
    self.wheel_position[idx][1]=ob.location[2]
    self.wheel_position[idx][2]=ob.location[1]

    if EXP_ENABLE_BBA:
      offs=self.chassis_bounds.getOffsets()
      self.wheel_position[idx][0]=self.wheel_position[idx][0]+offs[0]
      self.wheel_position[idx][1]=self.wheel_position[idx][1]+offs[2]
      self.wheel_position[idx][2]=self.wheel_position[idx][2]+offs[1]


  def execute(self, context):

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
        wheel_fr_objs.append(ob)
      elif ob.name == "wheel.fl":
        wheel_fl_objs.append(ob)
      elif ob.name == "wheel.rr":
        wheel_rr_objs.append(ob)
      elif ob.name == "wheel.rl":
        wheel_rl_objs.append(ob)
      elif ob.type == 'MESH':
        bb=ob.bound_box
        for v in bb:
          self.chassis_bounds.update(v)
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

    for ob in wheel_fr_objs:
      self.updateWheelInfo(ob)

    for ob in wheel_fl_objs:
      self.updateWheelInfo(ob)

    for ob in wheel_rr_objs:
      self.updateWheelInfo(ob)

    for ob in wheel_rl_objs:
      self.updateWheelInfo(ob)

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
    offs=self.chassis_bounds.getOffsets()
    for ob in chassis_objs: 
      addElements=[ ]
      el=exportObject(tmpdir,context,ob,offs,addElements)
      if el != None:
        el[0]="chassis"
        log("%s,%s\n"%(el[0],el[1]))
        elements.append(el)
      log("  -exporting obj '%s'\n"%ob.name)

    log("Exporting wheels\n");
    log("  - front left\n")
    for ob in wheel_fl_objs:
      el=exportObject(tmpdir,context,ob,None,addElements)
      if el != None: 
        el[0]="wfl";
        elements.append(el)

    log("  - front right\n")
    for ob in wheel_fr_objs:
      el=exportObject(tmpdir,context,ob,None,addElements)
      if el != None: 
        el[0]="wfr";
        elements.append(el)

    log("  - rear left\n")
    for ob in wheel_rl_objs:
      el=exportObject(tmpdir,context,ob,None,addElements)
      if el != None: 
        el[0]="wrl";
        elements.append(el)

    log("  - rear right\n")
    for ob in wheel_rr_objs:
      el=exportObject(tmpdir,context,ob,None,addElements)
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



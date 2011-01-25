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
import bpy,os,struct,zipfile

MARK_VERTICES=0xf100
MARK_FACES_ONLY=0xf101
MARK_MATERIAL=0xf102
MARK_USE_MATERIAL=0xf103

# LOG configuration
LOG_ON_STDOUT=1
LOG_ON_FILE=0
LOG_FILENAME="/tmp/log.txt"

def log(str):
  if LOG_ON_STDOUT==1:
    print(str,end="")
  if LOG_ON_FILE==1:
    f=open(LOG_FILENAME,"a")
    f.write(str)
    f.close()

def binWrite_mark(fp,value):
  fp.write(struct.pack("H",value))

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

class export_OT_track(bpy.types.Operator):
  bl_idname = "io_export_scene.crisalide_exporter"
  bl_description = 'Export to crisalide file format (.track)'
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


  def exportMesh3(self, fp, ob):
    materials=ob.data.materials
    faces=ob.data.faces
    vertices=ob.data.vertices

    # export vertices
    binWrite_mark(fp,MARK_VERTICES)
    binWrite_int(fp,len(vertices))
    log("vertices: %d\n"%len(vertices))
    for v in vertices:
      binWrite_pointVect(fp,v.co)
      #log("vertex: %f,%f,%f\n"%(v.co[0],v.co[1],v.co[2]))

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
    log("Exporting material: '%s'\n"%ma.name)
    alpha=ma.alpha
    specular=ma.specular_color
    diffuse=ma.diffuse_color
    binWrite_mark(fp,MARK_MATERIAL)
    Ka=[ 1-alpha, 1-alpha, 1-alpha ]
    binWrite_string(fp,ma.name)
    binWrite_color(fp,Ka)
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


  def exportObject(self,tmp_dir_name,obj):
    if obj.type == 'MESH':
      filename=tmp_dir_name+"/"+obj.name+".mesh"
      fp = open(filename, 'wb')    
      self.exportMesh3(fp,obj)
      fp.close()
      return [ 'mesh' , filename ]
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

def menu_func(self, context):
    self.layout.operator(export_OT_track.bl_idname, text="Crisalide track exporter")

def register():
    bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
  print("*** crisalide exporter ***")
  register()



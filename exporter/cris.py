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

Read the script manual for further information.
"""
import bpy,os,struct

MARK_VERTICES=0xf100
MARK_FACES_ONLY=0xf101
MARK_MATERIAL=0xf102

def binWrite_mark(fp,value):
  print("%X"%value)
  fp.write(struct.pack("H",value))

def binWrite_float(fp,value):
  v=struct.pack("d",value)
  fp.write(v)

def binWrite_int(fp,value):
  v=struct.pack("i",value)
  fp.write(v)

def binWrite_pointVect(fp,point):
  #TODO: use a transform matrix
  v=struct.pack("ddd",point[0],point[2],-point[1])
  fp.write(v)

def binWrite_color(fp,color):
  v=struct.pack("ddd",color[0],color[1],color[2])
  fp.write(v)

def binWrite_string(fp,string):
  l=len(string)
  fp.write(struct.pack("H",l))
  fp.write(struct.pack("s",string))

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
    print("Exporting material: ",ma.name)
    alpha=ma.alpha
    specular=ma.specular_color
    diffuse=ma.diffuse_color
    binWrite_mark(fp,MARK_MATERIAL)
    Ka=[ 1-alpha, 1-alpha, 1-alpha ]
    binWrite_string(fp,ma.name)
    binWrite_color(fp,Ka)
    binWrite_color(fp,diffuse)
    binWrite_color(fp,specular)

    

  def execute(self, context):
    filepath=self.properties.filepath
    name = os.path.basename(filepath)
    realpath = os.path.realpath(os.path.expanduser(filepath))
    fp = open(realpath, 'wb')    

    #materials
    for ma in bpy.data.materials:
      self.exportMaterial(fp,ma)

    # objects
    for ob in bpy.data.objects:
      if ob.type == 'MESH':
        self.exportMesh(fp,ob)


    binWrite_string(fp,"*** crisalide exported file: done ***\n")

    fp.close()

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



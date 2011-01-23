#!BPY
"""
Name: 'Crisalide track exporter (.cris,)...'
Blender: 253
Group: 'Export'
Tooltip: 'Exports meshes to Irrlicht mesh & scene files'
"""

__author__ = ['Gianni Masullo(pc0de)']
__version__ = '0.4'
__url__ = [ ]
__bpydoc__ = """\
Crisalide xml exporter

Read the script manual for further information.
"""
import bpy,os,struct

def binWrite_float(fp,value):
  v=struct.pack("d",value)
  fp.write(v)

def binWrite_pointVect(fp,point):
  #TODO: use a transform matrix
  v=struct.pack("ddd",point[0],point[2],-point[1])
  fp.write(v)

def binWrite_string(fp,string):
  fp.write(struct.pack("s",string))

def binWrite_face(fp,face):
  if len(face) == 4:
    v=struct.pack("iiii",face[0]+1, face[1]+1, face[2]+1,face[3]+1)
  else:
    v=struct.pack("iii",face[0]+1, face[1]+1, face[2]+1)
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

    binWrite_string(fp,"\n##\n## exporing mesh '%s'\n##\n"%ob.name)


    binWrite_string(fp,"### materials: \n")
    for mat in ob.material_slots:
      binWrite_string(fp,"## Name: '%s'\n"%mat.name)

    me = ob.data

    # export vertices
    binWrite_string(fp,"#Vertices\n")
    for v in me.vertices:
      x=v.co
      binWrite_pointVect(fp,x)

    # export faces
    if len(me.uv_textures) > 0:
      binWrite_string("#Faces with texture\n")
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
      binWrite_string(fp,"#Faces without textures\n")
      for f in me.faces:
        vs = f.vertices
        binWrite_face(fp,vs)

    binWrite_string(fp,"\n")

  def exportMaterial(self,fp,ma):
    binWrite_string(fp,"mat: '%s'\n"%ma.name)
    binWrite_string(fp,"ambient: %1.4f\n"%ma.ambient)
    binWrite_string(fp,"diffuse: %1.4f %1.4f %1.4f\n"%
        (ma.diffuse_color[0],
        ma.diffuse_color[1],
        ma.diffuse_color[2]))
    

  def execute(self, context):
    filepath=self.properties.filepath
    name = os.path.basename(filepath)
    realpath = os.path.realpath(os.path.expanduser(filepath))
    fp = open(realpath, 'wb')    
    binWrite_string(fp,"*** crisalide exported file ***\n")

    # objects
    for ob in bpy.data.objects:
      if ob.type == 'MESH':
        self.exportMesh(fp,ob)
      else:
        binWrite_string(fp,"##\n## obtype: '%s'\n##\n\n"%ob.type)

    #materials
    for ma in bpy.data.materials:
      self.exportMaterial(fp,ma)

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



#!/opt/local/bin/python
import sys,getopt,shutil,os,re

ACTION_NONE=0
ACTION_COPY_VEHICLES=1
ACTION_PRINT_HELP=2
ACTION_COPY_TRACKS=3
ACTION_BUILD_TREE=4
ACTION_COPY_FILE=5
ACTION_REMOVE_DIR=6
ACTION_EXTRACT_CFG=7

########## options
globargs=[]
action=ACTION_PRINT_HELP
target_dir=None
resource_dir=None
vehicles=[
  "sprinter",
  "turing_machine",
  "tractor",
  "skate"
]

tracks=[
  "devtrack",
  "arena_stadium",
  "beach",
  "mathclass",
  "tuxtollway",
  "farm",
  "canyon"
]

paths=[
  "Contents/MacOs",
  "Contents/Resources",
  "Contents/Resources/Vehicles",
  "Contents/Resources/Tracks"
]


def error(message):
	print("FATAL: %s"%message)
	sys.exit(2)

def copy_list(list,src_dir,tgt_dir):
  for f  in list:
    if type(f) == type(""): # is a string
      print(" - copying '%s'"%f)
      src=src_dir+"/"+f+"/"+f+".zip"
    try:
      shutil.copy(src,tgt_dir)
    except IOError, err:
      print("   cannot copy: '%s'"%err)
    except OSError, err:
      print("   cannot copy: '%s'"%err)

def extract_cfg_definition(filename, list):
  try:
    file=open(filename)
  except:
    error("Cannot open '%s'"%filename)
  lines=file.readlines()
  p_d = re.compile("CFG_PARAM_D\((.*)\)=([^;]*);.*");
  p_v3 = re.compile("CFG_PARAM_V3\((.*)\)=([^;]*);.*");
  p_bool = re.compile("CFG_PARAM_BOOL\((.*)\)=([^;]*);.*");
  p_uint = re.compile("CFG_PARAM_UINT\((.*)\)=([^;]*);.*");

  res=[ [ p_d, "double" ] , 
        [ p_v3, "v3" ] ,
        [ p_bool, "bool" ] ,
        [ p_uint, "unsigned" ] ]
  for line in lines:
    for c in res:
      r=c[0]
      t=c[1]
      m=r.match(line)
      if m and len(m.groups()) == 2:
        print("    found '%s' = '%s'"%(m.group(1),m.group(2)))
        list.append([ m.group(1), m.group(2), t ])
  file.close()

def build_tree():
  print("Building application tree")
  if target_dir == None:
    error("Please define a target (-tgtdir=...)")

  print("  - creating dir '%s'"%target_dir)
  try:
    os.makedirs(target_dir)
  except IOError, err:
    print("   cannot create: '%s'"%err)
  except OSError, err:
    print("   cannot create: '%s'"%err)


  for p in paths:
    dname=target_dir+"/"+p
    print("  - creating dir '%s'"%dname)
    try:
      os.makedirs(dname)
    except IOError, err:
      print("   cannot create: '%s'"%err)
    except OSError, err:
      print("   cannot create: '%s'"%err)

def copy_vehicles():
  if target_dir == None:
    error("please define a target dir (-tgtdir=...)")

  if resource_dir == None:
    error("Please define a resource dir (-resdir=...)")

  print("Copying vehicles to %s"%target_dir)
  copy_list(vehicles,resource_dir,target_dir)

def copy_tracks():
  if target_dir == None:
    error("please define a target dir (-tgtdir=...)")

  if resource_dir == None:
    error("please define a resource dir (-resdir=...)")

  print("Copying tracks to %s"%target_dir)

  copy_list(tracks,resource_dir,target_dir)


def extract_configuration():
  global globargs
  print("Extracting cfg directives from sources ")
  list=[ ]
  for f in globargs:
   print("  - extracting cfg from '%s'"%f)
   if f != "config.cc":
     extract_cfg_definition(f, list)
  
  print("Creating config.xml")
  f=open("config.xml","w")
  f.write("<config>\n")
  for el in list:
    f.write("  <%s>%s</%s>\n"%(el[0],el[1],el[0]))
  f.write("</config>\n")
  f.close()

  print("Creating config.cc")
  f=open("config.cc","w")
  f.write("#include \"ResourceManager.h\"\n")
  f.write("#include \"config.h\"\n")
  f.write("#include \"gmlog.h\"\n")
  for el in list:
    if el[2] == 'double':
      f.write("extern double %s;\n"%(el[0]))
    elif el[2] == 'bool':
      f.write("extern bool %s;\n"%(el[0]))
    elif el[2] == 'v3':
      f.write("extern double %s[3];\n"%(el[0]))
    elif el[2] == 'unsigned':
      f.write("extern unsigned %s;\n"%(el[0]))

  f.write("void ConfigInit::initGlobVariables(ResourceManager * resman)\n{\n")
  for el in list:
    f.write("  if(!resman->cfgGet(\"%s\",%s)) {\n"%(el[0],el[0]));
    f.write("    GM_LOG(\"returning for '%s' value '%s'\\n\");\n"%(el[0],el[1]))
    f.write("    %s=%s;\n"%(el[0],el[1]));
    f.write("  }\n");
  f.write("}\n");

  f.close()

def remove_dir():
  global globargs
  if len(globargs) != 1:
    error("Can only remove one dir")
  tgt=globargs[0]
  print ("Removing dir '%s'"%tgt)
  try:
    shutil.rmtree(tgt)
  except OSError,err:
    print("  cannot remove '%s'"%err)

def copy_file():
  global globargs
  if len(globargs) != 2: 
    error("Can only copy one file")
  src=globargs[0]
  dst=globargs[1]
  print ("Copying '%s' to '%s'"%(src,dst))
  try:
    shutil.copy(src,dst)
  except IOError,err:
    error("Cannot copy '%s'"%err)
  except OSError,err:
    error("Cannot copy '%s'"%err)

def usage():
	print("Usage:")

def parse_command_line():
  global action
  global target_dir
  global resource_dir
  global globargs
  try:                                
    opts, globargs = getopt.getopt(sys.argv[1:], "", ["help", "tgtdir=", "resdir=", "cpv", "cpt", "mkt", "cp", "rmd", "cfg"]) 
  except getopt.GetoptError,err: 
    error(err)                          
  for opt,arg in opts:
    if opt == "--cpv":
     action=ACTION_COPY_VEHICLES
    if opt == "--cpt":
      action=ACTION_COPY_TRACKS
    if opt == "--mkt":
      action=ACTION_BUILD_TREE
    if opt == "--cp":
      action=ACTION_COPY_FILE
    if opt == "--rmd":
      action=ACTION_REMOVE_DIR
    if opt == "--cfg":
      action=ACTION_EXTRACT_CFG
    elif opt == "--tgtdir":
      target_dir=arg
    elif opt == "--resdir":
      resource_dir=arg

parse_command_line()

if action == ACTION_NONE:
  print("No action required");
elif action == ACTION_COPY_VEHICLES:
  copy_vehicles()
elif action == ACTION_COPY_TRACKS:
  copy_tracks()
elif action == ACTION_BUILD_TREE:
  build_tree()
elif action == ACTION_COPY_FILE:
  copy_file()
elif action == ACTION_REMOVE_DIR:
  remove_dir()
elif action == ACTION_EXTRACT_CFG:
  extract_configuration()
elif action == ACTION_PRINT_HELP:
  usage()
	



#!/opt/local/bin/python
import sys,getopt,shutil,os

ACTION_NONE=0
ACTION_COPY_VEHICLES=1
ACTION_PRINT_HELP=2
ACTION_COPY_TRACKS=3
ACTION_BUILD_TREE=4
ACTION_COPY_FILE=5
ACTION_REMOVE_DIR=6

########## options
globargs=[]
action=ACTION_PRINT_HELP
target_dir=None
resource_dir=None
vehicles=[
  "squared",
  "sprinter",
]

tracks=[
  "devtrack",
  "track-1",
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

  print("Coying vehicles to %s"%target_dir)
  copy_list(vehicles,resource_dir,target_dir)

def copy_tracks():
  if target_dir == None:
    error("please define a target dir (-tgtdir=...)")

  if resource_dir == None:
    error("please define a resource dir (-resdir=...)")

  print("Coying tracks to %s"%target_dir)

  copy_list(tracks,resource_dir,target_dir)

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
    opts, globargs = getopt.getopt(sys.argv[1:], "", ["help", "tgtdir=", "resdir=", "cpv", "cpt", "mkt", "cp", "rmd"]) 
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
elif action == ACTION_PRINT_HELP:
  usage()
	



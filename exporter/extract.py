#!/opt/local/bin/python
import zipfile,os,sys

vehicle_MANIFEST="VEHICLE"

# /////////////////////////
def build_output_vehicle_MANIFEST_name(archive_name):
  return archive_name + "_VEHICLE.xml"



# /////////////////////////

def extract_VEHICLE(archive_name,output=""):
  if output == "":
    output=build_output_vehicle_MANIFEST_name(archive_name)
  print("Extracting vehicle manifest from '%s' into '%s'"%(archive_name,output))
  try:
    archive=zipfile.ZipFile(archive_name)
  except:
    print("FATAL: Cannot open '%s'"%archive_name)
    return

  try:
    archive.write(vehicle_MANIFEST,output)
  except:
    print("FATAL: '%s' is either corrupted or doesnt not contain vehicle manifest")
    return


  archive.close()


def usage():
  print("usage:")
  print("  extract ev <zipfile> - extract vehicle manifest file from compressed archive")


if len(sys.argv)<2:
  usage()
  sys.exit()

if sys.argv[1]=='ev':
  if len(sys.argv) != 3:
    print("FATAL: please give the vehicle archive filename")
    sys.exit()
  extract_VEHICLE(sys.argv[2])

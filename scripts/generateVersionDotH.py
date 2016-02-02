#!/usr/bin/python
import sys
import subprocess
import os
import time
import re

# This script is used for auto-generation of OSP Firmware version based on the git tag available for the commit
# Tags should have the format of <ID>_X.Y.Z or <ID>-X.Y.Z where <ID> is any string identifier e.g. "tag" or "release"
# and X.Y.Z are Major.Minor.Patch decimal numbers. Examples of tag: "tag_OSP-1.1.16", "release-2.0.1"
# If no git tag is available (e.g. for developer builds), version is set as 0.0.0.git#
# This script needs Python 2.7 installed and in environment path to be able to run in windows/Linux
# IMPORTANT: This script must be invoked as pre-build step to generate the header file that will be included
# in the library sources

def start(scriptpath):
  #scriptpath = os.path.dirname(os.path.realpath('__file__'))
  #os.chdir(scriptpath+"/../")

  #Set default values for version in case no tag exists on HEAD
  verMajor = 0
  verMinor = 0
  verPatch = 0

  ghash = subprocess.check_output("git rev-parse --short HEAD".split()).strip('\n')
  gtag  = subprocess.check_output("git tag -l --contains HEAD".split()).strip('\n')
  if gtag:
    verList = re.split('_|-',gtag)[-1].split('.')
    if len(verList) >= 3:
      verMajor = int(verList[0])
      verMinor = int(verList[1])
      verPatch = int(verList[2])

  tagVersion = "%d.%d.%d" %(verMajor,verMinor,verPatch)

  modified_files = subprocess.check_output("git status -s --porcelain".split() )
  filelist = modified_files.split('\n')
  dirty = 0
  for afile in filelist:
    if not('??' in afile) and ('M' in afile):
      dirty = dirty+1
  if dirty > 0 : ghash = "%s#%d" %(ghash,dirty)

  with open(scriptpath+"/../include/"+"osp-sh-version.h","w") as outputfile:
    outputfile.write("/*\n * Auto-generated file! Do not edit and do not commit to git.\n */\n")
    outputfile.write("#ifndef __OSP_SH_VERSION_H\n")
    outputfile.write("#define __OSP_SH_VERSION_H\n\n")
    outputfile.write("#define SH_MODULE_NAME     \"Open Sensor Platform Hub\"\n")
    outputfile.write("#define SH_VERSION_MAJOR   %d\n" %(verMajor))
    outputfile.write("#define SH_VERSION_MINOR   %d\n" %(verMinor))
    outputfile.write("#define SH_VERSION_PATCH   %d\n" %(verPatch))
    outputfile.write("#define SH_VERSION_STRING  \"%s.%s\"\n" %(tagVersion,ghash.strip()))

    outputfile.write("\n#endif /* __OSP_SH_VERSION_H */\n")
    outputfile.write("/*\n * END OF FILE\n */\n")

def main():
  if len(sys.argv) > 0:
    start(os.path.dirname(os.path.realpath(sys.argv[0])))
    sys.exit(0)


if __name__ == '__main__':
  main()
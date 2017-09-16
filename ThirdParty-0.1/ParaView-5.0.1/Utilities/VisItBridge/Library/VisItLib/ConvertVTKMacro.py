import os
import fileinput
#convert VTK Rev macros in VisIt


def cleanRevMacro( filename, filepath ):
  fpath = os.path.join(filepath,filename)
  for line in fileinput.input(fpath,inplace=1):  
    if line.find("vtkTypeRevisionMacro") >= 0:
      line = line.replace("vtkTypeRevisionMacro","vtkTypeMacro")
    elif line.find("vtkCxxRevisionMacro") >= 0:
      continue #these aren't needed anymore
    line = line[:-1]
    print line

if __name__ == "__main__":
  hidden = "."+os.sep+"."  
  
  #parse the first time to build the set of file names we can parse
  for dirname,dirnames,filenames in os.walk('.'):         
    if (dirname[0:3] == hidden or dirname == "."):
      #do not traverse hidden directores
      continue
    for filename in filenames:    
      if filename[0:3] == "vtk":
        cleanRevMacro(filename, dirname)
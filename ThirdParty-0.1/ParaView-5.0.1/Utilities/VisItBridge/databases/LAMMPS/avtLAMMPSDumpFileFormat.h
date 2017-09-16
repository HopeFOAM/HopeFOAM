
// ************************************************************************* //
//                            avtLAMMPSDumpFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_LAMMPS_DUMP_FILE_FORMAT_H
#define AVT_LAMMPS_DUMP_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtFileFormatInterface.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtLAMMPSDumpFileFormat
//
//  Purpose:
//      Reads in LAMMPS molecular files.  Supports up to MAX_LAMMPS_VARS extra
//      data fields.  Supports multiple timesteps in one file.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  6, 2009
//
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes bounds/unit cell, and an optional atom format string.
//
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
// ****************************************************************************

class avtLAMMPSDumpFileFormat : public avtMTSDFileFormat
{
private:
  struct AtomInfo
  {
    AtomInfo():vars(),names(),types(),ids(),
              enableType(false),enableIds(false),numVars(0){}

    void Clear()
        { vars.resize(0); names.resize(0); types.resize(0); ids.resize(0);
          enableType = false; enableIds = false; numVars = 0;
        };

    void AddVariable(const std::string& name)
        {
        //if it is id or type we don't push back since it is stored separately
        if(name == "id")
            {
            enableIds = true;
            }
        if(name == "type")
            {
            enableType = true;
            }
        else
            {
            names.push_back(name);
            vars.resize(names.size());
            }
        ++numVars;
        }
    int GetNumberOfVariables() const
        {
        return numVars;
        }

    void SetNumberOfAtoms(std::size_t size)
    {
        if(HasIds())    { ids.resize(size);   }
        if(HasTypes())  { types.resize(size); }
        for(std::size_t i=0; i < vars.size(); ++i) { vars[i].resize(size); }
    }


    //helper methods to get the variable names
    typedef std::vector<std::string>::const_iterator name_iterator;
    name_iterator GetVariableNamesBegin() const { return names.begin(); }
    name_iterator GetVariableNamesEnd() const { return names.end(); }
    int GetVarIndex(const std::string& n) const
    {
    for(int i=0; i < names.size(); ++i)
        {
        if(names[i] == n)
          { return i; }
        }
        return -1;
    }

    //helper methods for types
    bool SetHasTypes(bool b) { enableType = b; }
    bool HasTypes() const { return enableType; }
    const std::vector<int>& GetTypes() const { return types; }
    void SetType(std::size_t pos, int t) { types[pos]=t; }

    //helper methods for ids
    bool SetHasIds(bool b) { enableIds = b; }
    bool HasIds() const { return enableIds; }
    const std::vector<int>& GetIds() const { return ids; }
    void SetId(std::size_t pos, int i) { ids[pos]=i; }


    //operators to make this like you can access the vars easily
    const std::vector<float> &operator[](std::size_t idx) const
        { return vars[idx]; }
    std::vector<float>& operator[](std::size_t idx)
        { return vars[idx]; }


    private:
        std::vector < std::vector<float> > vars;
        std::vector < std::string > names;
        std::vector < int > types;
        std::vector < int > ids;
        bool enableType, enableIds;
        int numVars;
  };

public:
    static bool        FileExtensionIdentify(const std::string&);
    static bool        FileContentsIdentify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);
  public:
                       avtLAMMPSDumpFileFormat(const char *);
    virtual           ~avtLAMMPSDumpFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "LAMMPS"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    ifstream                       in;
    std::vector<int>               cycles;
    std::vector<istream::pos_type> file_positions;
    std::string                    filename;
    bool                           metaDataRead;
    int                            nTimeSteps;

    double                         xMin, xMax;
    double                         yMin, yMax;
    double                         zMin, zMax;

    int                      currentTimestep;
    bool                     xScaled,yScaled,zScaled;
    int                      xIndex, yIndex, zIndex;
    int                      speciesIndex, idIndex;

    std::vector<int>               nAtoms;
    AtomInfo                       atomVars;

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void            OpenFileAtBeginning();

    void ReadTimeStep(int);
    void ReadAllMetaData();
};


#endif

/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtGadget_customFileFormat.C                   //
// ************************************************************************* //

#include <avtGadget_customFileFormat.h>

#include <string>
#include <vtkVertex.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkIntArray.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <avtIntervalTree.h>

using std::string;

// ****************************************************************************
//  Method: avtGadget_customFileFormat::Block constructor
//
//  A Block is used to represent a Gadget2 datablock
//  On construction, we read the block name and determine from its size
//  if we deal with vector or scalar data. We also determine to which particle
//  types this block belongs (by guessing from some possibilities). For every
//  available type, we then store the offset in the snapshot file and the
//  size in bytes. This makes it very easy to read the actual data later on.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
avtGadget_customFileFormat::Block::Block(std::istream& stream,
                                         unsigned int* npart)
{
    unsigned int blocksize = get_blocksize(stream);
    char name[5];
    stream.read(name, 4);
    name[4] = '\0';
    _name = std::string(name);
    if(_name == "HEAD"){
        _vec = false;
        unsigned int datasize = get_blocksize(stream);
        blocksize -= get_blocksize(stream);
        if(blocksize){
            // ERROR!
            cerr << "ERROR!" << endl;
        }
        blocksize = get_blocksize(stream);
        stream.seekg(blocksize, stream.cur);
        blocksize -= get_blocksize(stream);
        if(blocksize){
            // ERROR
            cerr << "ERROR" << endl;
        }
    } else {
        unsigned int datasize = get_blocksize(stream);
        blocksize -= get_blocksize(stream);
        if(blocksize){
            // ERROR!
            cerr << "ERROR!" << endl;
        }
        // subtract size of block info at start and ending of datablock
        datasize -= 8;
        unsigned int totnpart = 0;
        for(unsigned int i = 0; i < 6; i++){
            totnpart += npart[i];
        }
        _vec = (datasize/(totnpart*sizeof(float)) == 3);
        if(npart[0]*sizeof(float) == datasize){
            _mesh[0] = true;
            _mesh[1] = false;
            _mesh[2] = false;
            _mesh[3] = false;
            _mesh[4] = false;
            _mesh[5] = false;
        } else {
            _mesh[0] = true;
            _mesh[1] = npart[1] > 0;
            _mesh[2] = npart[2] > 0;
            _mesh[3] = npart[3] > 0;
            _mesh[4] = npart[4] > 0;
            _mesh[5] = npart[5] > 0;
        }
        if((npart[0]+npart[5])*sizeof(float) == datasize){
            _mesh[0] = true;
            _mesh[1] = false;
            _mesh[2] = false;
            _mesh[3] = false;
            _mesh[4] = false;
            _mesh[5] = true;
        }
        blocksize = get_blocksize(stream);
        for(unsigned int i = 0; i < 6; i++){
            _streampos[i] = stream.tellg();
            if(_mesh[i]){
                _streamsize[i] = npart[i]*sizeof(float);
                if(_vec){
                    _streamsize[i] *= 3;
                }
                stream.seekg(_streamsize[i], stream.cur);
            }
        }
        blocksize -= get_blocksize(stream);
        if(blocksize){
            // ERROR
            cerr << "ERROR" << endl;
        }
    }
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::Block::is_vec
//
//  This function returns true if the Block consists of vector data.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
bool avtGadget_customFileFormat::Block::is_vec()
{
    return _vec;
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::Block::get_name
//
//  This function returns a string representation of the name of the Block
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
std::string avtGadget_customFileFormat::Block::get_name()
{
    return _name;
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::Block::has_data
//
//  This function returns true if the Block contains data for the given particle
//  type.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
bool avtGadget_customFileFormat::Block::has_data(unsigned int index)
{
    return _mesh[index];
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::Block::get_data
//
//  This method reads the data stored in the Block for the given particle type.
//  If the requested type has no data in this Block, nothing happens.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
void avtGadget_customFileFormat::Block::get_data(std::istream& stream, 
                                                 float* data,
                                                 unsigned int parttype)
{
    if(_mesh[parttype]){
        stream.seekg(_streampos[parttype]);
        stream.read(reinterpret_cast<char*>(data), _streamsize[parttype]);
    } else {
        // ERROR
    }
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::get_blocks
//
//  This function returns a vector containing the Blocks stored in the snapshot.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
std::vector<avtGadget_customFileFormat::Block*> 
avtGadget_customFileFormat::get_blocks(std::istream& stream)
{
    // make sure the stream is at its beginning
    stream.seekg(0);
    std::vector<Block*> blocks;
    while(stream.peek() != EOF){
        blocks.push_back(new Block(stream, _npart));
    }
    
    return blocks;
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::read_gadget_head
//
//  This method reads the header of the snapshot file and stores some useful
//  data in the provided variables.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
void
avtGadget_customFileFormat::read_gadget_head(unsigned int* npart,
                                             double* massarr,
                                             double* time,
                                             double* redshift,
                                             std::istream& stream)
{
    unsigned int blocksize = get_blocksize(stream);
    char name[4];
    stream.read(name, 4);
    unsigned int headersize = get_blocksize(stream);
    blocksize -= get_blocksize(stream);
    if(blocksize){
        // throw exception
    }
    blocksize = get_blocksize(stream);
    stream.read(reinterpret_cast<char*>(npart), 6*sizeof(int));
    stream.read(reinterpret_cast<char*>(massarr), 6*sizeof(double));
    stream.read(reinterpret_cast<char*>(time), sizeof(double));
    stream.read(reinterpret_cast<char*>(redshift), sizeof(double));
    char rest[168];
    stream.read(rest, 168);
    blocksize -= get_blocksize(stream);
    if(blocksize){
        // throw exception
    }
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat constructor
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
avtGadget_customFileFormat::avtGadget_customFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), _fname(filename)
{
    std::ifstream ifile(filename);
    if(!ifile.good())
    {
        EXCEPTION1(InvalidDBTypeException, "Can't open file\n");
    }  
    else
    {
        read_gadget_head(_npart, _masstab, &_time, &_redshift, ifile);
    }
}


// ****************************************************************************
//  Method: avtGadget_customFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
void
avtGadget_customFileFormat::FreeUpResources(void)
{
    for(unsigned int i = 0; i < _blocks.size(); i++){
        delete _blocks[i];
    }
}


// ****************************************************************************
//  Method: avtGadget_customFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
void
avtGadget_customFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ifstream ifile(_fname.c_str());
    if(!ifile)
    {
        EXCEPTION1(InvalidDBTypeException,"Can't open file\n");
    }
    
    for(unsigned int i = 0; i < 6; i++){
        if(_npart[i]){
            avtMeshMetaData *mmd = new avtMeshMetaData;
            std::stringstream datalabel;
            datalabel << "position" << i;
            mmd->name = datalabel.str();
            mmd->spatialDimension = 3;
            mmd->topologicalDimension = 0;
            mmd->meshType = AVT_POINT_MESH;
            mmd->numBlocks = 1;
            md->Add(mmd);
        }
    }
    
    _blocks = get_blocks(ifile);
    
    for(unsigned int i = 0; i < _blocks.size(); i++){
        if(_blocks[i]->get_name() != "POS "
           && _blocks[i]->get_name() != "HEAD"){
            for(unsigned int j = 0; j < 6; j++){
                if(_blocks[i]->has_data(j)){
                    std::stringstream label;
                    label << _blocks[i]->get_name() << j;
                    std::stringstream meshname;
                    meshname << "position" << j;
                    if(_blocks[i]->is_vec()){
                        avtVectorMetaData *smd = new avtVectorMetaData;
                        smd->name = label.str();
                        smd->meshName = meshname.str();
                        smd->centering = AVT_ZONECENT;
                        md->Add(smd);
                    } else {
                        avtScalarMetaData *smd = new avtScalarMetaData;
                        smd->name = label.str();
                        smd->meshName = meshname.str();
                        smd->centering = AVT_ZONECENT;
                        md->Add(smd);
                    }
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGadget_customFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
vtkDataSet *
avtGadget_customFileFormat::GetMesh(const char *meshname)
{
    unsigned int i = 0;
    while(i < _blocks.size() && _blocks[i]->get_name() != "POS "){
        i++;
    }
    // -'0' to convert from char to int
    unsigned int parttype = meshname[strlen(meshname)-1] - '0';
    
    std::ifstream ifile(_fname.c_str());
    if(!ifile){
        // ERROR
    }
    vtkPoints* points = vtkPoints::New();
    points->SetNumberOfPoints(_npart[parttype]);
    float* pts = (float*) points->GetVoidPointer(0);
    _blocks[i]->get_data(ifile, pts, parttype);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    ugrid->Allocate(_npart[parttype]);
    vtkIdType onevertex;
    for(int i = 0; i < _npart[parttype]; ++i)
    {
        onevertex = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }
    return ugrid;
}


// ****************************************************************************
//  Method: avtGadget_customFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
vtkDataArray *
avtGadget_customFileFormat::GetVar(const char *varname)
{
    // -'0' to convert from char to int
    unsigned int parttype = varname[strlen(varname)-1] - '0';
    
    unsigned int i = 0;
    while(i < _blocks.size()
          && _blocks[i]->get_name() != string(varname, strlen(varname)-1)){
        i++;
    }
    if(i == _blocks.size()){
        cerr << "ERROR: block not found!" << endl;
    }
    
    std::ifstream ifile(_fname.c_str());
    vtkDataArray* rv = vtkIntArray::New();
    rv->SetNumberOfTuples(_npart[parttype]);
    float* data = (float*) rv->GetVoidPointer(0);
    _blocks[i]->get_data(ifile, data, parttype);
    
    return rv;
}


// ****************************************************************************
//  Method: avtGadget_customFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Bert Vandenbroucke -- generated by xml2avt
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
vtkDataArray *
avtGadget_customFileFormat::GetVectorVar(const char *varname)
{
    // -'0' to convert from char to int
    unsigned int parttype = varname[strlen(varname)-1] - '0';
    
    unsigned int i = 0;
    while(i < _blocks.size()
          && _blocks[i]->get_name() != string(varname, strlen(varname)-1)){
        i++;
    }
    if(i == _blocks.size()){
        cerr << "ERROR: block not found!" << endl;
    }
    
    vtkFloatArray* rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(_npart[parttype]);
    float* pts = (float*) rv->GetVoidPointer(0);
    
    std::ifstream ifile(_fname.c_str());
    _blocks[i]->get_data(ifile, pts, parttype);
    
    return rv;
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::GetCycle
//
//  This function returns a cycle number that is displayed in the timeslider
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
int avtGadget_customFileFormat::GetCycle(void)
{
    return GetCycleFromFilename(_fname.c_str());
}

// ****************************************************************************
//  Method: avtGadget_customFileFormat::GetTime
//
//  This function returns a time value that is shown in the plots
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 18 11:19:21 PDT 2014
//
// ****************************************************************************
double avtGadget_customFileFormat::GetTime(void)
{
    return _time;
}

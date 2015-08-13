/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
//                            avtSWIZMOFileFormat.C                          //
// ************************************************************************* //

#include <avtSWIZMOFileFormat.h>

#include <string>
#include <vector>
#include <sstream>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkVertex.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#include <hdf5.h>
#include <visit-hdf5.h>


using     std::string;
using     std::stringstream;


// ****************************************************************************
//  Method: avtSWIZMOFileFormat constructor
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  For the moment, we only store the filename. The file is opened when data
//  is requested.
//
// ****************************************************************************

avtSWIZMOFileFormat::avtSWIZMOFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    _filename = filename;
    _ndim = 0;
}

// ****************************************************************************
//  Method: avtSWIZMOFileFormat::GetCycle
//
//  Purpose:
//      Get the cycle associated with the file
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 19 15:47:10 CET 2013
//
//  We extract the cycle from the filename.
//
// ****************************************************************************

int
avtSWIZMOFileFormat::GetCycle(void)
{
    return GetCycleFromFilename(_filename);
}

// ****************************************************************************
//  Method: avtSWIZMOFileFormat::GetTime
//
//  Purpose:
//      Get the time associated with the file
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Tue Nov 19 15:44:44 CET 2013
//
//  We open the file and read the time. It's easy as that.
//
// ****************************************************************************

double
avtSWIZMOFileFormat::GetTime(void)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    double time;
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "Time", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_DOUBLE, &time);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    status = H5Fclose(file);
    
    return time;
}

// ****************************************************************************
//  Method: avtSWIZMOFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: bwvdnbro -- generated by xml2avt
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
// ****************************************************************************

void
avtSWIZMOFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSWIZMOFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  Since we are only interested in the hydrodynamical variables (density,
//  velocity, pressure), the metadata are currently hardcoded.
//
// ****************************************************************************

void
avtSWIZMOFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    herr_t status;
    
    _ndim = 3;
    
    for(unsigned int ip = 0; ip < 6; ip++){
        stringstream groupname;
        groupname << "PartType" << ip;
        htri_t exists = H5Lexists(file, groupname.str().c_str(), H5P_DEFAULT);
        if(exists){
            hid_t group = H5Gopen(file, groupname.str().c_str(), H5P_DEFAULT);
            
            stringstream meshname;
            meshname << groupname.str() << "/Coordinates";
        
            DataSetList ds(group);
            
            status = H5Gclose(group);

            avtMeshMetaData *mmd = new avtMeshMetaData;
            mmd->name = meshname.str();
            mmd->spatialDimension = _ndim;
            mmd->topologicalDimension = 0;
            mmd->meshType = AVT_POINT_MESH;
            mmd->numBlocks = 1;
            md->Add(mmd);
            
            std::vector<std::string> scalars = ds.get_scalars();
            for(unsigned int i = 0; i < scalars.size(); i++){
                avtScalarMetaData *scalar = new avtScalarMetaData;
                stringstream scalarname;
                scalarname << groupname.str() << "/" << scalars[i];
                scalar->name = scalarname.str();
                scalar->meshName = meshname.str();
                scalar->centering = AVT_NODECENT;
                scalar->hasUnits = false;
                md->Add(scalar);
            }
            
            std::vector<std::string> vectors = ds.get_vectors();
            for(unsigned int i = 0; i < vectors.size(); i++){
                avtVectorMetaData *vector = new avtVectorMetaData;
                stringstream vectorname;
                vectorname << groupname.str() << "/" << vectors[i];
                vector->name = vectorname.str();
                vector->meshName = meshname.str();
                vector->centering = AVT_NODECENT;
                vector->hasUnits = false;
                vector->varDim = 3;
                md->Add(vector);
            }
            
            std::vector<std::string> tensors = ds.get_tensors();
            for(unsigned int i = 0; i < tensors.size(); i++){
                for(unsigned int j = 0; j < 3; j++){
                    avtVectorMetaData *vector = new avtVectorMetaData;
                    std::stringstream name;
                    name << groupname.str() << "/" << tensors[i] << j;
                    vector->name = name.str();
                    vector->meshName = meshname.str();
                    vector->centering = AVT_NODECENT;
                    vector->hasUnits = false;
                    vector->varDim = 3;
                    md->Add(vector);
                }
            }
        }
    }
    status = H5Fclose(file);
}


// ****************************************************************************
//  Method: avtSWIZMOFileFormat::GetMesh
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
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  There is only one mesh, so the argument is ignored.
//  We first read in the number of particles from the file and then the grid
//  coordinates. VTK uses float types, while the file contains doubles. We
//  read in doubles and convert them to floats when making the VTK grid.
//
// ****************************************************************************

vtkDataSet *
avtSWIZMOFileFormat::GetMesh(const char *meshname)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    unsigned int npartread[6];
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "NumPart_ThisFile", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    
    unsigned int ip = get_particle_type(meshname);
    
    unsigned int npart;
    npart = npartread[ip];
    double *coords = new double[npart*3];
    
    stringstream groupname;
    groupname << "PartType" << ip;
    
    group = H5Gopen(file, groupname.str().c_str(), H5P_DEFAULT);
    
    hid_t dataset = H5Dopen(file, meshname, H5P_DEFAULT);
    status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords);
    status = H5Dclose(dataset);
    status = H5Gclose(group);
    status = H5Fclose(file);
    
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(npart);
    float *pts = (float *) points->GetVoidPointer(0);
    double *xc = &coords[0];
    for(int i = 0; i < npart; ++i){
        *pts++ = (float) *xc++;
        *pts++ = (float) *xc++;
        *pts++ = (float) *xc++;
    }
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    ugrid->Allocate(npart);
    vtkIdType onevertex;
    for(int i = 0; i < npart; ++i){
        onevertex = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }
    
    delete [] coords;

    return ugrid;
}


// ****************************************************************************
//  Method: avtSWIZMOFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  Same as for the grid: we first read in the number of particles and then
//  access the data. In this case, we do use the varname argument.
//  The same applies as for the grid: we read in doubles and convert them to
//  floats.
//
// ****************************************************************************

vtkDataArray *
avtSWIZMOFileFormat::GetVar(const char *varname)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    unsigned int npartread[6];
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "NumPart_ThisFile", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    
    unsigned int ip = get_particle_type(varname);
    
    unsigned int npart = npartread[ip];

    float *darray = new float[npart];
    
    stringstream groupname;
    groupname << "/PartType" << ip;
    
    group = H5Gopen(file, groupname.str().c_str(), H5P_DEFAULT);
    hid_t dataset = H5Dopen(file, varname, H5P_DEFAULT);
    status = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, darray);
    status = H5Dclose(dataset);
    status = H5Gclose(group);
    
    status = H5Fclose(file);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(npart);
    float *data = (float*)arr->GetVoidPointer(0);
    float *dc = &darray[0];
    for(unsigned int i = npart; i--;){
        *data++ = *dc++;
    }
    
    delete [] darray;

    return arr;
}


// ****************************************************************************
//  Method: avtSWIZMOFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  Details are the same as the previous two functions.
//  Although we currently have only one vector variable, we do use the varname.
//
// ****************************************************************************

vtkDataArray *
avtSWIZMOFileFormat::GetVectorVar(const char *varname)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    unsigned int npartread[6];
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "NumPart_ThisFile", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    
    unsigned int ip = get_particle_type(varname);
    
    unsigned int npart = npartread[ip];
    
    float *darray;
    
    stringstream groupname;
    groupname << "/PartType" << ip;
    
    group = H5Gopen(file, groupname.str().c_str(), H5P_DEFAULT);
    std::stringstream dname;
    int index = -1;
    if(isdigit(varname[strlen(varname)-1])){
        index = varname[strlen(varname)-1] - '0';
        dname << string(varname, strlen(varname)-1);
        darray = new float[npart*9];
    } else {
        dname << varname;
        darray = new float[npart*3];
    }
    hid_t dataset = H5Dopen(file, dname.str().c_str(), H5P_DEFAULT);
    status = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, darray);
    status = H5Dclose(dataset);
    status = H5Gclose(group);
    
    status = H5Fclose(file);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(npart);
    float *data = (float*)arr->GetVoidPointer(0);
    float *dx = &darray[0];
    if(index < 0){
        for(unsigned int i = npart; i--;){
            *data++ = (float) *dx++;
            *data++ = (float) *dx++;
            *data++ = (float) *dx++;
        }
    } else {
        for(unsigned int i = npart; i--;){
            if(index > 0){
                dx += index*3;
            }
            *data++ = (float) *dx++;
            *data++ = (float) *dx++;
            *data++ = (float) *dx++;
            if(index < 2){
                dx += (2-index)*3;
            }
        }
    }
    
    delete [] darray;
    
    return arr;
}

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
//                            avtShadowfaxFileFormat.C                           //
// ************************************************************************* //

#include <avtShadowfaxFileFormat.h>

#include <string>
#include <vector>

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


// ****************************************************************************
//  Method: avtShadowfaxFileFormat constructor
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
//  For the moment, we only store the filename. The file is opened when data
//  is requested.
//
// ****************************************************************************

avtShadowfaxFileFormat::avtShadowfaxFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    _filename = filename;
    _ndim = 0;
}

// ****************************************************************************
//  Method: avtShadowfaxFileFormat::GetCycle
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
avtShadowfaxFileFormat::GetCycle(void)
{
    return GetCycleFromFilename(_filename);
}

// ****************************************************************************
//  Method: avtShadowfaxFileFormat::GetTime
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
avtShadowfaxFileFormat::GetTime(void)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    double time;
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "time", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_DOUBLE, &time);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    status = H5Fclose(file);
    
    return time;
}

// ****************************************************************************
//  Method: avtShadowfaxFileFormat::FreeUpResources
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
avtShadowfaxFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtShadowfaxFileFormat::PopulateDatabaseMetaData
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
avtShadowfaxFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // get number of dimensions from snapshot file
    // disable HDF5 error printing
    H5E_auto2_t oldfunc;
    void* old_client_data;
    H5Eget_auto(H5E_DEFAULT, &oldfunc, &old_client_data);
    H5Eset_auto(H5E_DEFAULT, NULL, NULL);
    
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    herr_t status;
    
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "ndim", H5P_DEFAULT);
    if(attr < 0){
        hid_t cells = H5Gopen(file, "/cells", H5P_DEFAULT);
        hid_t dataset = H5Dopen(file, "/cells/velocity_z", H5P_DEFAULT);
        if(dataset < 0){
            _ndim = 2;
        } else {
            _ndim = 3;
            status = H5Dclose(dataset);
        }
        status = H5Gclose(cells);
    } else {  
        status = H5Aread(attr, H5T_NATIVE_UINT32, &_ndim);
        status = H5Aclose(attr);
    }
    attr = H5Aopen(group, "npart", H5P_DEFAULT);
    unsigned int npart[2];
    status = H5Aread(attr, H5T_NATIVE_UINT32, &npart);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    status = H5Fclose(file);
    
    H5Eset_auto(H5E_DEFAULT, oldfunc, old_client_data);

    if(npart[0]){
	      avtMeshMetaData *mmd = new avtMeshMetaData;
	      mmd->name = "mesh_generators";
	      mmd->spatialDimension = _ndim;
	      mmd->topologicalDimension = 0;
	      mmd->meshType = AVT_POINT_MESH;
	      mmd->numBlocks = 1;
	      md->Add(mmd);
	      
	      avtScalarMetaData *density = new avtScalarMetaData;
	      density->name = "density";
	      density->meshName = "mesh_generators";
	      density->centering = AVT_NODECENT;
	      density->hasUnits = false;
	      md->Add(density);
	      
	      avtVectorMetaData *velocity = new avtVectorMetaData;
	      velocity->name = "velocity_gas";
	      velocity->meshName = "mesh_generators";
	      velocity->centering = AVT_NODECENT;
	      velocity->hasUnits = false;
	      velocity->varDim = _ndim;
	      md->Add(velocity);
	      
	      avtVectorMetaData *acc = new avtVectorMetaData;
	      acc->name = "acceleration_gas";
	      acc->meshName = "mesh_generators";
	      acc->centering = AVT_NODECENT;
	      acc->hasUnits = false;
	      acc->varDim = _ndim;
	      md->Add(acc);
	      
	      avtScalarMetaData *pressure = new avtScalarMetaData;
	      pressure->name = "pressure";
	      pressure->meshName = "mesh_generators";
	      pressure->centering = AVT_NODECENT;
	      pressure->hasUnits = false;
	      md->Add(pressure);
	      
        Expression *radius = new Expression;
        radius->SetName("radius_gas");
        radius->SetDefinition("sqrt((coord(mesh_generators)[0]*1.e-10)*(coord(mesh_generators)[0]*1.e-10)+" \
                                   "(coord(mesh_generators)[1]*1.e-10)*(coord(mesh_generators)[1]*1.e-10)+" \
                                   "(coord(mesh_generators)[2]*1.e-10)*(coord(mesh_generators)[2]*1.e-10))");
        radius->SetType(Expression::ScalarMeshVar);
        radius->SetHidden(false);
        md->AddExpression(radius);
    }
    
    if(npart[1]){
        avtMeshMetaData *mmd = new avtMeshMetaData;
	      mmd->name = "dark_matter";
	      mmd->spatialDimension = _ndim;
	      mmd->topologicalDimension = 0;
	      mmd->meshType = AVT_POINT_MESH;
	      mmd->numBlocks = 1;
	      md->Add(mmd);
	      
	      avtVectorMetaData *velocity = new avtVectorMetaData;
	      velocity->name = "velocity_dm";
	      velocity->meshName = "dark_matter";
	      velocity->centering = AVT_NODECENT;
	      velocity->hasUnits = false;
	      velocity->varDim = _ndim;
	      md->Add(velocity);
	      
        Expression *radius = new Expression;
        radius->SetName("radius_dm");
        radius->SetDefinition("sqrt((coord(dark_matter)[0]*1.e-10)*(coord(dark_matter)[0]*1.e-10)+" \
                                   "(coord(dark_matter)[1]*1.e-10)*(coord(dark_matter)[1]*1.e-10)+" \
                                   "(coord(dark_matter)[2]*1.e-10)*(coord(dark_matter)[2]*1.e-10))");
        radius->SetType(Expression::ScalarMeshVar);
        radius->SetHidden(false);
        md->AddExpression(radius);
    }
}


// ****************************************************************************
//  Method: avtShadowfaxFileFormat::GetMesh
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
avtShadowfaxFileFormat::GetMesh(const char *meshname)
{
    if(_ndim==2){
        hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
        hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
        
        unsigned int npartread[2];
        hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
        hid_t attr = H5Aopen(group, "npart", H5P_DEFAULT);
        herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
        status = H5Aclose(attr);
        status = H5Gclose(group);
        
        unsigned int npart;
        string gridname;
        string gridxname;
        string gridyname;
        if(!strcmp(meshname, "mesh_generators")){
            npart = npartread[0];
            gridname = "/grid";
            gridxname = "/grid/x";
            gridyname = "/grid/y";
        } else {
            npart = npartread[1];
            gridname = "/DM";
            gridxname = "/DM/x";
            gridyname = "/DM/y";
        }
        
        double *xarray = new double[npart];
        double *yarray = new double[npart];
        
        group = H5Gopen(file, gridname.c_str(), H5P_DEFAULT);
        hsize_t dims[1] = {npart};
        hid_t dataset = H5Dopen(file, gridxname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, xarray);
        status = H5Dclose(dataset);
        dataset = H5Dopen(file, gridyname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, yarray);
        status = H5Dclose(dataset);
        status = H5Gclose(group);
        status = H5Fclose(file);
        
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(npart);
        float *pts = (float *) points->GetVoidPointer(0);
        double *xc = &xarray[0];
        double *yc = &yarray[0];
        for(int i = 0; i < npart; ++i){
            *pts++ = (float) *xc++;
            *pts++ = (float) *yc++;
            *pts++ = 0.;
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
        
        delete [] xarray;
        delete [] yarray;
    
        return ugrid;
    } else {
        hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
        hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
        
        unsigned int npartread[2];
        hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
        hid_t attr = H5Aopen(group, "npart", H5P_DEFAULT);
        herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
        status = H5Aclose(attr);
        status = H5Gclose(group);
        
        unsigned int npart;
        string gridname;
        string gridxname;
        string gridyname;
        string gridzname;
        if(!strcmp(meshname, "mesh_generators")){
            npart = npartread[0];
            gridname = "/grid";
            gridxname = "/grid/x";
            gridyname = "/grid/y";
            gridzname = "/grid/z";
        } else {
            npart = npartread[1];
            gridname = "/DM";
            gridxname = "/DM/x";
            gridyname = "/DM/y";
            gridzname = "/DM/z";
        }
        
        double *xarray = new double[npart];
        double *yarray = new double[npart];
        double *zarray = new double[npart];
        
        group = H5Gopen(file, gridname.c_str(), H5P_DEFAULT);
        hsize_t dims[1] = {npart};
        hid_t dataset = H5Dopen(file, gridxname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, xarray);
        status = H5Dclose(dataset);
        dataset = H5Dopen(file, gridyname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, yarray);
        status = H5Dclose(dataset);
        dataset = H5Dopen(file, gridzname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, zarray);
        status = H5Dclose(dataset);
        status = H5Gclose(group);
        status = H5Fclose(file);
        
        vtkPoints *points = vtkPoints::New();
        points->SetNumberOfPoints(npart);
        float *pts = (float *) points->GetVoidPointer(0);
        double *xc = &xarray[0];
        double *yc = &yarray[0];
        double *zc = &zarray[0];
        for(int i = 0; i < npart; ++i){
            *pts++ = (float) *xc++;
            *pts++ = (float) *yc++;
            *pts++ = (float) *zc++;
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
        
        delete [] xarray;
        delete [] yarray;
        delete [] zarray;
    
        return ugrid;
    }
}


// ****************************************************************************
//  Method: avtShadowfaxFileFormat::GetVar
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
avtShadowfaxFileFormat::GetVar(const char *varname)
{
    hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
    hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
    
    unsigned int npartread[2];
    hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
    hid_t attr = H5Aopen(group, "npart", H5P_DEFAULT);
    herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
    status = H5Aclose(attr);
    status = H5Gclose(group);
    
    unsigned int npart = npartread[0];

    double *darray = new double[npart];
    group = H5Gopen(file, "/cells", H5P_DEFAULT);
    hsize_t dims[1] = {npart};
    std::stringstream dname;
    dname << "/cells/" << varname;
    hid_t dataset = H5Dopen(file, dname.str().c_str(), H5P_DEFAULT);
    status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, darray);
    status = H5Dclose(dataset);
    status = H5Gclose(group);
    
    status = H5Fclose(file);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(npart);
    float *data = (float*)arr->GetVoidPointer(0);
    double *dc = &darray[0];
    for(unsigned int i = npart; i--;){
        *data++ = (float) *dc++;
    }
    
    delete [] darray;

    return arr;
}


// ****************************************************************************
//  Method: avtShadowfaxFileFormat::GetVectorVar
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
avtShadowfaxFileFormat::GetVectorVar(const char *varname)
{
    if(_ndim==2){
        hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
        hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
        
        unsigned int npartread[2];
        hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
        hid_t attr = H5Aopen(group, "npart", H5P_DEFAULT);
        herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
        status = H5Aclose(attr);
        status = H5Gclose(group);
        
        unsigned int npart;
        string groupname;
        string groupxname;
        string groupyname;
        if(!strcmp(varname, "velocity_gas")){
            npart = npartread[0];
            groupname = "/cells";
            groupxname = "/cells/velocity_x";
            groupyname = "/cells/velocity_y";
        } else {
            npart = npartread[1];
            groupname = "/DM";
            groupxname = "/DM/vx";
            groupyname = "/DM/vy";
        }
        
        double *dxarray = new double[npart];
        double *dyarray = new double[npart];
        
        group = H5Gopen(file, groupname.c_str(), H5P_DEFAULT);
        hsize_t dims[1] = {npart};
        hid_t dataset = H5Dopen(file, groupxname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dxarray);
        status = H5Dclose(dataset);
        
        dataset = H5Dopen(file, groupyname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dyarray);
        status = H5Dclose(dataset);
        status = H5Gclose(group);
        
        status = H5Fclose(file);

        vtkFloatArray *arr = vtkFloatArray::New();
        // The number of components should always be 3. Not 2, but 3. Always.
        arr->SetNumberOfComponents(3);
        arr->SetNumberOfTuples(npart);
        float *data = (float*)arr->GetVoidPointer(0);
        double *dx = &dxarray[0];
        double *dy = &dyarray[0];
        for(unsigned int i = npart; i--;){
            *data++ = (float) *dx++;
            *data++ = (float) *dy++;
            *data++ = 0.;
        }
        
        delete [] dxarray;
        delete [] dyarray;
        
        return arr;
    } else {
        hid_t flag = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fclose_degree(flag, H5F_CLOSE_SEMI);
        hid_t file= H5Fopen(_filename, H5F_ACC_RDONLY, flag);
        
        unsigned int npartread[2];
        hid_t group = H5Gopen(file, "/Header", H5P_DEFAULT);
        hid_t attr = H5Aopen(group, "npart", H5P_DEFAULT);
        herr_t status = H5Aread(attr, H5T_NATIVE_UINT32, &npartread);
        status = H5Aclose(attr);
        status = H5Gclose(group);
        
        unsigned int npart;
        string groupname;
        string groupxname;
        string groupyname;
        string groupzname;
        if(!strcmp(varname, "velocity_gas")){
            npart = npartread[0];
            groupname = "/cells";
            groupxname = "/cells/velocity_x";
            groupyname = "/cells/velocity_y";
            groupzname = "/cells/velocity_z";
        } else {
            if(!strcmp(varname, "acceleration_gas")){
                npart = npartread[0];
                groupname = "/cells";
                groupxname = "/cells/acceleration_x";
                groupyname = "/cells/acceleration_y";
                groupzname = "/cells/acceleration_z";
            } else {
                npart = npartread[1];
                groupname = "/DM";
                groupxname = "/DM/vx";
                groupyname = "/DM/vy";
                groupzname = "/DM/vz";
            }
        }
        
        double *dxarray = new double[npart];
        double *dyarray = new double[npart];
        double *dzarray = new double[npart];
        
        group = H5Gopen(file, groupname.c_str(), H5P_DEFAULT);
        hsize_t dims[1] = {npart};
        hid_t dataset = H5Dopen(file, groupxname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dxarray);
        status = H5Dclose(dataset);
        
        dataset = H5Dopen(file, groupyname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dyarray);
        status = H5Dclose(dataset);
        
        dataset = H5Dopen(file, groupzname.c_str(), H5P_DEFAULT);
        status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dzarray);
        status = H5Dclose(dataset);
        status = H5Gclose(group);
        
        status = H5Fclose(file);

        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfComponents(3);
        arr->SetNumberOfTuples(npart);
        float *data = (float*)arr->GetVoidPointer(0);
        double *dx = &dxarray[0];
        double *dy = &dyarray[0];
        double *dz = &dzarray[0];
        for(unsigned int i = npart; i--;){
            *data++ = (float) *dx++;
            *data++ = (float) *dy++;
            *data++ = (float) *dz++;
        }
        
        delete [] dxarray;
        delete [] dyarray;
        delete [] dzarray;
        
        return arr;
    }
}

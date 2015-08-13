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
//                            avtSWIZMOFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SWIZMO_FILE_FORMAT_H
#define AVT_SWIZMO_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <string.h>
#include <vector>
#include <hdf5.h>


// ****************************************************************************
//  Class: avtSWIZMOFileFormat
//
//  Purpose:
//      Reads in SWIZMO files as a plugin to VisIt.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
// ****************************************************************************

class avtSWIZMOFileFormat : public avtSTSDFileFormat
{
  private:
    enum DataType{
        SCALAR = 0,
        VECTOR,
        TENSOR,
        UNKNOWN
    };
    
    class DataSet{
    private:
        std::string _name;
        DataType _type;
        
    public:
        DataSet(std::string name, unsigned int size) : _name(name) {
            _type = UNKNOWN;
            if(size == 1){
                _type = SCALAR;
            }
            if(size == 3){
                _type = VECTOR;
            }
            if(size == 9){
                _type = TENSOR;
            }
        }
        
        bool is_vector(){
            return _type == VECTOR;
        }
        
        bool is_scalar(){
            return _type == SCALAR;
        }
        
        bool is_tensor(){
            return _type == TENSOR;
        }
        
        std::string get_name(){
            return _name;
        }
    };
    
    class DataSetList{
    private:
        std::vector<DataSet> _list;
        
    public:
        void add(std::string name, unsigned int size){
            _list.push_back(DataSet(name, size));
        }
        
        std::vector<std::string> get_scalars(){
            std::vector<std::string> scalars;
            for(unsigned int i = 0; i < _list.size(); i++){
                if(_list[i].is_scalar()){
                    scalars.push_back(_list[i].get_name());
                }
            }
            return scalars;
        }
        
        std::vector<std::string> get_vectors(){
            std::vector<std::string> vectors;
            for(unsigned int i = 0; i < _list.size(); i++){
                if(_list[i].is_vector()){
                    if(_list[i].get_name() != "Coordinates"){
                        vectors.push_back(_list[i].get_name());
                    }
                }
            }
            return vectors;
        }
        
        std::vector<std::string> get_tensors(){
            std::vector<std::string> tensors;
            for(unsigned int i = 0; i < _list.size(); i++){
                if(_list[i].is_tensor()){
                    tensors.push_back(_list[i].get_name());
                }
            }
            return tensors;
        }
        
        static herr_t fill_list(hid_t loc_id,
                                const char *name,
                                const H5L_info_t *info,
                                void *op_data){

            DataSetList &list = *((DataSetList*) op_data);
            
            hid_t dataset = H5Dopen(loc_id, name, H5P_DEFAULT);
            hid_t dataspace = H5Dget_space(dataset);
            
            hsize_t size[2];
            hsize_t maxsize[2];
            int ndim = H5Sget_simple_extent_dims(dataspace, size, maxsize);
            
            if(ndim == 1){
                size[1] = ndim;
            }
            
            H5Sclose(dataspace);
            H5Dclose(dataset);
            
            list.add(std::string(name), size[1]);

            return 0;
        }
        
        DataSetList(hid_t group){
            hsize_t it = 0;
            herr_t status = H5Literate(group, H5_INDEX_NAME, H5_ITER_NATIVE,
                                       &it, DataSetList::fill_list, this);
        }
    };
    
    inline unsigned int get_particle_type(const char *dsname){
        return dsname[8]-'0';
    }

  public:
                       avtSWIZMOFileFormat(const char *filename);
    virtual           ~avtSWIZMOFileFormat() {;};
    
    virtual bool          ReturnsValidCycle() const { return true; };
    virtual int           GetCycle(void);
    virtual bool          ReturnsValidTime() const { return true; };
    virtual double        GetTime(void);
    
    virtual int GetCycleFromFilename(const char *f) const
    {
        // we start 3 digits and a .hdf5 extension from the end
        // we then search for the first non-digit char to find the
        // offset of our cycle number
        int index = strlen(f)-8;
        while(isdigit(f[index-1])){
            index--;
        }
        unsigned int i;
        sscanf(&f[index], "%d.hdf5", &i);
        return i;
    }

    virtual const char    *GetType(void)   { return "SWIZMO"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    const char* _filename;
    unsigned int _ndim;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif

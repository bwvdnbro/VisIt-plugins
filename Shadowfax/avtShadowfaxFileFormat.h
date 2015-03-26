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
//                            avtShadowfaxFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Shadowfax_FILE_FORMAT_H
#define AVT_Shadowfax_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <string.h>


// ****************************************************************************
//  Class: avtShadowfaxFileFormat
//
//  Purpose:
//      Reads in Shadowfax files as a plugin to VisIt.
//
//  Programmer: Bert Vandenbroucke
//  Creation:   Mon Nov 18 14:21:23 PST 2013
//
// ****************************************************************************

class avtShadowfaxFileFormat : public avtSTSDFileFormat
{
  public:
                       avtShadowfaxFileFormat(const char *filename);
    virtual           ~avtShadowfaxFileFormat() {;};
    
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

    virtual const char    *GetType(void)   { return "Shadowfax"; };
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

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
//  File: ShadowfaxPluginInfo.C
// ************************************************************************* //

#include <ShadowfaxPluginInfo.h>

#include <visit-config.h>
VISIT_PLUGIN_VERSION(Shadowfax,DBP_EXPORT)

// ****************************************************************************
//  Function:  GetGeneralInfo
//
//  Purpose:
//    Return a new GeneralPluginInfo for the Shadowfax database.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
extern "C" DBP_EXPORT GeneralDatabasePluginInfo* Shadowfax_GetGeneralInfo()
{
    return new ShadowfaxGeneralPluginInfo;
}

// ****************************************************************************
//  Method: ShadowfaxGeneralPluginInfo::GetName
//
//  Purpose:
//    Return the name of the database plugin.
//
//  Returns:    A pointer to the name of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
ShadowfaxGeneralPluginInfo::GetName() const
{
    return "Shadowfax";
}

// ****************************************************************************
//  Method: ShadowfaxGeneralPluginInfo::GetVersion
//
//  Purpose:
//    Return the version of the database plugin.
//
//  Returns:    A pointer to the version of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
ShadowfaxGeneralPluginInfo::GetVersion() const
{
    return "1.0";
}

// ****************************************************************************
//  Method: ShadowfaxGeneralPluginInfo::GetID
//
//  Purpose:
//    Return the id of the database plugin.
//
//  Returns:    A pointer to the id of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
ShadowfaxGeneralPluginInfo::GetID() const
{
    return "Shadowfax_1.0";
}
// ****************************************************************************
//  Method: ShadowfaxGeneralPluginInfo::EnabledByDefault
//
//  Purpose:
//    Return true if this plugin should be enabled by default; false otherwise.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
ShadowfaxGeneralPluginInfo::EnabledByDefault() const
{
    return true;
}
// ****************************************************************************
//  Method: ShadowfaxGeneralPluginInfo::HasWriter
//
//  Purpose:
//    Return true if this plugin has a database writer.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
ShadowfaxGeneralPluginInfo::HasWriter() const
{
    return false;
}
// ****************************************************************************
//  Method:  ShadowfaxGeneralPluginInfo::GetDefaultFilePatterns
//
//  Purpose:
//    Returns the default patterns for a Shadowfax database.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
std::vector<std::string>
ShadowfaxGeneralPluginInfo::GetDefaultFilePatterns() const
{
    std::vector<std::string> defaultPatterns;
    defaultPatterns.push_back("*.hdf");
    defaultPatterns.push_back("*.hdf5");

    return defaultPatterns;
}

// ****************************************************************************
//  Method:  ShadowfaxGeneralPluginInfo::AreDefaultFilePatternsStrict
//
//  Purpose:
//    Returns if the file patterns for a Shadowfax database are
//    intended to be interpreted strictly by default.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
ShadowfaxGeneralPluginInfo::AreDefaultFilePatternsStrict() const
{
    return false;
}

// ****************************************************************************
//  Method:  ShadowfaxGeneralPluginInfo::OpensWholeDirectory
//
//  Purpose:
//    Returns if the Shadowfax plugin opens a whole directory name
//    instead of a single file.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
ShadowfaxGeneralPluginInfo::OpensWholeDirectory() const
{
    return false;
}

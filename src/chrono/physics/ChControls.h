//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010 Alessandro Tasora
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHCONTROLS_H
#define CHCONTROLS_H

//////////////////////////////////////////////////
//
//   ChControls.h
//
//   Class for controls
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include <math.h>

#include "physics/ChObject.h"

namespace chrono {

///
/// Basic interface class for 'controls', that are objects that change parameters
/// during the simulation, ex. to simulate PIDs, etc.
/// Must be inherited and implemented by user.
///

class ChApi ChControls : public ChObj {
    // Chrono simulation of RTTI, needed for serialization
    CH_RTTI(ChControls, ChObj);

  public:
    ChControls(){};
    virtual ~ChControls(){};

    // void Copy(ChControls* source);

    //
    // FUNCTIONS - interface to be implemeted -
    //

    virtual int ExecuteForStart() = 0;
    virtual int ExecuteForUpdate() = 0;
    virtual int ExecuteForStep() = 0;
    virtual int ExecuteFor3DStep() = 0;

    //
    // STREAMING
    //

    /// Method to allow serialization of transient data in archives.
    virtual void ArchiveOUT(ChArchiveOut& marchive) override;

    /// Method to allow de serialization of transient data from archives.
    virtual void ArchiveIN(ChArchiveIn& marchive) override;
};

}  // END_OF_NAMESPACE____

#endif  // END of header

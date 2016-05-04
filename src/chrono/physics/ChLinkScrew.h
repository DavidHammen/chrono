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

#ifndef CHLINKSCREW_H
#define CHLINKSCREW_H

///////////////////////////////////////////////////
//
//   ChLinkScrew.h
//
//
//   Classes for screw joint
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include "physics/ChLinkLock.h"

namespace chrono {
///
/// Screw joint between two rigid bodies. This
/// link type is able to couple translation and rotation.
///

class ChApi ChLinkScrew : public ChLinkLock {
    CH_RTTI(ChLinkScrew, ChLinkLock);

  protected:
    double tau;  // transmission coeff.

  public:
    // builders and destroyers
    ChLinkScrew();
    virtual ~ChLinkScrew();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
    virtual void Copy(ChLinkScrew* source);
#pragma GCC diagnostic pop
    virtual ChLink* new_Duplicate() override;  // always return base link class pointer

    // UPDATING FUNCTIONS - "screw" custom implementations

    // Inherit the link-lock computations like it were a
    // normal "revolute" joint, but then modifies the Z-lock parts of C,
    // Cdt, Cdtdt, [Cq] etc., in order to have z = tau * alpha.
    virtual void UpdateState() override;

    // data get/set
    double Get_tau() { return tau; };
    void Set_tau(double mset) { tau = mset; }
    double Get_thread() { return tau * (2 * CH_C_PI); };
    void Set_thread(double mset) { tau = mset / (2 * CH_C_PI); }

    //
    // SERIALIZATION
    //

    /// Method to allow serialization of transient data to archives.
    virtual void ArchiveOUT(ChArchiveOut& marchive) const override;

    /// Method to allow deserialization of transient data from archives.
    virtual void ArchiveIN(ChArchiveIn& marchive) override;
};

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

}  // END_OF_NAMESPACE____

#endif

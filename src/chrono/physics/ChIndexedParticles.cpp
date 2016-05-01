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

///////////////////////////////////////////////////
//
//   ChParticles.cpp
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include <stdlib.h>
#include <algorithm>

#include "core/ChTransform.h"
#include "physics/ChIndexedParticles.h"
#include "physics/ChSystem.h"
#include "physics/ChGlobal.h"

#include "physics/ChExternalObject.h"
#include "core/ChLinearAlgebra.h"

namespace chrono {

using namespace collision;
using namespace geometry;

// Register into the object factory, to enable run-time
// dynamic creation and persistence
ChClassRegisterABSTRACT<ChIndexedParticles> a_registration_ChIndexedParticles;

//////////////////////////////////////
//////////////////////////////////////

/// CLASS FOR A PARTICLE

ChParticleBase::ChParticleBase() {
}

ChParticleBase::~ChParticleBase() {
}

ChParticleBase::ChParticleBase(const ChParticleBase& other) : ChFrameMoving<double>(other) {
}

ChParticleBase& ChParticleBase::operator=(const ChParticleBase& other) {
    if (&other == this)
        return *this;

    // parent class copy
    ChFrameMoving<double>::operator=(other);

    return *this;
}

//////////////////////////////////////
//////////////////////////////////////

/// CLASS FOR PARTICLE CLUSTER

ChIndexedParticles::ChIndexedParticles() {
}

ChIndexedParticles::~ChIndexedParticles() {
}

//////// FILE I/O

void ChIndexedParticles::ArchiveOUT(ChArchiveOut& marchive) {

    // class version number
    marchive.VersionWrite(1);

    // serialize parent class too
    ChPhysicsItem::ArchiveOUT(marchive);

    // stream out all member data
}

void ChIndexedParticles::ArchiveIN(ChArchiveIn& marchive) {

    // class version number
    // int version =
    marchive.VersionRead();

    // deserialize parent class too
    ChPhysicsItem::ArchiveIN(marchive);

    // stream in all member data
}

}  // END_OF_NAMESPACE____

/////////////////////

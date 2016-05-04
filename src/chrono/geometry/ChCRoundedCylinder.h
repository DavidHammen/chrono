//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2012 Alessandro Tasora
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHC_ROUNDEDCYLINDER_H
#define CHC_ROUNDEDCYLINDER_H


#include "ChCGeometry.h"

namespace chrono {
namespace geometry {

#define CH_GEOCLASS_ROUNDEDCYLINDER 15

///
/// A capsule geometric object for collision, visualization, etc.
///

class ChApi ChRoundedCylinder : public ChGeometry {
    // Chrono simulation of RTTI, needed for serialization
    CH_RTTI(ChRoundedCylinder, ChGeometry);

  public:
    //
    // CONSTRUCTORS
    //

    ChRoundedCylinder() {
        center = ChVector<>(0, 0, 0);
        rad = 0;
        hlen = 0;
        radsphere = 0;
    };

    ChRoundedCylinder(ChVector<>& mcenter, double mrad, double mhlen, double mradsphere) {
        center = mcenter;
        rad = mrad;
        hlen = mhlen;
        radsphere = mradsphere;
    }

    ChRoundedCylinder(const ChRoundedCylinder& source) { Copy(&source); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
    void Copy(const ChRoundedCylinder* source) {
        center = source->center;
        rad = source->rad;
        hlen = source->hlen;
        radsphere = source->radsphere;
    }
#pragma GCC diagnostic pop

    ChGeometry* Duplicate() override {
        ChGeometry* mgeo = new ChRoundedCylinder();
        mgeo->Copy(this);
        return mgeo;
    }

    //
    // OVERRIDE BASE CLASS FUNCTIONS
    //

    virtual int GetClassType() override { return CH_GEOCLASS_ROUNDEDCYLINDER; }

    virtual void GetBoundingBox(double& xmin,
                                double& xmax,
                                double& ymin,
                                double& ymax,
                                double& zmin,
                                double& zmax,
                                ChMatrix33<>* Rot = NULL) override {
        //***TO DO*** Implement Bounding Box
    }

    virtual ChVector<> Baricenter() override { return center; }

    //***TO DO***  obsolete/unused
    virtual void CovarianceMatrix(ChMatrix33<>& C) override {
        C.Reset();
        C(0, 0) = center.x * center.x;
        C(1, 1) = center.y * center.y;
        C(2, 2) = center.z * center.z;
    }

    /// This is a solid
    virtual int GetManifoldDimension() override { return 3; }

    //
    // DATA
    //

    ChVector<> center;
    double rad;
    double hlen;
    /// Radius of sweeping sphere
    double radsphere;

    //
    // SERIALIZATION
    //

    virtual void ArchiveOUT(ChArchiveOut& marchive) const override
    {
        // version number
        marchive.VersionWrite(1);
        // serialize parent class
        ChGeometry::ArchiveOUT(marchive);
        // serialize all member data:
        marchive << CHNVP_OUT(center);
        marchive << CHNVP_OUT(rad);
        marchive << CHNVP_OUT(hlen);
        marchive << CHNVP_OUT(radsphere);
    }

    /// Method to allow de serialization of transient data from archives.
    virtual void ArchiveIN(ChArchiveIn& marchive) override
    {
        // version number
        // int version =
        marchive.VersionRead();
        // deserialize parent class
        ChGeometry::ArchiveIN(marchive);
        // stream in all member data:
        marchive >> CHNVP_IN(center);
        marchive >> CHNVP_IN(rad);
        marchive >> CHNVP_IN(hlen);
        marchive >> CHNVP_IN(radsphere);
    }


};

}  // END_OF_NAMESPACE____
}  // END_OF_NAMESPACE____

#endif

//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010, 2012 Alessandro Tasora
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHC_BOX_H
#define CHC_BOX_H


#include <math.h>

#include "geometry/ChCGeometry.h"

namespace chrono {
namespace geometry {

#define CH_GEOCLASS_BOX 3

///
/// A box.
/// Geometric object for collisions and such.
///

class ChApi ChBox : public ChGeometry {
    // Chrono simulation of RTTI, needed for serialization
    CH_RTTI(ChBox, ChGeometry);

  public:
    //
    // CONSTRUCTORS
    //

    ChBox() {
        Pos = VNULL;
        Size = VNULL;
        Rot.Set33Identity();
    };

    /// Build from pos, rotation, xyzlengths
    ChBox(const ChVector<>& mpos, const ChMatrix33<>& mrot, const ChVector<>& mlengths) {
        Pos = mpos;
        Size = 0.5 * mlengths;
        Rot.CopyFromMatrix(mrot);
    }

    /// Build from first corner and three other neighbouring corners
    ChBox(ChVector<>& mC0, ChVector<>& mC1, ChVector<>& mC2, ChVector<>& mC3);

    ChBox(ChBox& source) { Copy(&source); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
    void Copy(ChBox* source) {
        Pos = source->Pos;
        Size = source->Size;
        Rot.CopyFromMatrix(*source->GetRotm());
    }
#pragma GCC diagnostic pop

    ChGeometry* Duplicate() override {
        ChGeometry* mgeo = new ChBox();
        mgeo->Copy(this);
        return mgeo;
    };

    //
    // OVERRIDE BASE CLASS FUNCTIONS
    //

    virtual int GetClassType() override { return CH_GEOCLASS_BOX; };

    virtual void GetBoundingBox(double& xmin,
                                double& xmax,
                                double& ymin,
                                double& ymax,
                                double& zmin,
                                double& zmax,
                                ChMatrix33<>* bbRot = NULL) override;

    /// Computes the baricenter of the box
    virtual Vector Baricenter() override { return Pos; };

    /// Computes the covariance matrix for the box
    virtual void CovarianceMatrix(ChMatrix33<>& C) override;

    /// Evaluate position in cube volume
    virtual void Evaluate(Vector& pos, const double parU, const double parV = 0., const double parW = 0.) override;

    /// This is a solid
    virtual int GetManifoldDimension() override { return 3; }

    //
    // CUSTOM FUNCTIONS
    //

    /// Access the rotation of the box
    ChMatrix33<>* GetRotm() { return &Rot; };

    /// Access the rotation of the box
    ChMatrix33<> GetRotm() const { return Rot; };

    /// Access the position of the barycenter of the box
    ChVector<>& GetPos() { return Pos; };

    /// Access the position of the barycenter of the box
    ChVector<> GetPos() const { return Pos; };

    /// Access the size of the box: a vector with the
    /// three hemi-lengths (lengths divided by two!)
    ChVector<>& GetSize() { return Size; };

    /// Access the size of the box: a vector with the
    /// three hemi-lengths (lengths divided by two!)
    ChVector<> GetSize() const { return Size; };

    /// Get the x y z lengths of this box (that is, double
    /// the Size values)
    ChVector<> GetLengths() const { return 2.0 * Size; }

    /// Set the x y z lengths of this box (that is, double
    /// the Size values)
    void SetLengths(const ChVector<>& mlen) { Size = 0.5 * mlen; }

    // Get the 8 corner points, translated and rotated
    ChVector<> GetP1() const;
    ChVector<> GetP2() const;
    ChVector<> GetP3() const;
    ChVector<> GetP4() const;
    ChVector<> GetP5() const;
    ChVector<> GetP6() const;
    ChVector<> GetP7() const;
    ChVector<> GetP8() const;
    /// Get the n-th corner point, with ipoint = 1...8
    ChVector<> GetPn(int ipoint) const;

    /// Get the volume (assuming no scaling in Rot matrix)
    double GetVolume() const { return Size.x * Size.y * Size.z * 8.0; };

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
        marchive << CHNVP_OUT(Pos);
        marchive << CHNVP_OUT(Rot);
        ChVector<> Lengths = GetLengths();
        marchive << CHNVP_OUT(Lengths); // avoid storing 'Size', i.e. half lenths, because less intuitive
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
        marchive >> CHNVP_IN(Pos);
        marchive >> CHNVP_IN(Rot);
        ChVector<> Lengths;
        marchive >> CHNVP_IN(Lengths); // avoid storing 'Size', i.e. half lenths, because less intuitive
        SetLengths(Lengths);
    }


    //
    // DATA
    //

    /// Rotation of box
    ChMatrix33<> Rot;
    /// Position of center
    ChVector<> Pos;
    /// Hemi size (extension of box from center to corner)
    ChVector<> Size;
};

}  // END_OF_NAMESPACE____
}  // END_OF_NAMESPACE____

#endif

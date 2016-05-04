//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010-2011 Alessandro Tasora
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHCONVEYOR_H
#define CHCONVEYOR_H

//////////////////////////////////////////////////
//
//   ChConveyor.h
//
//   Class for conveyor belt (approximated by
//   parallelepiped with moving surface)
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include <math.h>

#include "physics/ChBody.h"
#include "physics/ChLinkLock.h"

namespace chrono {

///
/// Class for conveyor belt.
/// A conveyor belt is approximated by a box collision
/// shape, where the upper surface has continuous motion
/// in X direction. No cylindrical rounding is used at the ends.
///

class ChApi ChConveyor : public ChPhysicsItem {
    CH_RTTI(ChConveyor, ChPhysicsItem);

  private:
    //
    // DATA
    //

    double conveyor_speed;  // speed of conveyor, along the X direction of the box.

    // link between this body and conveyor plate
    ChLinkLockLock* internal_link;

    // used for the conveyor truss
    ChBody* conveyor_truss;
    // used for the conveyor plate
    ChBody* conveyor_plate;

  public:
    //
    // CONSTRUCTORS
    //

    /// Build a conveyor belt, with motion along x axis
    ChConveyor(double xlength = 1, double ythick = 0.1, double zwidth = 0.5);
    /// Destructor
    ~ChConveyor();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
    /// Copy from another ChChConveyor.
    void Copy(ChConveyor* source);
#pragma GCC diagnostic pop

    /// Set the pointer to the parent ChSystem() and 
    /// also add to new collision system / remove from old coll.system
    virtual void SetSystem(ChSystem* m_system) override;

    //
    // FUNCTIONS
    //

    /// Set the speed of the conveyor belt (upper part, X direction)
    void SetConveyorSpeed(double mspeed) { conveyor_speed = mspeed; }
    /// Get the speed of the conveyor belt (upper part, X direction)
    double GetConveyorSpeed() { return conveyor_speed; }

    /// Access the internal body used as the truss of the moving belt 
    ChBody* GetTruss() { return conveyor_truss; }

    /// Access the internal body used as the moving belt (a plate with const.vel.)
    ChBody* GetPlate() { return conveyor_plate; }

    //
    // Shortcuts for ChBody-like transformations etc. 
    // (these, and others, can be done also as my_conveyor->GetTruss()->ChBlabla(...) )
    //

    void SetBodyFixed(bool mev) { this->GetTruss()->SetBodyFixed(mev);}
    bool GetBodyFixed() { return this->GetTruss()->GetBodyFixed(); }

    ChCoordsys<>& GetCoord() { return this->GetTruss()->GetCoord(); }
    ChVector<>& GetPos()  { return this->GetTruss()->GetPos(); }
    ChQuaternion<>& GetRot() { return this->GetTruss()->GetRot(); }
    void SetCoord(const ChCoordsys<>& mcoord) { return this->GetTruss()->SetCoord(mcoord); }
    void SetCoord(const ChVector<>& mv, const ChQuaternion<>& mq) { this->GetTruss()->SetCoord(mv,mq);  }
    void SetRot(const ChQuaternion<>& mrot) { this->GetTruss()->SetRot(mrot); }
    void SetPos(const ChVector<>& mpos) { this->GetTruss()->SetPos(mpos);}

    /// Access the material surface properties of the conveyor belt (shortcut)
    std::shared_ptr<ChMaterialSurfaceBase>& GetMaterialSurfaceBase() { return this->GetPlate()->GetMaterialSurfaceBase(); }

    /// Access the DVI material surface properties of the conveyor belt (shortcut)
    std::shared_ptr<ChMaterialSurface> GetMaterialSurface() { return this->GetPlate()->GetMaterialSurface(); }

    /// Set the material surface properties by passing a ChMaterialSurface or ChMaterialSurfaceDEM object.
    void SetMaterialSurface(const std::shared_ptr<ChMaterialSurfaceBase>& mnewsurf) { this->GetPlate()->SetMaterialSurface(mnewsurf); }

    //
    // STATE FUNCTIONS
    //

    /// Number of coordinates: this contains an auxiliary body, so it is 14 (with quaternions for rotations)
    virtual int GetDOF() override { return 7 + 7; }
    /// Number of coordinates of the particle cluster, 2x6 because derivatives es. angular vel.
    virtual int GetDOF_w() override { return 6 + 6; }
    /// Get the number of scalar constraints. In this case, a lock constraint is embedded, so:
    virtual int GetDOC_c() override { return 6; }

    // (override/implement interfaces for global state vectors, see ChPhysicsItem for comments.)
    virtual void IntStateGather(const unsigned int off_x,
                                ChState& x,
                                const unsigned int off_v,
                                ChStateDelta& v,
                                double& T) override;
    virtual void IntStateScatter(const unsigned int off_x,
                                 const ChState& x,
                                 const unsigned int off_v,
                                 const ChStateDelta& v,
                                 const double T) override;
    virtual void IntStateGatherAcceleration(const unsigned int off_a, ChStateDelta& a) override;
    virtual void IntStateScatterAcceleration(const unsigned int off_a, const ChStateDelta& a) override;
    virtual void IntStateGatherReactions(const unsigned int off_L, ChVectorDynamic<>& L) override;
    virtual void IntStateScatterReactions(const unsigned int off_L, const ChVectorDynamic<>& L) override;
    virtual void IntStateIncrement(const unsigned int off_x,
                                   ChState& x_new,
                                   const ChState& x,
                                   const unsigned int off_v,
                                   const ChStateDelta& Dv) override;
    virtual void IntLoadResidual_F(const unsigned int off, ChVectorDynamic<>& R, const double c) override;
    virtual void IntLoadResidual_Mv(const unsigned int off,
                                    ChVectorDynamic<>& R,
                                    const ChVectorDynamic<>& w,
                                    const double c) override;
    virtual void IntLoadResidual_CqL(const unsigned int off_L,
                                     ChVectorDynamic<>& R,
                                     const ChVectorDynamic<>& L,
                                     const double c) override;
    virtual void IntLoadConstraint_C(const unsigned int off,
                                     ChVectorDynamic<>& Qc,
                                     const double c,
                                     bool do_clamp,
                                     double recovery_clamp) override;
    virtual void IntLoadConstraint_Ct(const unsigned int off, ChVectorDynamic<>& Qc, const double c) override;
    virtual void IntToLCP(const unsigned int off_v,
                          const ChStateDelta& v,
                          const ChVectorDynamic<>& R,
                          const unsigned int off_L,
                          const ChVectorDynamic<>& L,
                          const ChVectorDynamic<>& Qc) override;
    virtual void IntFromLCP(const unsigned int off_v, ChStateDelta& v, const unsigned int off_L, ChVectorDynamic<>& L) override;

    // Override/implement LCP system functions of ChPhysicsItem
    // (to assembly/manage data for LCP system solver)

    virtual void InjectVariables(ChLcpSystemDescriptor& mdescriptor) override;
    virtual void VariablesFbReset() override;
    virtual void VariablesFbLoadForces(double factor = 1.) override;
    virtual void VariablesQbLoadSpeed() override;
    virtual void VariablesFbIncrementMq() override;
    virtual void VariablesQbSetSpeed(double step = 0.) override;
    virtual void VariablesQbIncrementPosition(double step) override;

    virtual void InjectConstraints(ChLcpSystemDescriptor& mdescriptor) override;
    virtual void ConstraintsBiReset() override;
    virtual void ConstraintsBiLoad_C(double factor = 1., double recovery_clamp = 0.1, bool do_clamp = false) override;
    virtual void ConstraintsBiLoad_Ct(double factor = 1.) override;
    virtual void ConstraintsBiLoad_Qc(double factor = 1.) override;
    virtual void ConstraintsLoadJacobians() override;
    virtual void ConstraintsFetch_react(double factor = 1.) override;

    // Other functions

    virtual bool GetCollide() override { return true; }
    virtual void SyncCollisionModels() override;
    virtual void AddCollisionModelsToSystem() override;
    virtual void RemoveCollisionModelsFromSystem() override;

    //
    // UPDATE FUNCTIONS
    //

    /// Update all auxiliary data of the conveyor at given time
    virtual void Update(double mytime, bool update_assets = true) override;

    //
    // SERIALIZATION
    //

    /// Method to allow serialization of transient data to archives.
    virtual void ArchiveOUT(ChArchiveOut& marchive) const override;

    /// Method to allow deserialization of transient data from archives.
    virtual void ArchiveIN(ChArchiveIn& marchive) override;

};

}  // END_OF_NAMESPACE____

#endif

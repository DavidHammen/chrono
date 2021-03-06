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

#ifndef CHPARTICLESCLONES_H
#define CHPARTICLESCLONES_H

//////////////////////////////////////////////////
//
//   ChParticlesClones.h
//
//   Class for clusters of particle 'clones', that is many
//   rigid objects that share the same shape and mass.
//   This can be used to make granular flows.
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include <math.h>

#include "physics/ChIndexedParticles.h"
#include "collision/ChCCollisionModel.h"
#include "lcp/ChLcpVariablesBodySharedMass.h"
#include "physics/ChMaterialSurface.h"
#include "physics/ChContactable.h"

namespace chrono {

// Forward references (for parent hierarchy pointer)

class ChSystem;
class ChParticlesClones;

/// Class for a single particle clone in the ChParticlesClones cluster.
/// It does not define mass, inertia and shape because those are _shared_ among them.
class ChApi ChAparticle : public ChParticleBase, public ChContactable_1vars<6> {

  public:
    ChAparticle();
    ~ChAparticle();

    ChAparticle(const ChAparticle& other);             // Copy constructor
    ChAparticle& operator=(const ChAparticle& other);  // Assignment operator

    // Access the 'LCP variables' of the node
    virtual ChLcpVariables& Variables() { return variables; }

    // Get the container
    ChParticlesClones* GetContainer() const {return container;}
    // Set the container
    void SetContainer(ChParticlesClones* mc) { container = mc;}

    //
    // INTERFACE TO ChContactable
    //

    /// Access variables.
    virtual ChLcpVariables* GetVariables1() override { return &Variables(); }

    /// Tell if the object must be considered in collision detection.
    virtual bool IsContactActive() override { return true; }

    /// Get the number of DOFs affected by this object (position part).
    virtual int ContactableGet_ndof_x() override { return 7; }

    /// Get the number of DOFs affected by this object (speed part).
    virtual int ContactableGet_ndof_w() override { return 6; }

    /// Get all the DOFs packed in a single vector (position part)
    virtual void ContactableGetStateBlock_x(ChState& x) override { x.PasteCoordsys(this->GetCoord(), 0, 0); }

    /// Get all the DOFs packed in a single vector (speed part)
    virtual void ContactableGetStateBlock_w(ChStateDelta& w) override {
        w.PasteVector(this->GetPos_dt(), 0, 0);
        w.PasteVector(this->GetWvel_loc(), 3, 0);
    }

    /// Increment the provided state of this object by the given state-delta increment.
    /// Compute: x_new = x + dw.
    virtual void ContactableIncrementState(const ChState& x, const ChStateDelta& dw, ChState& x_new) override {
        // Increment position
        x_new(0) = x(0) + dw(0);
        x_new(1) = x(1) + dw(1);
        x_new(2) = x(2) + dw(2);

        // Increment rotation: rot' = delta*rot  (use quaternion for delta rotation)
        ChQuaternion<> mdeltarot;
        ChQuaternion<> moldrot = x.ClipQuaternion(3, 0);
        ChVector<> newwel_abs = Amatrix * dw.ClipVector(3, 0);
        double mangle = newwel_abs.Length();
        newwel_abs.Normalize();
        mdeltarot.Q_from_AngAxis(mangle, newwel_abs);
        ChQuaternion<> mnewrot = mdeltarot * moldrot;  // quaternion product
        x_new.PasteQuaternion(mnewrot, 3, 0);
    }

    /// Return the pointer to the contact surface material.
    virtual std::shared_ptr<ChMaterialSurfaceBase>& GetMaterialSurfaceBase() override;

    /// Express the local point in absolute frame, for the given state position.
    virtual ChVector<> GetContactPoint(const ChVector<>& loc_point, const ChState& state_x) override {
        ChCoordsys<> csys = state_x.ClipCoordsys(0, 0);
        return csys.TransformPointLocalToParent(loc_point);
    }

    /// Get the absolute speed of a local point attached to the contactable.
    /// The given point is assumed to be expressed in the local frame of this object.
    /// This function must use the provided states.
    virtual ChVector<> GetContactPointSpeed(const ChVector<>& loc_point,
                                            const ChState& state_x,
                                            const ChStateDelta& state_w) override {
        ChCoordsys<> csys = state_x.ClipCoordsys(0, 0);
        ChVector<> abs_vel = state_w.ClipVector(0, 0);
        ChVector<> loc_omg = state_w.ClipVector(3, 0);
        ChVector<> abs_omg = csys.TransformDirectionLocalToParent(loc_omg);

        return abs_vel + Vcross(abs_omg, loc_point);
    }

    /// Get the absolute speed of point abs_point if attached to the surface.
    /// Easy in this case because there are no roations..
    virtual ChVector<> GetContactPointSpeed(const ChVector<>& abs_point) override;

    /// Return the coordinate system for the associated collision model.
    /// ChCollisionModel might call this to get the position of the
    /// contact model (when rigid) and sync it.
    virtual ChCoordsys<> GetCsysForCollisionModel() override { return this->coord; }

    /// Apply the force, expressed in absolute reference, applied in pos, to the
    /// coordinates of the variables. Force for example could come from a penalty model.
    virtual void ContactForceLoadResidual_F(const ChVector<>& F,
                                            const ChVector<>& abs_point,
                                            ChVectorDynamic<>& R) override;

    /// Apply the given force at the given point and load the generalized force array.
    /// The force and its application point are specified in the gloabl frame.
    /// Each object must set the entries in Q corresponding to its variables, starting at the specified offset.
    /// If needed, the object states must be extracted from the provided state position.
    virtual void ContactForceLoadQ(const ChVector<>& F,
                                   const ChVector<>& point,
                                   const ChState& state_x,
                                   ChVectorDynamic<>& Q,
                                   int offset) override {
        ChCoordsys<> csys = state_x.ClipCoordsys(0, 0);
        ChVector<> point_loc = csys.TransformPointParentToLocal(point);
        ChVector<> force_loc = csys.TransformDirectionParentToLocal(F);
        ChVector<> torque_loc = Vcross(point_loc, force_loc);
        Q.PasteVector(F, offset + 0, 0);
        Q.PasteVector(torque_loc, offset + 3, 0);
    }

    /// Compute the jacobian(s) part(s) for this contactable item. For example,
    /// if the contactable is a ChBody, this should update the corresponding 1x6 jacobian.
    virtual void ComputeJacobianForContactPart(
        const ChVector<>& abs_point,
        ChMatrix33<>& contact_plane,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_N,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_U,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_V,
        bool second) override;

    /// Compute the jacobian(s) part(s) for this contactable item, for rolling about N,u,v
    /// (used only for rolling friction DVI contacts)
    virtual void ComputeJacobianForRollingContactPart(
        const ChVector<>& abs_point,
        ChMatrix33<>& contact_plane,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_N,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_U,
        ChLcpVariableTupleCarrier_1vars<6>::type_constraint_tuple& jacobian_tuple_V,
        bool second) override;

    /// used by some DEM code
    virtual double GetContactableMass() override { return this->variables.GetBodyMass(); }

    /// This is only for backward compatibility
    virtual ChPhysicsItem* GetPhysicsItem() override;

    // SERIALIZATION

    virtual void ArchiveOUT(ChArchiveOut& marchive);
    virtual void ArchiveIN(ChArchiveIn& marchive);

    //
    // DATA
    //

    ChParticlesClones* container;
    ChLcpVariablesBodySharedMass variables;
    collision::ChCollisionModel* collision_model;
    ChVector<> UserForce;
    ChVector<> UserTorque;
};

/// Class for clusters of 'clone' particles, that is many
/// rigid objects with the same shape and mass.
/// This can be used to make granular flows, where
/// you have thousands of objects with the same shape.
/// In fact, a single ChParticlesClones object can
/// be more memory-efficient than many ChBody objects,
/// because they share many features, such as mass and
/// collision shape.
/// If you have N different families of shapes in your
/// granular simulations (ex. 50% of particles are large
/// spheres, 25% are small spheres and 25% are polihedrons)
/// you can simply add three ChParticlesClones objects to the
/// ChSystem. This would be more efficient anyway than
/// creating all shapes as ChBody.

class ChApi ChParticlesClones : public ChIndexedParticles {
    // Chrono simulation of RTTI, needed for serialization
    CH_RTTI(ChParticlesClones, ChIndexedParticles);

  private:
    //
    // DATA
    //

    // The particles:
    std::vector<ChAparticle*> particles;

    // Shared mass of particles
    ChSharedMassBody particle_mass;

    // Sample collision model
    collision::ChCollisionModel* particle_collision_model;

    // data for surface contact and impact (can be shared):
    std::shared_ptr<ChMaterialSurfaceBase> matsurface;

    bool do_collide;
    bool do_limit_speed;
    bool do_sleep;

    float max_speed;  // limit on linear speed (useful for VR & videagames)
    float max_wvel;   // limit on angular vel. (useful for VR & videagames)

    float sleep_time;
    float sleep_minspeed;
    float sleep_minwvel;
    float sleep_starttime;

  public:
    //
    // CONSTRUCTORS
    //

    /// Build a cluster of particles.
    /// By default the cluster will contain 0 particles.
    ChParticlesClones();

    /// Destructor
    ~ChParticlesClones();

    /// Copy from another ChParticlesClones.
    void Copy(ChParticlesClones* source);

    //
    // FLAGS
    //

    /// Enable/disable the collision for this cluster of particles.
    /// After setting ON, remember RecomputeCollisionModel()
    /// before anim starts (it is not automatically
    /// recomputed here because of performance issues.)
    void SetCollide(bool mcoll);
    bool GetCollide() { return do_collide; }

    /// Trick. Set the maximum linear speed (beyond this limit it will
    /// be clamped). This is useful in virtual reality and real-time
    /// simulations, because it reduces the risk of bad collision detection.
    /// The realism is limited, but the simulation is more stable.
    void SetLimitSpeed(bool mlimit) { do_limit_speed = mlimit; };
    bool GetLimitSpeed() { return do_limit_speed; };

    //
    // FUNCTIONS
    //

    /// Get the number of particles
    size_t GetNparticles() const { return particles.size(); }

    /// Access the N-th particle
    ChParticleBase& GetParticle(unsigned int n) {
        assert(n < particles.size());
        return *particles[n];
    }

    /// Resize the particle cluster. Also clear the state of
    /// previously created particles, if any.
    /// NOTE! Define the sample collision shape using GetCollisionModel()->...
    /// before adding particles!
    void ResizeNparticles(int newsize);

    /// Add a new particle to the particle cluster, passing a
    /// coordinate system as initial state.
    /// NOTE! Define the sample collision shape using GetCollisionModel()->...
    /// before adding particles!
    void AddParticle(ChCoordsys<double> initial_state = CSYSNORM);


    /// Set the material surface for contacts
    void SetMaterialSurface(const std::shared_ptr<ChMaterialSurfaceBase>& mnewsurf) { matsurface = mnewsurf; }

    /// Set the material surface for contacts 
    virtual std::shared_ptr<ChMaterialSurfaceBase>& GetMaterialSurfaceBase() { return matsurface;}



    //
    // STATE FUNCTIONS
    //

    // (override/implement interfaces for global state vectors, see ChPhysicsItem for comments.)
    virtual void IntStateGather(const unsigned int off_x,
                                ChState& x,
                                const unsigned int off_v,
                                ChStateDelta& v,
                                double& T);
    virtual void IntStateScatter(const unsigned int off_x,
                                 const ChState& x,
                                 const unsigned int off_v,
                                 const ChStateDelta& v,
                                 const double T);
    virtual void IntStateGatherAcceleration(const unsigned int off_a, ChStateDelta& a);
    virtual void IntStateScatterAcceleration(const unsigned int off_a, const ChStateDelta& a);
    virtual void IntStateIncrement(const unsigned int off_x,
                                   ChState& x_new,
                                   const ChState& x,
                                   const unsigned int off_v,
                                   const ChStateDelta& Dv);
    virtual void IntLoadResidual_F(const unsigned int off, ChVectorDynamic<>& R, const double c);
    virtual void IntLoadResidual_Mv(const unsigned int off,
                                    ChVectorDynamic<>& R,
                                    const ChVectorDynamic<>& w,
                                    const double c);
    virtual void IntToLCP(const unsigned int off_v,
                          const ChStateDelta& v,
                          const ChVectorDynamic<>& R,
                          const unsigned int off_L,
                          const ChVectorDynamic<>& L,
                          const ChVectorDynamic<>& Qc);
    virtual void IntFromLCP(const unsigned int off_v, ChStateDelta& v, const unsigned int off_L, ChVectorDynamic<>& L);

    //
    // LCP FUNCTIONS
    //

    // Override/implement LCP system functions of ChPhysicsItem
    // (to assembly/manage data for LCP system solver)

    void VariablesFbReset();
    void VariablesFbLoadForces(double factor = 1.);
    void VariablesQbLoadSpeed();
    void VariablesFbIncrementMq();
    void VariablesQbSetSpeed(double step = 0.);
    void VariablesQbIncrementPosition(double step);
    virtual void InjectVariables(ChLcpSystemDescriptor& mdescriptor);

    // Other functions

    /// Set no speed and no accelerations (but does not change the position)
    void SetNoSpeedNoAcceleration();

    /// Acess the collision model for the collision engine: this is the 'sample'
    /// collision model that is used by all particles.
    /// To get a non-null pointer, remember to SetCollide(true), before.
    collision::ChCollisionModel* GetCollisionModel() { return particle_collision_model; }

    /// Synchronize coll.models coordinates and bounding boxes to the positions of the particles.
    virtual void SyncCollisionModels();
    virtual void AddCollisionModelsToSystem();
    virtual void RemoveCollisionModelsFromSystem();

    /// After you added collision shapes to the sample coll.model (the one
    /// that you access with GetCollisionModel() ) you need to call this
    /// function so that all collision models of particles will reference the sample coll.model.
    void UpdateParticleCollisionModels();


    /// Mass of each particle. Must be positive.
    void SetMass(double newmass) {
        if (newmass > 0.)
            this->particle_mass.SetBodyMass(newmass);
    }
    double GetMass() { return this->particle_mass.GetBodyMass(); }

    /// Set the inertia tensor of each particle
    void SetInertia(const ChMatrix33<>& newXInertia);
    /// Set the diagonal part of the inertia tensor of each particle
    void SetInertiaXX(const ChVector<>& iner);
    /// Get the diagonal part of the inertia tensor of each particle
    ChVector<> GetInertiaXX();
    /// Set the extradiagonal part of the inertia tensor of each particle
    /// (xy, yz, zx values, the rest is symmetric)
    void SetInertiaXY(const ChVector<>& iner);
    /// Get the extradiagonal part of the inertia tensor of each particle
    /// (xy, yz, zx values, the rest is symmetric)
    ChVector<> GetInertiaXY();

    /// Trick. Set the maximum linear speed (beyond this limit it will
    /// be clamped). This is useful in virtual reality and real-time
    /// simulations, because it reduces the risk of bad collision detection.
    /// This speed limit is active only if you set  SetLimitSpeed(true);
    void SetMaxSpeed(float m_max_speed) { max_speed = m_max_speed; }
    float GetMaxSpeed() { return max_speed; }

    /// Trick. Set the maximum angualar speed (beyond this limit it will
    /// be clamped). This is useful in virtual reality and real-time
    /// simulations, because it reduces the risk of bad collision detection.
    /// This speed limit is active only if you set  SetLimitSpeed(true);
    void SetMaxWvel(float m_max_wvel) { max_wvel = m_max_wvel; }
    float GetMaxWvel() { return max_wvel; }

    /// When this function is called, the speed of particles is clamped
    /// into limits posed by max_speed and max_wvel  - but remember to
    /// put the body in the SetLimitSpeed(true) mode.
    void ClampSpeed();

    /// Set the amount of time which must pass before going automatically in
    /// sleep mode when the body has very small movements.
    void SetSleepTime(float m_t) { sleep_time = m_t; }
    float GetSleepTime() { return sleep_time; }

    /// Set the max linear speed to be kept for 'sleep_time' before freezing.
    void SetSleepMinSpeed(float m_t) { sleep_minspeed = m_t; }
    float GetSleepMinSpeed() { return sleep_minspeed; }

    /// Set the max linear speed to be kept for 'sleep_time' before freezing.
    void SetSleepMinWvel(float m_t) { sleep_minwvel = m_t; }
    float GetSleepMinWvel() { return sleep_minwvel; }

    //
    // UPDATE FUNCTIONS
    //

    /// Update all auxiliary data of the particles
    virtual void Update(double mytime, bool update_assets = true);
    /// Update all auxiliary data of the particles
    virtual void Update(bool update_assets = true);

    // SERIALIZATION

    virtual void ArchiveOUT(ChArchiveOut& marchive);
    virtual void ArchiveIN(ChArchiveIn& marchive);
};

}  // END_OF_NAMESPACE____

#endif

//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010-2012 Alessandro Tasora
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHLINKDIRFRAME_H
#define CHLINKDIRFRAME_H

//////////////////////////////////////////////////
//
//   ChLinkPointFrame.h
//
//   Class for creating a constraint between the direction
//   of a ChNodeFEAxyzD and a ChBody object.
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include "chrono/lcp/ChLcpConstraintTwoGeneric.h"
#include "chrono/physics/ChBodyFrame.h"
#include "chrono/physics/ChLinkBase.h"
#include "chrono_fea/ChNodeFEAxyzD.h"

namespace chrono {

class ChIndexedNodes;  // forward ref

namespace fea {

/// @addtogroup fea_constraints
/// @{

/// Class for creating a constraint between the direction of a FEA node
/// of ChNodeFEAxyzD class, and a ChBodyFrame (frame).
/// The D direction of the ChNodeFEAxyzD is enforced to stay parallel
/// to a given direction associated to the ChBodyFrame.
class ChApiFea ChLinkDirFrame : public ChLinkBase {

						// Chrono simulation of RTTI, needed for serialization
	CH_RTTI(ChLinkDirFrame,ChLinkBase);

private:
			//
	  		// DATA
			//

	ChVector<> react;					
	
						// used as an interface to the LCP solver.
	ChLcpConstraintTwoGeneric constraint1;
	ChLcpConstraintTwoGeneric constraint2;

    std::shared_ptr<fea::ChNodeFEAxyzD> mnode;
    std::shared_ptr<ChBodyFrame>  body;

	ChVector<> direction; 
	ChCoordsys<> csys_direction;

public:

			//
	  		// CONSTRUCTORS
			//

				/// Build a shaft.
	ChLinkDirFrame ();
				/// Destructor
	~ChLinkDirFrame ();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
				/// Copy from another ChLinkDirFrame. 
	void Copy(ChLinkDirFrame* source);
#pragma GCC diagnostic pop


			//
	  		// FLAGS
			//

			//
	  		// FUNCTIONS
			//

				/// Get the number of scalar variables affected by constraints in this link 
	virtual int GetNumCoords() override {return 3 + 4;}

				/// Number of scalar costraints 
	virtual int GetDOC_c  () override {return 2;}

				/// To get reaction force, expressed in link coordinate system:
	virtual ChVector<> Get_react_torque() override {return GetReactionOnBody();}

    // Get constraint violations
    ChMatrixNM<double, 2, 1> GetC();

	 		//
			// STATE FUNCTIONS
			//

				// (override/implement interfaces for global state vectors, see ChPhysicsItem for comments.)
	virtual void IntStateGatherReactions(const unsigned int off_L,	ChVectorDynamic<>& L) override;
	virtual void IntStateScatterReactions(const unsigned int off_L,	const ChVectorDynamic<>& L) override;
	virtual void IntLoadResidual_CqL(const unsigned int off_L, ChVectorDynamic<>& R, const ChVectorDynamic<>& L, const double c) override;
	virtual void IntLoadConstraint_C(const unsigned int off, ChVectorDynamic<>& Qc,	const double c, bool do_clamp,	double recovery_clamp) override;
	virtual void IntToLCP(const unsigned int off_v,	const ChStateDelta& v, const ChVectorDynamic<>& R, const unsigned int off_L, const ChVectorDynamic<>& L, const ChVectorDynamic<>& Qc) override;
	virtual void IntFromLCP(const unsigned int off_v, ChStateDelta& v, const unsigned int off_L, ChVectorDynamic<>& L) override;


			// Override/implement LCP system functions of ChPhysicsItem
			// (to assembly/manage data for LCP system solver

	virtual void InjectConstraints(ChLcpSystemDescriptor& mdescriptor) override;
	virtual void ConstraintsBiReset() override;
	virtual void ConstraintsBiLoad_C(double factor=1., double recovery_clamp=0.1, bool do_clamp=false) override;
	virtual void ConstraintsBiLoad_Ct(double factor=1.) override;
	virtual void ConstraintsLoadJacobians() override;
	virtual void ConstraintsFetch_react(double factor=1.) override;


			   // Other functions
	
	virtual ChCoordsys<> GetLinkAbsoluteCoords() override;

				/// Use this function after object creation, to initialize it, given  
				/// the node and body frame to join. 
				/// The attachment position is the actual position of the node (unless
				/// otherwise defines, using the optional 'mattach' parameter).
				/// Note, mnodes and mbody must belong to the same ChSystem. 
	virtual int Initialize(std::shared_ptr<ChNodeFEAxyzD> anode, ///< xyzD node to join (with the direction)
						   std::shared_ptr<ChBodyFrame>  mbody,  ///< body (frame) to join 
						   ChVector<>* dir=0			         ///< optional: if not null, sets the direction in absolute coordinates 
						   );

				/// Get the connected xyz node (point)
    virtual std::shared_ptr<ChNodeFEAxyzD> GetConstrainedNode() { return this->mnode; }
				
				/// Get the connected body (frame)
    virtual std::shared_ptr<ChBodyFrame> GetConstrainedBodyFrame() { return this->body; }

					/// Get the attachment position, in the reference coordinates of the body.
	ChVector<> GetDirection() {return direction;}
					/// Set the attachment position, in the reference coordinates of the body
	void SetDirectionInBodyCoords(ChVector<> mattach);
					/// Set the attachment position, in the absolute coordinates
	void SetDirectionInAbsoluteCoords(ChVector<> mattach);


				/// Get the reaction torque considered as applied to the FEA node.
	ChVector<> GetReactionOnNode() {return -(react);}

				/// Get the reaction torque considered as applied to ChBody.
	ChVector<> GetReactionOnBody() {return react;}

	
			//
			// UPDATE FUNCTIONS
			//

				/// Update all auxiliary data of the gear transmission at given time
  virtual void Update(double mytime, bool update_assets = true) override;


			//
			// STREAMING
			//


				/// Method to allow deserializing a persistent binary archive (ex: a file)
				/// into transient data.
	void StreamIN(ChStreamInBinary& mstream);

				/// Method to allow serializing transient data into a persistent
				/// binary archive (ex: a file).
	void StreamOUT(ChStreamOutBinary& mstream);

};

/// @} fea_constraints

} // END_OF_NAMESPACE____
} // END_OF_NAMESPACE____


#endif

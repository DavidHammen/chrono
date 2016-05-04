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

#ifndef CHLINKPOINTPOINT_H
#define CHLINKPOINTPOINT_H

#include "chrono/lcp/ChLcpConstraintTwoGeneric.h"
#include "chrono/physics/ChLinkBase.h"
#include "chrono_fea/ChNodeFEAxyz.h"

namespace chrono {

class ChIndexedNodes; // forward ref

namespace fea {

/// @addtogroup fea_constraints
/// @{

/// Class for creating a constraint between two xyz FEA nodes (points).
/// That is, the two nodes will be joined, as overlapping.
/// Nodes are 3-DOF points that are used in point-based 
/// primitives, such as ChMatterSPH or finite elements.
class ChApiFea ChLinkPointPoint : public ChLinkBase {

						// Chrono simulation of RTTI, needed for serialization
	CH_RTTI(ChLinkPointPoint,ChLinkBase);

private:
			//
	  		// DATA
			//

	ChVector<> react;					
	
						// used as an interface to the LCP solver.
	ChLcpConstraintTwoGeneric constraint1;
	ChLcpConstraintTwoGeneric constraint2;
	ChLcpConstraintTwoGeneric constraint3;

    std::shared_ptr<fea::ChNodeFEAxyz> mnodeA;
    std::shared_ptr<fea::ChNodeFEAxyz> mnodeB;

public:

			//
	  		// CONSTRUCTORS
			//

				/// Build a shaft.
	ChLinkPointPoint ();
				/// Destructor
	~ChLinkPointPoint ();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
				/// Copy from another ChLinkPointFrame. 
	void Copy(ChLinkPointPoint* source);
#pragma GCC diagnostic pop


			//
	  		// FLAGS
			//

			//
	  		// FUNCTIONS
			//

				/// Get the number of scalar variables affected by constraints in this link 
	virtual int GetNumCoords() override {return 3 + 3;}

				/// Number of scalar constraints 
	virtual int GetDOC_c  () override {return 3;}

				/// To get reaction force, expressed in link coordinate system:
	virtual ChVector<> Get_react_force() override {return GetReactionOnNode();}

    // Get constraint violations
    ChMatrixNM<double, 3, 1> GetC();

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
				/// the two nodes join. 
				/// The attachment position is the actual position of the node.
				/// Note, mnodes must belong to the same ChSystem. 
    virtual int Initialize(std::shared_ptr<ChNodeFEAxyz> anodeA,  ///< xyz node (point) to join
						   std::shared_ptr<ChNodeFEAxyz> anodeB   ///< xyz node (point) to join
						   );

				/// Get the 1st connected xyz node (point)
    virtual std::shared_ptr<fea::ChNodeFEAxyz> GetConstrainedNodeA() { return this->mnodeA; }

    			/// Get the 2nd connected xyz node (point)
    virtual std::shared_ptr<fea::ChNodeFEAxyz> GetConstrainedNodeB() { return this->mnodeB; }
				
				/// Get the reaction force considered as applied to ChShaft.
	ChVector<> GetReactionOnNode() {return -(react);}


	
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

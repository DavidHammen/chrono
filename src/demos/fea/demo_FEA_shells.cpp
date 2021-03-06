//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

//
//   Demo code about
//
//     - FEA for 3D beams of 'cable' type (ANCF gradient-deficient beams)

// Include some headers used by this tutorial...

#include "chrono/lcp/ChLcpIterativeMINRES.h"
#include "chrono/lcp/ChLcpIterativePMINRES.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChSystem.h"
#include "chrono/timestepper/ChTimestepper.h"

#include "chrono_fea/ChElementShellANCF.h"
#include "chrono_fea/ChElementShellEANS4.h"
#include "chrono_fea/ChLinkDirFrame.h"
#include "chrono_fea/ChLinkPointFrame.h"
#include "chrono_fea/ChMesh.h"
#include "chrono_fea/ChVisualizationFEAmesh.h"
#include "chrono_mkl/ChLcpMklSolver.h"
#include "chrono_irrlicht/ChIrrApp.h"
#include <vector>

// Remember to use the namespace 'chrono' because all classes 
// of Chrono::Engine belong to this namespace and its children...

using namespace chrono;
using namespace chrono::fea;
using namespace chrono::irrlicht;
using namespace irr;

int main(int argc, char* argv[]) {
    // Create a Chrono::Engine physical system
    ChSystem my_system;

    // Create the Irrlicht visualization (open the Irrlicht device,
    // bind a simple user interface, etc. etc.)
    ChIrrApp application(&my_system, L"Shells FEA", core::dimension2d<u32>(800, 600), false, true);

    // Easy shortcuts to add camera, lights, logo and sky in Irrlicht scene:
    application.AddTypicalLogo();
    application.AddTypicalSky();
    application.AddTypicalLights();
    application.AddTypicalCamera(core::vector3df(0.f, 0.6f, -1.f));

    // Create a mesh, that is a container for groups
    // of elements and their referenced nodes.
    auto my_mesh = std::make_shared<ChMesh>();

    // Remember to add the mesh to the system!
    my_system.Add(my_mesh);

    //my_system.Set_G_acc(VNULL); or 
    my_mesh->SetAutomaticGravity(false);

    //
    // Add an ANCF SHELL:
    //
    if(false)
    {
        double shell_thickness = 0.01;
        double shell_L = 0.4;
        double shell_W = 0.2;

        // Create the nodes (each with position & normal to shell)
        auto hnodeancf1 = std::make_shared<ChNodeFEAxyzD>(ChVector<>(0, 0.02, 0), ChVector<>(0, 1, 0));
        auto hnodeancf2 = std::make_shared<ChNodeFEAxyzD>(ChVector<>(shell_L, 0, 0), ChVector<>(0, 1, 0));
        auto hnodeancf3 = std::make_shared<ChNodeFEAxyzD>(ChVector<>(0, 0, shell_W), ChVector<>(0, 1, 0));
        auto hnodeancf4 = std::make_shared<ChNodeFEAxyzD>(ChVector<>(shell_L, 0, shell_W), ChVector<>(0, 1, 0));

        my_mesh->AddNode(hnodeancf1);
        my_mesh->AddNode(hnodeancf2);
        my_mesh->AddNode(hnodeancf3);
        my_mesh->AddNode(hnodeancf4);

        // Create the element

        auto elementancf1 = std::make_shared<ChElementShellANCF>();
        my_mesh->AddElement(elementancf1);

        // Set its nodes
        elementancf1->SetNodes(hnodeancf1, hnodeancf2, hnodeancf3, hnodeancf4);

        // Set element dimensions
        elementancf1->SetDimensions(shell_L, shell_W);

        // Create an orthotropic material 
        double rho = 500;
        ChVector<> E(2.1e7, 2.1e7, 2.1e7);
        ChVector<> nu(0.3, 0.3, 0.3);
        ChVector<> G(8.0769231e6, 8.0769231e6, 8.0769231e6);
        auto mat = std::make_shared<ChMaterialShellANCF>(rho, E, nu, G);

        // Add a single layers with a fiber angle of 0 degrees.
        elementancf1->AddLayer(shell_thickness, 0 * CH_C_DEG_TO_RAD, mat);

        // Set other element properties
        elementancf1->SetAlphaDamp(0.0);    // Structural damping for this element
        elementancf1->SetGravityOn(false);  // turn internal gravitational force calculation off

        // Apply a lumped force to a node:
        hnodeancf3->SetForce(ChVector<>(0, 50, 0));

        hnodeancf1->SetFixed(true);
        hnodeancf2->SetFixed(true);
    }

    //
    // Add an EANS SHELL:
    //
    std::shared_ptr<ChNodeFEAxyzrot> node3;
    std::shared_ptr<ChNodeFEAxyzrot> node4;

    if (false)
    {
        double shell_thickness = 0.02;
        double shell_L = 1;
        double shell_W = 1;

        // Create a material 
        double rho = 0.0;
        double E = 1e9;
        double nu = 0.4; 
        auto mat = std::make_shared<ChMaterialShellEANS>(shell_thickness,
                                                         rho, 
                                                         E, 
                                                         nu,
                                                         1.00,
                                                         0.01);

        // Create the nodes (each with position & normal to shell)
        
        auto hnodeeans1 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, 0, 0)));
        auto hnodeeans2 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, 0, 0)));
        auto hnodeeans3 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, shell_W, 0)));
        auto hnodeeans4 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, shell_W, 0 )));
        
        /*
        auto hnodeeans1 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, 0, 0)));
        auto hnodeeans2 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, 0, 0)));
        auto hnodeeans3 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, 0, shell_W)));
        auto hnodeeans4 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, 0 , shell_W )));
        */
        node3 = hnodeeans3;
        node4 = hnodeeans4;
        double mn = 10.5;
        hnodeeans1->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans2->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans3->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans4->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans1->SetMass(mn);
        hnodeeans1->SetMass(mn);
        hnodeeans1->SetMass(mn);
        hnodeeans1->SetMass(mn);
        my_mesh->AddNode(hnodeeans1);
        my_mesh->AddNode(hnodeeans2);
        my_mesh->AddNode(hnodeeans3);
        my_mesh->AddNode(hnodeeans4);

        hnodeeans1->SetFixed(true);
        hnodeeans2->SetFixed(true);

        // Create the element

        auto elementeans = std::make_shared<ChElementShellEANS4>();
        my_mesh->AddElement(elementeans);

        // Set its nodes
        elementeans->SetNodes(hnodeeans1, hnodeeans2, hnodeeans3, hnodeeans4);

        // Set element dimensions
        //elementeans->SetDimensions(shell_L, shell_W); // not needed, already set at initialization from initial pos of nodes

        // Add a single layers with a fiber angle of 0 degrees.
        elementeans->AddLayer(shell_thickness, 0 * CH_C_DEG_TO_RAD, mat);

        // Set other element properties
        elementeans->SetAlphaDamp(0.02);    // Structural damping for this element

        // Apply a lumped force to a node:
       // hnodeeans3->SetPos(hnodeeans3->GetPos()+ChVector<>(0, 0, 0.01));
       // hnodeeans4->SetPos(hnodeeans4->GetPos()+ChVector<>(0, 0, 0.01));
       // hnodeeans3->SetForce(ChVector<>(0, 3000, 0));
       // hnodeeans4->SetForce(ChVector<>(0, 3000, 0));
       //hnodeeans3->SetForce(ChVector<>(0, 0, 100));
       //hnodeeans4->SetForce(ChVector<>(0, 0, 100));
       // hnodeeans3->SetForce(ChVector<>(0, 50, 0));
       // hnodeeans4->SetForce(ChVector<>(0, 50, 0));
       //hnodeeans3->SetTorque(ChVector<>(0.2, 0, 0));
       //hnodeeans4->SetTorque(ChVector<>(0.2, 0, 0));
       // hnodeeans4->SetMass(2000);


        
        auto hnodeeans5 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, shell_W*2, 0)));
        auto hnodeeans6 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, shell_W*2, 0 )));
        
        //auto hnodeeans5 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(shell_L, 0, shell_W*2)));
        //auto hnodeeans6 = std::make_shared<ChNodeFEAxyzrot>(ChFrame<>(ChVector<>(0, 0, shell_W*2 )));
        
        hnodeeans5->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans6->GetInertia().FillDiag(1./12.*pow((shell_L/2.),3)*mn);
        hnodeeans5->SetMass(mn);
        hnodeeans6->SetMass(mn);

        my_mesh->AddNode(hnodeeans5);
        my_mesh->AddNode(hnodeeans6);

        // Create the element

        auto elementeansb = std::make_shared<ChElementShellEANS4>();
        my_mesh->AddElement(elementeansb);

        // Set its nodes
        elementeansb->SetNodes(hnodeeans4, hnodeeans3, hnodeeans5, hnodeeans6);

        // Set element dimensions
        //elementeans->SetDimensions(shell_L, shell_W); // not needed, already set at initialization from initial pos of nodes

        // Add a single layers with a fiber angle of 0 degrees.
        elementeansb->AddLayer(shell_thickness, 0 * CH_C_DEG_TO_RAD, mat);

        // Set other element properties
        elementeansb->SetAlphaDamp(0.0);    // Structural damping for this element

        hnodeeans5->SetForce(ChVector<>(0, 0, 10));
        hnodeeans6->SetForce(ChVector<>(0, 0, 10));
        //hnodeeans5->SetForce(ChVector<>(0, 10, 0));
        //hnodeeans6->SetForce(ChVector<>(0, 10, 0));
        //hnodeeans5->SetTorque(ChVector<>(5, 0, 0));
        //hnodeeans6->SetTorque(ChVector<>(5, 0, 0));
        //hnodeeans6->SetMass(2000);
        
        
    }


    //
    // Add an EANS SHELL cantilever:
    //
    std::shared_ptr<ChNodeFEAxyzrot> nodeA;
    std::shared_ptr<ChNodeFEAxyzrot> nodeB;

    if (true)
    {
        double rect_thickness = 0.10;
        double rect_L = 10.0;
        double rect_W = 1.0;

        // Create a material 
        double rho = 0.0;
        double E = 1.2e6;
        double nu = 0.0; 
        auto mat = std::make_shared<ChMaterialShellEANS>(rect_thickness,
                                                         rho, 
                                                         E, 
                                                         nu,
                                                         1.0,
                                                         0.01);

        // Create the nodes (each with position & normal to shell)
        double node_density=0.01;
        
        int nels_L = 15;
        int nels_W = 1;
        std::vector<std::shared_ptr<ChElementShellEANS4>> elarray(nels_L*nels_W);
        std::vector<std::shared_ptr<ChNodeFEAxyzrot>>     nodearray((nels_L+1)*(nels_W+1));
        std::vector<std::shared_ptr<ChNodeFEAxyzrot>>     nodes_start(nels_W+1);
        std::vector<std::shared_ptr<ChNodeFEAxyzrot>>     nodes_end(nels_W+1);

        for (int il = 0; il<= nels_L; ++il) {
            for (int iw = 0; iw<= nels_W; ++iw) {
                // Make nodes
                ChVector<> nodepos(rect_L*((double)il/(double)nels_L), 0,  rect_W*((double)iw/(double)nels_W));
                ChQuaternion<> noderot(QUNIT);
                ChFrame<> nodeframe(nodepos,noderot);

                auto mnode = std::make_shared<ChNodeFEAxyzrot>(nodeframe);
                my_mesh->AddNode(mnode);

                double mn = node_density*(rect_L*rect_W*rect_thickness)/(nels_L*nels_W); // approx
                mnode->GetInertia().FillDiag(1./12.*pow(((rect_L/nels_L)/2.),3)*mn); // approx
                mnode->SetMass(mn);

                nodearray[il*(nels_W+1) + iw] = mnode;

                if (il==0)
                    nodes_start[iw] = mnode;
                if (il==nels_L)
                    nodes_end[iw] = mnode;

                // Make elements
                if (il>0 && iw>0) {
                    auto melement = std::make_shared<ChElementShellEANS4>();
                    my_mesh->AddElement(melement);
                    melement->SetNodes(
                        nodearray[(il-1)*(nels_W+1) + (iw-1)], 
                        nodearray[(il  )*(nels_W+1) + (iw-1)],
                        nodearray[(il  )*(nels_W+1) + (iw  )],
                        nodearray[(il-1)*(nels_W+1) + (iw  )]);
                    melement->AddLayer(rect_thickness, 0 * CH_C_DEG_TO_RAD, mat);
                    melement->SetAlphaDamp(0.02);   
                    elarray[(il-1)*(nels_W) + (iw-1)] = melement;
                }
            }
        }
        for (auto mendnode : nodes_end) {
            mendnode->SetForce(ChVector<>(0,4,0) * (1./ (double)nodes_end.size()) );
            //mendnode->SetTorque(ChVector<>(0,0 , 50*CH_C_PI/3) * (1./ (double)nodes_end.size()) );
        }
        for (auto mstartnode : nodes_start) {
            mstartnode->SetFixed(true);
        }

    }


    

    // ==Asset== attach a visualization of the FEM mesh.
    // This will automatically update a triangle mesh (a ChTriangleMeshShape
    // asset that is internally managed) by setting  proper
    // coordinates and vertex colours as in the FEM elements.
    // Such triangle mesh can be rendered by Irrlicht or POVray or whatever
    // postprocessor that can handle a coloured ChTriangleMeshShape).
    // Do not forget AddAsset() at the end!

    auto mvisualizeshellA = std::make_shared<ChVisualizationFEAmesh>(*(my_mesh.get()));
    mvisualizeshellA->SetSmoothFaces(true);
	mvisualizeshellA->SetWireframe(true);
	my_mesh->AddAsset(mvisualizeshellA);

    auto mvisualizeshellB = std::make_shared<ChVisualizationFEAmesh>(*(my_mesh.get()));
    mvisualizeshellB->SetFEMdataType(ChVisualizationFEAmesh::E_PLOT_NONE);
    mvisualizeshellB->SetFEMglyphType(ChVisualizationFEAmesh::E_GLYPH_NODE_DOT_POS);
    mvisualizeshellB->SetSymbolsThickness(0.006);
    my_mesh->AddAsset(mvisualizeshellB);

    auto mvisualizeshellC = std::make_shared<ChVisualizationFEAmesh>(*(my_mesh.get()));
    mvisualizeshellC->SetFEMdataType(ChVisualizationFEAmesh::E_PLOT_NONE);
    mvisualizeshellC->SetFEMglyphType(ChVisualizationFEAmesh::E_GLYPH_NODE_CSYS);
    mvisualizeshellC->SetSymbolsThickness(0.02);
    mvisualizeshellC->SetZbufferHide(false);
    my_mesh->AddAsset(mvisualizeshellC);

    // ==IMPORTANT!== Use this function for adding a ChIrrNodeAsset to all items
    // in the system. These ChIrrNodeAsset assets are 'proxies' to the Irrlicht meshes.
    // If you need a finer control on which item really needs a visualization proxy in
    // Irrlicht, just use application.AssetBind(myitem); on a per-item basis.

    application.AssetBindAll();

    // ==IMPORTANT!== Use this function for 'converting' into Irrlicht meshes the assets
    // that you added to the bodies into 3D shapes, they can be visualized by Irrlicht!

    application.AssetUpdateAll();

    // Mark completion of system construction
    my_system.SetupInitial();

    //
    // THE SOFT-REAL-TIME CYCLE
    //
    // Change solver to MKL
    ChLcpMklSolver* mkl_solver_stab = new ChLcpMklSolver;
    ChLcpMklSolver* mkl_solver_speed = new ChLcpMklSolver;
    my_system.ChangeLcpSolverStab(mkl_solver_stab);
    my_system.ChangeLcpSolverSpeed(mkl_solver_speed);
	mkl_solver_stab->SetSparsityPatternLock(true);
	mkl_solver_speed->SetSparsityPatternLock(true);
    /*
    my_system.SetLcpSolverType(ChSystem::LCP_ITERATIVE_MINRES); // <- NEEDED THIS or Matlab or MKL solver
	my_system.SetIterLCPwarmStarting(true); // this helps a lot to speedup convergence in this class of problems
	my_system.SetIterLCPmaxItersSpeed(200);
	my_system.SetIterLCPmaxItersStab(200);
	my_system.SetTolForce(1e-13);
	chrono::ChLcpIterativeMINRES* msolver = (chrono::ChLcpIterativeMINRES*)my_system.GetLcpSolverSpeed();
	msolver->SetVerbose(false);
	msolver->SetDiagonalPreconditioning(true);
    */

    // Change type of integrator:
    my_system.SetIntegrationType(chrono::ChSystem::INT_EULER_IMPLICIT_LINEARIZED);  // fast, less precise
 /*   
    my_system.SetIntegrationType(chrono::ChSystem::INT_HHT);  // precise,slower, might iterate each step

    // if later you want to change integrator settings:
    if (auto mystepper = std::dynamic_pointer_cast<ChTimestepperHHT>(my_system.GetTimestepper())) {
        mystepper->SetAlpha(-0.2);
        mystepper->SetMaxiters(5);
        mystepper->SetAbsTolerances(1e-6);
    }
*/    
    application.SetTimestep(0.01);
    application.SetPaused(true);
    my_system.Setup();
    my_system.Update();
    //my_system.DoStaticLinear();

    while (application.GetDevice()->run()) {
        application.BeginScene();

        application.DrawAll();

        // .. draw also a grid
        ChIrrTools::drawGrid(application.GetVideoDriver(), 1, 1);

        application.DoStep();

        if (false)
        if(!application.GetPaused()) {
            GetLog() << "\n\n Time = " << application.GetSystem()->GetChTime();

            if (node3) 
                GetLog() << "Node 3 pos:" << node3->GetPos() << "Node 3 rot: " << node3->GetRot().Q_to_Rotv() << "\n";
            if (node4) 
                GetLog() << "Node 4 pos:" << node4->GetPos() << "Node 4 rot: " << node4->GetRot().Q_to_Rotv() << "\n\n";
        }

        application.EndScene();
    }

    return 0;
}



// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban
// =============================================================================
//
// =============================================================================

#include "assets/ChColorAsset.h"
#include "geometry/ChCLineBezier.h"

#include "utils/ChUtilsInputOutput.h"

namespace chrono {
namespace utils {

// -----------------------------------------------------------------------------
// WriteBodies
//
// Write to a CSV file pody position, orientation, and (optionally) linear and
// angular velocity. Optionally, only active bodies are processed.
// -----------------------------------------------------------------------------
void WriteBodies(ChSystem* system,
                 const std::string& filename,
                 bool active_only,
                 bool dump_vel,
                 const std::string& delim) {
    CSV_writer csv(delim);

    for (int i = 0; i < system->Get_bodylist()->size(); i++) {
        std::shared_ptr<ChBody> body = system->Get_bodylist()->at(i);
        if (active_only && !body->IsActive())
            continue;
        csv << body->GetPos() << body->GetRot();
        if (dump_vel)
            csv << body->GetPos_dt() << body->GetWvel_loc();
        csv << std::endl;
    }

    csv.write_to_file(filename);
}

// -----------------------------------------------------------------------------
// WriteCheckpoint
//
// Create a CSV file with a checkpoint ...
//
// -----------------------------------------------------------------------------
bool WriteCheckpoint(ChSystem* system, const std::string& filename) {
    // Create the CSV stream.
    CSV_writer csv(" ");

    std::vector<std::shared_ptr<ChBody> >::iterator ibody = system->Get_bodylist()->begin();
    for (; ibody != system->Get_bodylist()->end(); ++ibody) {
        std::shared_ptr<ChBody> body = *ibody;

        // Infer body type (0: DVI, 1:DEM)
        int btype = (body->GetContactMethod() == ChMaterialSurfaceBase::DVI) ? 0 : 1;

        // Write body type, body identifier, the body fixed flag, and the collide flag
        csv << btype << body->GetIdentifier() << body->GetBodyFixed() << body->GetCollide();

        // Write collision family information.
        csv << body->GetCollisionModel()->GetFamilyGroup() << body->GetCollisionModel()->GetFamilyMask();

        // Write body mass and inertia
        csv << body->GetMass() << body->GetInertiaXX();

        // Write body position, orientation, and their time derivatives
        csv << body->GetPos() << body->GetRot();
        csv << body->GetPos_dt() << body->GetRot_dt();

        csv << std::endl;

        // Write material information
        if (btype == 0) {
            // Write DVI material surface information
            std::shared_ptr<ChMaterialSurface> mat = body->GetMaterialSurface();
            csv << mat->static_friction << mat->sliding_friction << mat->rolling_friction << mat->spinning_friction;
            csv << mat->restitution << mat->cohesion << mat->dampingf;
            csv << mat->compliance << mat->complianceT << mat->complianceRoll << mat->complianceSpin;
        } else {
            // Write DEM material surface information
            std::shared_ptr<ChMaterialSurfaceDEM> mat = body->GetMaterialSurfaceDEM();
            csv << mat->young_modulus << mat->poisson_ratio;
            csv << mat->static_friction << mat->sliding_friction;
            csv << mat->restitution << mat->constant_adhesion << mat->adhesionMultDMT;
        }

        csv << std::endl;

        // Count and write all visual assets.
        int num_visual_assets = 0;
        std::vector<std::shared_ptr<ChAsset> >::iterator iasset = (*ibody)->GetAssets().begin();
        for (; iasset != (*ibody)->GetAssets().end(); ++iasset) {
            if (std::dynamic_pointer_cast<ChVisualization>(*iasset))
                num_visual_assets++;
        }
        csv << num_visual_assets << std::endl;

        // Loop over each asset and, for selected visual assets only, write its data
        // on a separate line. If we encounter an unsupported type, return false.
        iasset = (*ibody)->GetAssets().begin();
        for (; iasset != (*ibody)->GetAssets().end(); ++iasset) {
            auto visual_asset = std::dynamic_pointer_cast<ChVisualization>(*iasset);
            if (!visual_asset)
                continue;

            // Write relative position and rotation
            csv << visual_asset->Pos << visual_asset->Rot.Get_A_quaternion();

            // Write shape type and geometry data
            if (auto sphere = std::dynamic_pointer_cast<ChSphereShape>(visual_asset)) {
                csv << collision::SPHERE << sphere->GetSphereGeometry().rad;
            } else if (auto ellipsoid = std::dynamic_pointer_cast<ChEllipsoidShape>(visual_asset)) {
                csv << collision::ELLIPSOID << ellipsoid->GetEllipsoidGeometry().rad;
            } else if (auto box = std::dynamic_pointer_cast<ChBoxShape>(visual_asset)) {
                csv << collision::BOX << box->GetBoxGeometry().Size;
            } else if (auto capsule = std::dynamic_pointer_cast<ChCapsuleShape>(visual_asset)) {
                const geometry::ChCapsule& geom = capsule->GetCapsuleGeometry();
                csv << collision::CAPSULE << geom.rad << geom.hlen;
            } else if (auto cylinder = std::dynamic_pointer_cast<ChCylinderShape>(visual_asset)) {
                const geometry::ChCylinder& geom = cylinder->GetCylinderGeometry();
                csv << collision::CYLINDER << geom.rad << (geom.p1.y - geom.p2.y) / 2;
            } else if (auto cone = std::dynamic_pointer_cast<ChConeShape>(visual_asset)) {
                const geometry::ChCone& geom = cone->GetConeGeometry();
                csv << collision::CONE << geom.rad.x << geom.rad.y;
            } else if (auto rbox = std::dynamic_pointer_cast<ChRoundedBoxShape>(visual_asset)) {
                const geometry::ChRoundedBox& geom = rbox->GetRoundedBoxGeometry();
                csv << collision::ROUNDEDBOX << geom.Size << geom.radsphere;
            } else if (auto rcyl = std::dynamic_pointer_cast<ChRoundedCylinderShape>(visual_asset)) {
                const geometry::ChRoundedCylinder& geom = rcyl->GetRoundedCylinderGeometry();
                csv << collision::ROUNDEDCYL << geom.rad << geom.hlen << geom.radsphere;
            } else {
                // Unsupported visual asset type.
                return false;
            }
            csv << std::endl;
        }
    }

    csv.write_to_file(filename);

    return true;
}

// -----------------------------------------------------------------------------
// ReadCheckpoint
//
// Read a CSV file with checkpoint data and create the bodies.
//
// -----------------------------------------------------------------------------
void ReadCheckpoint(ChSystem* system, const std::string& filename) {
    // Open input file stream
    std::ifstream ifile(filename.c_str());
    std::string line;

    while (std::getline(ifile, line)) {
        std::istringstream iss1(line);

        // Read body type, Id, flags
        int btype, bid, bfixed, bcollide;
        short family_group, family_mask;
        iss1 >> btype >> bid >> bfixed >> bcollide >> family_group >> family_mask;

        // Read body mass and inertia
        double mass;
        ChVector<> inertiaXX;
        iss1 >> mass >> inertiaXX.x >> inertiaXX.y >> inertiaXX.z;

        // Read body position, orientation, and their time derivatives
        ChVector<> bpos, bpos_dt;
        ChQuaternion<> brot, brot_dt;
        iss1 >> bpos.x >> bpos.y >> bpos.z >> brot.e0 >> brot.e1 >> brot.e2 >> brot.e3;
        iss1 >> bpos_dt.x >> bpos_dt.y >> bpos_dt.z >> brot_dt.e0 >> brot_dt.e1 >> brot_dt.e2 >> brot_dt.e3;

        // Get the next line in the file (material properties)
        std::getline(ifile, line);
        std::istringstream iss2(line);

        // Create a body of the appropriate type, read and apply material properties
        ChBody* body = system->NewBody();
        if (btype == 0) {
            std::shared_ptr<ChMaterialSurface> mat = body->GetMaterialSurface();
            iss2 >> mat->static_friction >> mat->sliding_friction >> mat->rolling_friction >> mat->spinning_friction;
            iss2 >> mat->restitution >> mat->cohesion >> mat->dampingf;
            iss2 >> mat->compliance >> mat->complianceT >> mat->complianceRoll >> mat->complianceSpin;
        } else {
            std::shared_ptr<ChMaterialSurfaceDEM> mat = body->GetMaterialSurfaceDEM();
            iss2 >> mat->young_modulus >> mat->poisson_ratio;
            iss2 >> mat->static_friction >> mat->sliding_friction;
            iss2 >> mat->restitution >> mat->constant_adhesion >> mat->adhesionMultDMT;
        }

        // Add the body to the system.
        system->AddBody(std::shared_ptr<ChBody>(body));

        // Set body properties and state
        body->SetPos(bpos);
        body->SetRot(brot);
        body->SetPos_dt(bpos_dt);
        body->SetRot_dt(brot_dt);

        body->SetIdentifier(bid);
        body->SetBodyFixed(bfixed != 0);
        body->SetCollide(bcollide != 0);

        body->SetMass(mass);
        body->SetInertiaXX(inertiaXX);

        // Get next line in the file (number of visualization assets)
        std::getline(ifile, line);
        std::istringstream iss3(line);

        int numAssets;
        iss3 >> numAssets;

        // In a loop, read information about each asset and add geometry to the body
        body->GetCollisionModel()->ClearModel();

        for (int j = 0; j < numAssets; j++) {
            std::getline(ifile, line);
            std::istringstream iss(line);

            // Get relative position and rotation
            ChVector<> apos;
            ChQuaternion<> arot;
            iss >> apos.x >> apos.y >> apos.z >> arot.e0 >> arot.e1 >> arot.e2 >> arot.e3;

            // Get visualization asset type and geometry data.
            // Create the appropriate shape (both visualization and contact).
            int atype;
            iss >> atype;

            switch (collision::ShapeType(atype)) {
                case collision::SPHERE: {
                    double radius;
                    iss >> radius;
                    AddSphereGeometry(body, radius, apos, arot);
                } break;
                case collision::ELLIPSOID: {
                    ChVector<> size;
                    iss >> size.x >> size.y >> size.z;
                    AddEllipsoidGeometry(body, size, apos, arot);
                } break;
                case collision::BOX: {
                    ChVector<> size;
                    iss >> size.x >> size.y >> size.z;
                    AddBoxGeometry(body, size, apos, arot);
                } break;
                case collision::CAPSULE: {
                    double radius, hlen;
                    iss >> radius >> hlen;
                    AddCapsuleGeometry(body, radius, hlen, apos, arot);
                } break;
                case collision::CYLINDER: {
                    double radius, hlen;
                    iss >> radius >> hlen;
                    AddCylinderGeometry(body, radius, hlen, apos, arot);
                } break;
                case collision::CONE: {
                    double radius, height;
                    iss >> radius >> height;
                    AddConeGeometry(body, radius, height, apos, arot);
                } break;
                case collision::ROUNDEDBOX: {
                    ChVector<> size;
                    double srad;
                    iss >> size.x >> size.y >> size.z >> srad;
                    AddRoundedBoxGeometry(body, size, srad, apos, arot);
                } break;
                case collision::ROUNDEDCYL: {
                    double radius, hlen, srad;
                    iss >> radius >> hlen >> srad;
                    AddRoundedCylinderGeometry(body, radius, hlen, srad, apos, arot);
                } break;
            }
        }

        // Set the collision family group and the collision family mask.
        body->GetCollisionModel()->SetFamilyGroup(family_group);
        body->GetCollisionModel()->SetFamilyMask(family_mask);

        // Complete construction of the collision model.
        body->GetCollisionModel()->BuildModel();
    }
}

// -----------------------------------------------------------------------------
// WriteShapesPovray
//
// Write CSV output file for PovRay.
// First line contains the number of visual assets and links to follow.
// A line with information about a visualization asset contains:
//    bodyId, bodyActive, x, y, z, e0, e1, e2, e3, shapeType, [shape Data]
// A line with information about a link contains:
//    linkType, [linkData]
//
// NOTE: we do not account for any transform specified for the ChGeometry of
// a visual asset (except for cylinders where that is implicit)!
// -----------------------------------------------------------------------------
enum POVRayShapeType {
    SPHERE,
    ELLIPSOID,
    BOX,
    CYLINDER,
    CONVEXHULL,
    TRIANGLEMESH,
    BARREL,
    CAPSULE,
    CONE,
    ROUNDEDBOX,
    ROUNDEDCYL,
    ROUNDEDCONE,
    BEZIER
};

void WriteShapesPovray(ChSystem* system, const std::string& filename, bool body_info, const std::string& delim) {
    CSV_writer csv(delim);

    // If requested, Loop over all bodies and write out their position and
    // orientation.  Otherwise, body count is left at 0.
    int b_count = 0;

    if (body_info) {
        std::vector<std::shared_ptr<ChBody> >::iterator ibody = system->Get_bodylist()->begin();
        for (; ibody != system->Get_bodylist()->end(); ++ibody) {
            const ChVector<>& body_pos = (*ibody)->GetFrame_REF_to_abs().GetPos();
            const ChQuaternion<>& body_rot = (*ibody)->GetFrame_REF_to_abs().GetRot();

            csv << (*ibody)->GetIdentifier() << (*ibody)->IsActive() << body_pos << body_rot << std::endl;

            b_count++;
        }
    }

    // Loop over all bodies and over all their assets.
    int a_count = 0;
    std::vector<std::shared_ptr<ChBody> >::iterator ibody = system->Get_bodylist()->begin();
    for (; ibody != system->Get_bodylist()->end(); ++ibody) {
        const ChVector<>& body_pos = (*ibody)->GetFrame_REF_to_abs().GetPos();
        const ChQuaternion<>& body_rot = (*ibody)->GetFrame_REF_to_abs().GetRot();

        ChColor color(0.8f, 0.8f, 0.8f);

        // First loop over assets -- search for a color asset
        std::vector<std::shared_ptr<ChAsset> >::iterator iasset = (*ibody)->GetAssets().begin();
        for (; iasset != (*ibody)->GetAssets().end(); ++iasset) {
            if (auto color_asset = std::dynamic_pointer_cast<ChColorAsset>(*iasset))
                color = color_asset->GetColor();
        }

        // Loop over assets once again -- write information for supported types.
        iasset = (*ibody)->GetAssets().begin();
        for (; iasset != (*ibody)->GetAssets().end(); ++iasset) {
            auto visual_asset = std::dynamic_pointer_cast<ChVisualization>(*iasset);
            if (!visual_asset)
                continue;

            const Vector& asset_pos = visual_asset->Pos;
            Quaternion asset_rot = visual_asset->Rot.Get_A_quaternion();

            Vector pos = body_pos + body_rot.Rotate(asset_pos);
            Quaternion rot = body_rot % asset_rot;

            bool supported = true;
            std::stringstream gss;

            if (auto sphere = std::dynamic_pointer_cast<ChSphereShape>(visual_asset)) {
                gss << SPHERE << delim << sphere->GetSphereGeometry().rad;
                a_count++;
            } else if (auto ellipsoid = std::dynamic_pointer_cast<ChEllipsoidShape>(visual_asset)) {
                const Vector& size = ellipsoid->GetEllipsoidGeometry().rad;
                gss << ELLIPSOID << delim << size.x << delim << size.y << delim << size.z;
                a_count++;
            } else if (auto box = std::dynamic_pointer_cast<ChBoxShape>(visual_asset)) {
                const Vector& size = box->GetBoxGeometry().Size;
                gss << BOX << delim << size.x << delim << size.y << delim << size.z;
                a_count++;
            } else if (auto capsule = std::dynamic_pointer_cast<ChCapsuleShape>(visual_asset)) {
                const geometry::ChCapsule& geom = capsule->GetCapsuleGeometry();
                gss << CAPSULE << delim << geom.rad << delim << geom.hlen;
                a_count++;
            } else if (auto cylinder = std::dynamic_pointer_cast<ChCylinderShape>(visual_asset)) {
                const geometry::ChCylinder& geom = cylinder->GetCylinderGeometry();
                gss << CYLINDER << delim << geom.rad << delim << geom.p1.x << delim << geom.p1.y << delim << geom.p1.z
                    << delim << geom.p2.x << delim << geom.p2.y << delim << geom.p2.z;
                a_count++;
            } else if (auto cone = std::dynamic_pointer_cast<ChConeShape>(visual_asset)) {
                const geometry::ChCone& geom = cone->GetConeGeometry();
                gss << CONE << delim << geom.rad.x << delim << geom.rad.y;
                a_count++;
            } else if (auto rbox = std::dynamic_pointer_cast<ChRoundedBoxShape>(visual_asset)) {
                const geometry::ChRoundedBox& geom = rbox->GetRoundedBoxGeometry();
                gss << ROUNDEDBOX << delim << geom.Size.x << delim << geom.Size.y << delim << geom.Size.z << delim
                    << geom.radsphere;
                a_count++;
            } else if (auto rcyl = std::dynamic_pointer_cast<ChRoundedCylinderShape>(visual_asset)) {
                const geometry::ChRoundedCylinder& geom = rcyl->GetRoundedCylinderGeometry();
                gss << ROUNDEDCYL << delim << geom.rad << delim << geom.hlen << delim << geom.radsphere;
                a_count++;
            } else if (auto mesh = std::dynamic_pointer_cast<ChTriangleMeshShape>(visual_asset)) {
                gss << TRIANGLEMESH << delim << "\"" << mesh->GetName() << "\"";
                a_count++;
            } else if (auto line = std::dynamic_pointer_cast<ChLineShape>(visual_asset)) {
                std::shared_ptr<geometry::ChLine> geom = line->GetLineGeometry();
                if (auto bezier = std::dynamic_pointer_cast<geometry::ChLineBezier>(geom)) {
                    gss << BEZIER << delim << "\"" << line->GetName() << "\"";
                    a_count++;
                } else {
                    supported = false;
                }
            } else {
                supported = false;
            }

            if (supported) {
                csv << (*ibody)->GetIdentifier() << (*ibody)->IsActive() << pos << rot << color << gss.str()
                    << std::endl;
            }
        }
    }

    // Loop over all links.  Write information on selected types of links.
    int l_count = 0;
    std::vector<std::shared_ptr<ChLink> >::iterator ilink = system->Get_linklist()->begin();
    for (; ilink != system->Get_linklist()->end(); ++ilink) {
        int type = (*ilink)->GetType();

        if (ChLinkLockRevolute* link = dynamic_cast<ChLinkLockRevolute*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << frA_abs.GetA().Get_A_Zaxis() << std::endl;
            l_count++;
        } else if (ChLinkLockSpherical* link = dynamic_cast<ChLinkLockSpherical*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << std::endl;
            l_count++;
        }
        if (ChLinkLockPrismatic* link = dynamic_cast<ChLinkLockPrismatic*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << frA_abs.GetA().Get_A_Zaxis() << std::endl;
            l_count++;
        } else if (ChLinkUniversal* link = dynamic_cast<ChLinkUniversal*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = link->GetFrame1Abs();
            chrono::ChFrame<> frB_abs = link->GetFrame2Abs();

            csv << type << frA_abs.GetPos() << frA_abs.GetA().Get_A_Xaxis() << frB_abs.GetA().Get_A_Yaxis()
                << std::endl;
            l_count++;
        } else if (ChLinkSpring* link = dynamic_cast<ChLinkSpring*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << frB_abs.GetPos() << std::endl;
            l_count++;
        } else if (ChLinkSpringCB* link = dynamic_cast<ChLinkSpringCB*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << frB_abs.GetPos() << std::endl;
            l_count++;
        } else if (ChLinkDistance* link = dynamic_cast<ChLinkDistance*>((*ilink).get())) {
            csv << type << link->GetEndPoint1Abs() << link->GetEndPoint2Abs() << std::endl;
            l_count++;
        } else if (ChLinkEngine* link = dynamic_cast<ChLinkEngine*>((*ilink).get())) {
            chrono::ChFrame<> frA_abs = *(link->GetMarker1()) >> *(link->GetBody1());
            chrono::ChFrame<> frB_abs = *(link->GetMarker2()) >> *(link->GetBody2());

            csv << type << frA_abs.GetPos() << frA_abs.GetA().Get_A_Zaxis() << std::endl;
            l_count++;
        }
    }

    // Write the output file, including a first line with number of bodies, visual
    // assets, and links.
    std::stringstream header;
    header << b_count << delim << a_count << delim << l_count << delim << std::endl;

    csv.write_to_file(filename, header.str());
}

// -----------------------------------------------------------------------------
// WriteMeshPovray
//
// Write the triangular mesh from the specified OBJ file as a macro in a PovRay
// include file.
// -----------------------------------------------------------------------------
void WriteMeshPovray(geometry::ChTriangleMeshConnected trimesh,
                     const std::string& mesh_name,
                     const std::string& out_dir,
                     const ChColor& col,
                     const ChVector<>& pos,
                     const ChQuaternion<>& rot,
                     bool smoothed) {
    // Transform vertices.
    for (unsigned int i = 0; i < trimesh.m_vertices.size(); i++)
        trimesh.m_vertices[i] = pos + rot.Rotate(trimesh.m_vertices[i]);

    // Transform normals
    if (smoothed) {
        for (unsigned int i = 0; i < trimesh.m_normals.size(); i++)
            trimesh.m_normals[i] = rot.Rotate(trimesh.m_normals[i]);
    }

    // Open output file.
    std::string pov_filename = out_dir + "/" + mesh_name + ".inc";
    std::ofstream ofile(pov_filename.c_str());

    ofile << "#declare " << mesh_name << "_mesh = mesh2 {" << std::endl;

    // Write vertices.
    ofile << "vertex_vectors {" << std::endl;
    ofile << trimesh.m_vertices.size();
    for (unsigned int i = 0; i < trimesh.m_vertices.size(); i++) {
        ChVector<> v = trimesh.m_vertices[i];
        ofile << ",\n<" << v.x << ", " << v.z << ", " << v.y << ">";
    }
    ofile << "\n}" << std::endl;

    // Write normals.
    if (smoothed) {
        ofile << "normal_vectors {" << std::endl;
        ofile << trimesh.m_normals.size();
        for (unsigned int i = 0; i < trimesh.m_normals.size(); i++) {
            ChVector<> n = trimesh.m_normals[i];
            ofile << ",\n<" << n.x << ", " << n.z << ", " << n.y << ">";
        }
        ofile << "\n}" << std::endl;
    }

    // Write face connectivity.
    ofile << "face_indices {" << std::endl;
    ofile << trimesh.m_face_v_indices.size();
    for (int i = 0; i < trimesh.m_face_v_indices.size(); i++) {
        ChVector<int> face = trimesh.m_face_v_indices[i];
        ofile << ",\n<" << face.x << ", " << face.y << ", " << face.z << ">";
    }
    ofile << "\n}" << std::endl;

    ofile << "\n}" << std::endl;

    // Write the object
    ofile << "#declare " << mesh_name << " = object {" << std::endl;

    ofile << "   " << mesh_name << "_mesh" << std::endl;
    ofile << "   texture {" << std::endl;
    ofile << "      pigment {color rgb<" << col.R << ", " << col.G << ", " << col.B << ">}" << std::endl;
    ofile << "      finish  {phong 0.2  diffuse 0.6}" << std::endl;
    ofile << "    }" << std::endl;
    ofile << "}" << std::endl;

    // Close the output file.
    ofile.close();
}

void WriteMeshPovray(const std::string& obj_filename,
                     const std::string& mesh_name,
                     const std::string& out_dir,
                     const ChColor& col,
                     const ChVector<>& pos,
                     const ChQuaternion<>& rot) {
    // Read trimesh from OBJ file
    geometry::ChTriangleMeshConnected trimesh;
    trimesh.LoadWavefrontMesh(obj_filename, false, false);

    // Generate output
    WriteMeshPovray(trimesh, mesh_name, out_dir, col, pos, rot);
}

// -----------------------------------------------------------------------------
// WriteCurvePovray
//
// Write the specified Bezier curve as a macro in a PovRay include file.
// -----------------------------------------------------------------------------
void WriteCurvePovray(const ChBezierCurve& curve,
                      const std::string& curve_name,
                      const std::string& out_dir,
                      double radius,
                      const ChColor& col) {
    int nP = 20;
    double dt = 1.0 / nP;
    size_t nS = curve.getNumPoints() - 1;

    // Open output file.
    std::string pov_filename = out_dir + "/" + curve_name + ".inc";
    std::ofstream ofile(pov_filename.c_str());

    ofile << "#declare " << curve_name << " = object {" << std::endl;
    ofile << "  sphere_sweep {" << std::endl;
    ofile << "    linear_spline " << nP* nS + 1 << "," << std::endl;

    ChVector<> v = curve.eval(0, 0.0);
    ofile << "        <" << v.x << ", " << v.z << ", " << v.x << "> ," << radius << std::endl;

    for (int iS = 0; iS < nS; iS++) {
        for (int iP = 1; iP <= nP; iP++) {
            v = curve.eval(iS, iP * dt);
            ofile << "        <" << v.x << ", " << v.z << ", " << v.y << "> ," << radius << std::endl;
        }
    }

    ofile << "    texture {" << std::endl;
    ofile << "      pigment {color rgb<" << col.R << ", " << col.G << ", " << col.B << ">}" << std::endl;
    ofile << "      finish  {phong 0.2  diffuse 0.6}" << std::endl;
    ofile << "     }" << std::endl;

    ofile << "  }" << std::endl;
    ofile << "}" << std::endl;

    // Close the output file.
    ofile.close();
}

}  // namespace utils
}  // namespace chrono

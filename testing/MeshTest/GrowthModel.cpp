/*
 * GrowthModel.cpp
 *
 *  Created on: Oct 13, 2017
 *      Author: andy
 */

#include "GrowthModel.h"
#include <MxMeshGmshImporter.h>
#include <MxDebug.h>
#include <iostream>
#include "MxMeshVoronoiImporter.h"
#include "MeshTest.h"


static struct RedCellType : MxCellType
{
    virtual Magnum::Color4 color(struct MxCell *cell) {
        return Color4{1.0f, 0.0f, 0.0f, 0.08f};
    }
} redCellType;

static struct BlueCellType : MxCellType
{
    virtual Magnum::Color4 color(struct MxCell *cell) {
        return Color4{0.0f, 0.0f, 1.0f, 0.08f};
    }
} blueCellType;

static struct ClearCellType : MxCellType
{
    virtual Magnum::Color4 color(struct MxCell *cell) {
        switch(cell->id) {
            case 4: return Color4{0.0f, 0.0f, 1.0f, 0.08f};
            default: return Color4{0.0f, 0.0f, 0.0f, 0.00f};
        }
        
    }
} clearCellType;



GrowthModel::GrowthModel()  {

    //loadMonodisperseVoronoiModel();
    //loadSimpleSheetModel();
    //loadSheetModel();
    loadCubeModel();
    
    /*
    Matrix4 rot = Matrix4::rotationY(Rad{3.14/3});
    
    for(int i = 0; i < mesh->vertices.size(); ++i) {
        mesh->vertices[i]->position = rot.transformPoint(mesh->vertices[i]->position);
        
    }
     */

    testEdges();
}

//const float targetArea = 0.45;

//const float targetArea = 2.0;




HRESULT GrowthModel::calcForce() {

    HRESULT result;

    for(CellPtr cell : mesh->cells) {
        if((result = cellAreaForce(cell)) != S_OK) {
            return mx_error(result, "cell area force");
        }

        if((result = cellVolumeForce(cell)) != S_OK) {
            return mx_error(result, "cell volume force");
        }
    }

    return S_OK;
}

HRESULT GrowthModel::cellAreaForce(CellPtr cell) {

    //return S_OK;

    if(mesh->rootCell() == cell) {
        return S_OK;
    }

    assert(cell->area >= 0);

    float diff =  - cell->area;
    //float diff = -0.35;

    for(auto pt: cell->boundary) {

        TrianglePtr tri = pt->triangle;

        assert(tri->area >= 0);
        
        //float areaFraction = tri->area / cell->area;

        //std::cout << "id: " << tri->id << ",AR " << tri->aspectRatio << std::endl;

        Vector3 dir[3];
        float len[3];
        float totLen = 0;
        for(int v = 0; v < 3; ++v) {
            dir[v] = tri->vertices[v]->position - tri->centroid;
            //dir[v] = ((tri->vertices[v]->position - tri->vertices[(v+1)%3]->position) +
            //          (tri->vertices[v]->position - tri->vertices[(v+2)%3]->position)) / 2;
            len[v] = dir[v].length();
            //dir[v] = dir[v].normalized();
            totLen += len[v];
        }
        
        

        for(int v = 0; v < 3; ++v) {
            //pt->force[v] -= surfaceTension * (tri->vertices[v]->position - tri->centroid).normalized();
            
            Vector3 p1 = tri->vertices[(v+1)%3]->position;
            Vector3 p2 = tri->vertices[(v+2)%3]->position;
            float len = (p1-p2).length();
            pt->force[v] -= surfaceTension * len * (tri->vertices[v]->position - tri->centroid).normalized();
            //pt->force[v] -= surfaceTension * (tri->vertices[v]->position - tri->centroid);
            
            //for(int i = 0; i < 3; ++i) {
            //    if(i != v) {
            //        pt->force[v] -= 0.5 * (tri->vertices[v]->position - tri->vertices[i]->position);
            //    }
            //}
            //pt->force[v] +=  -100 * areaFraction * dir[v] / totLen;
            //pt->force[v] +=  10.5 * diff * (tri->area / cell->area) *  dir[v].normalized();
            // pt->force[v] +=  -30.5 * (pt->triangle->vertices[v]->position - pt->triangle->centroid);
            //pt->force[v] += -300 * areaFraction * (pt->triangle->vertices[v]->position - pt->triangle->centroid).normalized();
            
            //for(int o = 0; o < 3; ++o) {
            //    if(o != v) {
            //        pt->force[v] +=  -10.5 * (pt->triangle->vertices[v]->position - pt->triangle->vertices[o]->position);
            //    }
            //}
        }

    }
    return S_OK;
}

HRESULT GrowthModel::cellVolumeForce(CellPtr cell)
{

    if(mesh->rootCell() == cell) {
        return S_OK;
    }

    assert(cell->area >= 0);
    
    //std::cout << "cell id: " << cell->id << ", volume: " << cell->volume << std::endl;
    
    if(this->volumeForceType == ConstantVolume) {
        
        float diff = targetVolume - cell->volume;
        
        
        diff = (targetVolumeLambda / cell->volume) * diff;
        
        for(auto pt: cell->boundary) {
            TrianglePtr tri = pt->triangle;
            Vector3 normal = tri->cellNormal(cell);
            Vector3 force = (pressure + diff) * tri->area * normal;
            
            for(int v = 0; v < 3; ++v) {
                pt->force[v] +=  force;
            }
        }
    }
    
    else {
        for(auto pt: cell->boundary) {
            TrianglePtr tri = pt->triangle;
            Vector3 normal = tri->cellNormal(cell);
            Vector3 force = pressure * tri->area * normal;
            
            for(int v = 0; v < 3; ++v) {
                pt->force[v] +=  force;
            }
        }
    }


    



    return S_OK;
}

void GrowthModel::testEdges() {

    return;
}

void GrowthModel::loadSheetModel() {
    mesh = new MxMesh();

    MxMeshGmshImporter importer{*mesh,
        [](Gmsh::ElementType, int id) {
            if((id % 2) == 0) {
                return (MxCellType*)&redCellType;
            } else {
                return (MxCellType*)&blueCellType;
            }
        }
    };

    mesh->setShortCutoff(0.02);
    //mesh->setLongCutoff(0.12);
    mesh->setLongCutoff(0.15);
    importer.read("/Users/andy/src/mechanica/testing/gmsh1/sheet.msh");
    
    pressureMin = 0;
    pressure = 5;
    pressureMax = 10;
    
    surfaceTensionMin = 0;
    surfaceTension = 1;
    surfaceTensionMax = 6;
    
    targetVolume = 0.04;
    minTargetVolume = 0.005;
    maxTargetVolume = 0.06;
    targetVolumeLambda = 500.;
}

void GrowthModel::loadSimpleSheetModel() {
    mesh = new MxMesh();



    MxMeshGmshImporter importer{*mesh,
        [](Gmsh::ElementType, int id) {
            
            if((id % 2) == 0) {
                return (MxCellType*)&redCellType;
            } else {
                return (MxCellType*)&blueCellType;
            }
            
            //return (id == 16) ? (MxCellType*)&blueCellType : (MxCellType*)&clearCellType;
        }
    };


    mesh->setShortCutoff(0.01);
    mesh->setLongCutoff(0.45);
    importer.read("/Users/andy/src/mechanica/testing/MeshTest/simplesheet.msh");

    pressureMin = 0;
    pressure = 5;
    pressureMax = 15;
    
    surfaceTensionMin = 0;
    surfaceTension = 4;
    surfaceTensionMax = 15;
    
    targetVolume = 0.6;
    minTargetVolume = 0.005;
    maxTargetVolume = 10.0;
    targetVolumeLambda = 5.;

}

void GrowthModel::loadCubeModel() {

    mesh = new MxMesh();

    MxMeshGmshImporter importer{*mesh,
        [](Gmsh::ElementType, int id) {
            return (MxCellType*)&redCellType;
        }
    };

    mesh->setShortCutoff(0);
    mesh->setLongCutoff(0.3);
    importer.read("/Users/andy/src/mechanica/testing/MeshTest/cube.msh");
    
    pressureMin = 0;
    pressure = 5;
    pressureMax = 15;
    
    surfaceTensionMin = 0;
    surfaceTension = 5;
    surfaceTensionMax = 15;
    
    targetVolume = 0.6;
    minTargetVolume = 0.005;
    maxTargetVolume = 10.0;
    targetVolumeLambda = 5.;
}

void GrowthModel::loadMonodisperseVoronoiModel() {
    mesh = new MxMesh();

    MxMeshVoronoiImporter importer(*mesh);

    importer.monodisperse();
}

HRESULT GrowthModel::getForces(float time, uint32_t len, const Vector3* pos, Vector3* force)
{
    HRESULT result;

    if(len != mesh->vertices.size()) {
        return E_FAIL;
    }
    
    if(pos) {
        if(!SUCCEEDED(result = mesh->setPositions(len, pos))) {
            return result;
        }
    }

    calcForce();

    for(int i = 0; i < mesh->vertices.size(); ++i) {
        VertexPtr v = mesh->vertices[i];

        assert(v->mass > 0 && v->area > 0);

        for(CTrianglePtr tri : v->triangles()) {
            for(int j = 0; j < 3; ++j) {
                if(tri->vertices[j] == v) {
                    force[i] += tri->force(j);
                }
            }
        }
    }


    return S_OK;
}

HRESULT GrowthModel::getMasses(float time, uint32_t len, float* masses)
{
    if(len != mesh->vertices.size()) {
        return E_FAIL;
    }

    for(int i = 0; i < len; ++i) {
        masses[i] = mesh->vertices[i]->mass;
    }
    return S_OK;
}

HRESULT GrowthModel::getPositions(float time, uint32_t len, Vector3* pos)
{
    for(int i = 0; i < len; ++i) {
        pos[i] = mesh->vertices[i]->position;
    }
    return S_OK;
}

HRESULT GrowthModel::setPositions(float time, uint32_t len, const Vector3* pos)
{
    return mesh->setPositions(len, pos);
}

HRESULT GrowthModel::getAccelerations(float time, uint32_t len,
        const Vector3* pos, Vector3* acc)
{
    HRESULT result;

    if(len != mesh->vertices.size()) {
        return E_FAIL;
    }

    if(pos) {
        if(!SUCCEEDED(result = mesh->setPositions(len, pos))) {
            return result;
        }
    }

    calcForce();

    for(int i = 0; i < mesh->vertices.size(); ++i) {
        VertexPtr v = mesh->vertices[i];

        assert(v->mass > 0 && v->area > 0);

        Vector3 force;

        for(CTrianglePtr tri : v->triangles()) {
            for(int j = 0; j < 3; ++j) {
                if(tri->vertices[j] == v) {
                    force += tri->force(j);
                }
            }
        }

        acc[i] = force / v->mass;
    }


    return S_OK;
}

/*=========================================================================

  Name:        VTKPipeline.cpp

  Author:      David Borland, The Renaissance Computing Institute (RENCI)

  Copyright:   The Renaissance Computing Institute (RENCI)

  License:     Licensed under the RENCI Open Source Software License v. 1.0

               See included License.txt or
               http://www.renci.org/resources/open-source-software-license
               for details.

  Description: Container class for all urban wind visualization VTK code.

=========================================================================*/


#include "VTKPipeline.h"

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkArrayCalculator.h>
#include <vtkAssignAttribute.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkClipDataSet.h>
#include <vtkColorTransferFunction.h>
#include <vtkContourFilter.h>
#include <vtkCoordinate.h>
#include <vtkCutter.h>
#include <vtkCubeSource.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkDiskSource.h>
#include <vtkDoubleArray.h>
#include <vtkExtractGeometry.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSTLReader.h>
#include <vtkTetra.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTriangleFilter.h>
#include <vtkTriangle.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkRendererCallback.h"

#include "MainWindow.h"

#include <fstream>


VTKPipeline::VTKPipeline(vtkRenderWindowInteractor* rwi, MainWindow* qtWindow) 
: interactor(rwi), mainWindow(qtWindow) {
    // Clipping transform
    clippingBoxTransform = vtkTransform::New();


    // Define a unit cube, and use the transform to position/size it
    // Use either a single box or multple planes.  A single box is faster, 
    // but produces errors at the edges
    vtkBox* box = vtkBox::New();
    box->SetBounds(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);
    box->SetTransform(clippingBoxTransform);

    vtkPlane* plane1 = vtkPlane::New();
    plane1->SetOrigin(-0.5, 0.0, 0.0);
    plane1->SetNormal(1.0, 0.0, 0.0);
    plane1->SetTransform(clippingBoxTransform);

    vtkPlane* plane2 = vtkPlane::New();
    plane2->SetOrigin(0.5, 0.0, 0.0);
    plane2->SetNormal(-1.0, 0.0, 0.0);
    plane2->SetTransform(clippingBoxTransform);

    vtkPlane* plane3 = vtkPlane::New();
    plane3->SetOrigin(0.0, -0.5, 0.0);
    plane3->SetNormal(0.0, 1.0, 0.0);
    plane3->SetTransform(clippingBoxTransform);

    vtkPlane* plane4 = vtkPlane::New();
    plane4->SetOrigin(0.0, 0.5, 0.0);
    plane4->SetNormal(0.0, -1.0, 0.0);
    plane4->SetTransform(clippingBoxTransform);

    vtkPlane* plane5 = vtkPlane::New();
    plane5->SetOrigin(0.0, 0.0, -0.5);
    plane5->SetNormal(0.0, 0.0, 1.0);
    plane5->SetTransform(clippingBoxTransform);

    vtkPlane* plane6 = vtkPlane::New();
    plane6->SetOrigin(0.0, 0.0, 0.5);
    plane6->SetNormal(0.0, 0.0, -1.0);
    plane6->SetTransform(clippingBoxTransform);

    
    // Plane for cutting
    cutPlane = vtkPlane::New();
    cutPlane->SetOrigin(0.0, 0.0, 0.0);
    cutPlane->SetNormal(0.0, 0.0, 1.0);
    cutPlane->SetTransform(clippingBoxTransform);


    // Representation of the clipping box
    clippingCubeSource = vtkCubeSource::New();
    clippingCubeSource->SetCenter(0.0, 0.0, 0.0);
    clippingCubeSource->SetXLength(0.0);
    clippingCubeSource->SetYLength(0.0);
    clippingCubeSource->SetZLength(0.0);
    clippingCubeSource->ReleaseDataFlagOn();

    vtkPolyDataMapper* clippingBoxMapper = vtkPolyDataMapper::New();
    clippingBoxMapper->SetInputConnection(clippingCubeSource->GetOutputPort());
    clippingBoxMapper->ReleaseDataFlagOn();

    clippingBoxActor = vtkActor::New();
    clippingBoxActor->SetMapper(clippingBoxMapper);
    clippingBoxActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    clippingBoxActor->GetProperty()->SetOpacity(0.25);
    clippingBoxActor->GetProperty()->SetAmbient(0.5);
    clippingBoxActor->GetProperty()->EdgeVisibilityOn();

    clippingBoxMapper->Delete();


    // Roof offset reader
    roofOffsetReader = vtkXMLPolyDataReader::New();

    // Mesh reader
    meshReader = vtkXMLUnstructuredGridReader::New();


    // Data attribute to use
    dataAttribute = vtkAssignAttribute::New();
    // Call SetInputConnection() when loading/switching between RoofOffset and Mesh
    dataAttribute->Assign("velocityNormXYMag", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);
    vectorData = XYMagnitude;


    // Filter for extracting geometry
    extractData = vtkExtractGeometry::New();
    extractData->SetInputConnection(dataAttribute->GetOutputPort());
    extractData->SetImplicitFunction(box);
    extractData->ExtractInsideOn();
    extractData->ReleaseDataFlagOn();


    // Filter for fast clipping
    clipDataBox = vtkClipDataSet::New();
    clipDataBox->SetInputConnection(dataAttribute->GetOutputPort());
    clipDataBox->SetClipFunction(box);
    clipDataBox->InsideOutOn();
    clipDataBox->ReleaseDataFlagOn();


    // Daisy-chain the clipping filters for accurate clipping
    clipDataPlanesFirst = vtkClipDataSet::New();
    // Call SetInputConnection() in UpdateClipping()
    clipDataPlanesFirst->SetClipFunction(plane1);
    clipDataPlanesFirst->ReleaseDataFlagOn();

    vtkClipDataSet* dataClip2 = vtkClipDataSet::New();
    dataClip2->SetInputConnection(clipDataPlanesFirst->GetOutputPort());
    dataClip2->SetClipFunction(plane2);
    dataClip2->ReleaseDataFlagOn();

    vtkClipDataSet* dataClip3 = vtkClipDataSet::New();
    dataClip3->SetInputConnection(dataClip2->GetOutputPort());
    dataClip3->SetClipFunction(plane3);
    dataClip3->ReleaseDataFlagOn();

    vtkClipDataSet* dataClip4 = vtkClipDataSet::New();
    dataClip4->SetInputConnection(dataClip3->GetOutputPort());
    dataClip4->SetClipFunction(plane4);
    dataClip4->ReleaseDataFlagOn();

    vtkClipDataSet* dataClip5 = vtkClipDataSet::New();
    dataClip5->SetInputConnection(dataClip4->GetOutputPort());
    dataClip5->SetClipFunction(plane5);
    dataClip5->ReleaseDataFlagOn();

    // The final filter can be removed from and added to the pipeline
    clipDataPlanesLast = vtkClipDataSet::New();
    clipDataPlanesLast->SetInputConnection(dataClip5->GetOutputPort());
    clipDataPlanesLast->SetClipFunction(plane6);
    clipDataPlanesLast->ReleaseDataFlagOn();
    
    plane1->Delete();
    plane2->Delete();
    plane3->Delete();
    plane4->Delete();
    plane5->Delete();
    plane6->Delete();

    dataClip2->Delete();
    dataClip3->Delete();
    dataClip4->Delete();
    dataClip5->Delete();


    // Filter for cut plane
    cutData = vtkCutter::New();
    cutData->SetInputConnection(dataAttribute->GetOutputPort());
    cutData->SetCutFunction(cutPlane);
    cutData->ReleaseDataFlagOn();


    // Triangulate the output to make computing areas/volumes easier
    dataTriangle = vtkDataSetTriangleFilter::New();
    // Call SetInputConnection() in UpdateClipping()
    dataTriangle->SetInputConnection(clipDataBox->GetOutputPort());


    // Zero contour for Z component data
    vtkContourFilter* contour = vtkContourFilter::New();
    contour->SetInputConnection(dataTriangle->GetOutputPort());
    contour->SetNumberOfContours(1);
    contour->SetValue(0, 0.0);
    contour->ReleaseDataFlagOn();

    vtkPolyDataMapper* contourMapper = vtkPolyDataMapper::New();
    contourMapper->SetInputConnection(contour->GetOutputPort());
    contourMapper->ScalarVisibilityOff();
    contourMapper->ReleaseDataFlagOn();

    contour->Delete();

    contourActor = vtkActor::New();
    contourActor->SetMapper(contourMapper);
    contourActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    contourActor->VisibilityOff();

    contourMapper->Delete();


    // Need cell data for calculating statistics
    dataCellData = vtkPointDataToCellData::New();
    dataCellData->SetInputConnection(dataTriangle->GetOutputPort());
    dataCellData->PassPointDataOff();

    dataColor = vtkColorTransferFunction::New();


    // Below is for rendering


    // Make some poly data
    dataSurface = vtkDataSetSurfaceFilter::New();
    dataSurface->SetInputConnection(dataTriangle->GetOutputPort());
    dataSurface->ReleaseDataFlagOn();


    // For extruding roofs
    roofOffsetExtrusion = vtkLinearExtrusionFilter::New();
    roofOffsetExtrusion->SetInputConnection(dataSurface->GetOutputPort());
    roofOffsetExtrusion->SetExtrusionTypeToVectorExtrusion();
    roofOffsetExtrusion->SetVector(0.0, 0.0, -1.0);
    roofOffsetExtrusion->CappingOn();
    roofOffsetExtrusion->SetScaleFactor(1.0);
    roofOffsetExtrusion->ReleaseDataFlagOn();


    // Rendering
    dataMapper = vtkDataSetMapper::New();
    // Call SetInputConnection() when loading/switching between RoofOffset and Mesh
    dataMapper->ScalarVisibilityOn();
    dataMapper->SetLookupTable(dataColor);
    dataMapper->UseLookupTableScalarRangeOn();
    dataMapper->ReleaseDataFlagOn();

    dataActor = vtkActor::New();
    dataActor->SetMapper(dataMapper);
    dataActor->GetProperty()->LightingOff();


    // Color map legend
    double width = 0.5;
    double height = 0.1;
    legend = vtkScalarBarActor::New();
    legend->SetLookupTable(dataColor);
    legend->SetOrientationToHorizontal();
    legend->SetPosition((1.0 - width) * 0.5, 0.9);
    legend->SetWidth(width);
    legend->SetHeight(height);
    legend->SetLabelFormat("%g");
    legend->SetTitle("XY Velocity Magnitude (m/s)");
    legend->SetMaximumNumberOfColors(256);

    // Border
    // XXX: This is a hack, and probably only looks right on my laptop, 
    // which was necessary for making some images for Vis Viewpoints...
    double b = 0.0025;
    vtkCoordinate* coord = vtkCoordinate::New();
    coord->SetCoordinateSystemToNormalizedViewport();

    vtkPlaneSource* legendBorderPlane = vtkPlaneSource::New();
    legendBorderPlane->SetOrigin(0.0, 0.0, 0.0);
    legendBorderPlane->SetPoint1(0.0, height * 0.47, 0.0);
    legendBorderPlane->SetPoint2(width + 2.0 * b, 0.0, 0.0);
    
    vtkPolyDataMapper2D* legendBorderMapper = vtkPolyDataMapper2D::New();
    legendBorderMapper->SetInputConnection(legendBorderPlane->GetOutputPort());
    legendBorderMapper->SetTransformCoordinate(coord);

    legendBorderPlane->Delete();

    legendBorderActor = vtkActor2D::New();
    legendBorderActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    legendBorderActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    legendBorderActor->SetMapper(legendBorderMapper);
    legendBorderActor->SetPosition(0, 0);
    legendBorderActor->SetPosition2(legend->GetPosition()[0] - b, legend->GetPosition()[1] - 1.5 * b);
    legendBorderActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    legendBorderMapper->GetTransformCoordinate()->SetReferenceCoordinate(legendBorderActor->GetPosition2Coordinate());

    legendBorderMapper->Delete();


    // Color wheel
    CreateColorWheel();


    // Labels
    CreateLabels();


    // Building
    buildingReader = vtkSTLReader::New();

    vtkDataSetMapper* buildingMapper = vtkDataSetMapper::New();
    buildingMapper->SetInputConnection(buildingReader->GetOutputPort());
    buildingMapper->ReleaseDataFlagOn();

    buildingActor = vtkActor::New();
    buildingActor->SetMapper(buildingMapper);
    buildingActor->GetProperty()->SetColor(0.5, 0.5, 0.5);

    buildingMapper->Delete();


    // Renderer
    renderer = vtkRenderer::New();
renderer->SetBackground(0.5, 0.5, 0.5);

    // Don't add actors yet!!!

    interactor->GetRenderWindow()->AddRenderer(renderer);


    // Renderer callback
    rendererCallback = vtkRendererCallback::New();
    rendererCallback->SetVTKPipeline(this);
    renderer->AddObserver(vtkCommand::StartEvent, rendererCallback);
}

VTKPipeline::~VTKPipeline() {
    renderer->Delete();
    rendererCallback->Delete();

    clippingBoxTransform->Delete();
    clippingCubeSource->Delete();
    clippingBoxActor->Delete();

    extractData->Delete();

    cutPlane->Delete();
    cutData->Delete();

    clipDataBox->Delete();
    
    clipDataPlanesFirst->Delete();
    clipDataPlanesLast->Delete();

    roofOffsetReader->Delete();
    roofOffsetExtrusion->Delete();

    meshReader->Delete();

    dataAttribute->Delete();
    dataTriangle->Delete();
    dataSurface->Delete();
    dataCellData->Delete();
    dataColor->Delete();
    dataMapper->Delete();
    dataActor->Delete();
    contourActor->Delete();

    legend->Delete();
    legendBorderActor->Delete();
    colorWheelActor->Delete();
    colorWheelBorderActor->Delete();
    statisticsLabel->Delete();
    volumeLabel->Delete();
    fileNameLabel->Delete();
    clipLabel->Delete();
    cameraLabel->Delete();

    buildingReader->Delete();
    buildingActor->Delete();
}


void VTKPipeline::OpenRoofOffsetFile(const char* fileName) {
    bool needReset = !HasRoofOffset() && !HasMesh() && !HasBuilding();

    // Read the data
    roofOffsetReader->SetFileName(fileName);

    SetDataSet(VTKPipeline::RoofOffset);

    // Add the actors
    renderer->AddViewProp(dataActor);
    renderer->AddViewProp(contourActor);
    renderer->AddViewProp(clippingBoxActor);
    renderer->AddViewProp(legendBorderActor);
    renderer->AddViewProp(legend);
    renderer->AddViewProp(colorWheelBorderActor);
    renderer->AddViewProp(colorWheelActor);
    renderer->AddViewProp(statisticsLabel);
    renderer->AddViewProp(fileNameLabel);
    renderer->AddViewProp(clipLabel);
    renderer->AddViewProp(cameraLabel);

    // Reset the camera
    if (needReset) renderer->ResetCamera();
}

void VTKPipeline::OpenMeshFile(const char* fileName) {
    bool needReset = !HasRoofOffset() && !HasMesh() && !HasBuilding();

    // Read the data
    meshReader->SetFileName(fileName);
    
    SetDataSet(VTKPipeline::Mesh);

    // Add the actors
    renderer->AddViewProp(dataActor);
    renderer->AddViewProp(contourActor);
    renderer->AddViewProp(clippingBoxActor);
    renderer->AddViewProp(legendBorderActor);
    renderer->AddViewProp(legend);
    renderer->AddViewProp(colorWheelBorderActor);
    renderer->AddViewProp(colorWheelActor);
    renderer->AddViewProp(statisticsLabel);
    renderer->AddViewProp(volumeLabel);
    renderer->AddViewProp(fileNameLabel);
    renderer->AddViewProp(clipLabel);
    renderer->AddViewProp(cameraLabel);

    // Reset the camera
    if (needReset) renderer->ResetCamera();   
}

void VTKPipeline::OpenBuildingFile(const char* fileName) {
    bool needReset = !HasRoofOffset() && !HasMesh() && !HasBuilding();

    // Read the data
    buildingReader->SetFileName(fileName);
    
    // Add the actors
    renderer->AddViewProp(buildingActor);
    renderer->AddViewProp(cameraLabel);

    // Render
    if (needReset) renderer->ResetCamera();
}

void VTKPipeline::SaveData(const char* fileName) {
    // Make sure data is up-to-date
    dataCellData->Update();
    dataTriangle->Update();

    vtkDataSet* data = dataCellData->GetOutput();
    vtkDataArray* cd = data->GetCellData()->GetScalars();

    if (data == NULL || cd == NULL) return;

    // Open the file for writing
    std::ofstream file;
    file.open(fileName);

    if (!file.good()) {
        std::cout << "Could not open " << fileName << " for writing" << std::endl;
    }

    // Write header
    std::string av;
    if (dataSet == RoofOffset || clipType == CutX || clipType == CutY || clipType == CutZ) {
        av = "Area";
    }
    else {
        av = "Volume";
    }
    file << "Value, " << av << ", X, Y, Z" << std::endl;

    // Save the cell data, along with the area/volume of each cell
    for (int i = 0; i < cd->GetNumberOfTuples(); i++) {
        vtkCell* cell = data->GetCell(i);

        vtkPoints* p;
        double cellSize = 0.0;
        double p0[3], p1[3], p2[3], p3[3];
        double center[3];

        switch (cell->GetCellType()) {
            case VTK_TRIANGLE:
                p = cell->GetPoints();
                
                p->GetPoint(0, p0);
                p->GetPoint(1, p1);
                p->GetPoint(2, p2);

                cellSize = vtkTriangle::SafeDownCast(cell)->ComputeArea();

                vtkTriangle::TriangleCenter(p0, p1, p2, center);

                break;

            case VTK_TETRA:
                p = cell->GetPoints();
                
                p->GetPoint(0, p0);
                p->GetPoint(1, p1);
                p->GetPoint(2, p2);
                p->GetPoint(3, p3);

                cellSize = abs(vtkTetra::ComputeVolume(p0, p1, p2, p3));

                vtkTetra::TetraCenter(p0, p1, p2, p3, center);

                break;

            default:
                // Should never be here, due to the vtkDataSetTriangleFilter
                std::cout << "Unknown cell type: " << cell->GetCellType() << std::endl;
        }

        // Write to the file
        file << cd->GetTuple1(i) << ", " << cellSize << ", " << 
                center[0] << ", " << center[1] << ", " << center[2] << std::endl;
    }

    file.close();
}

void VTKPipeline::SaveScreenshot(const char* fileName) {
    interactor->Render();
    interactor->GetRenderWindow()->Modified();

    vtkWindowToImageFilter* image = vtkWindowToImageFilter::New();
    image->SetInput(interactor->GetRenderWindow());

    vtkPNGWriter* writer = vtkPNGWriter::New();
    writer->SetInputConnection(image->GetOutputPort());
    writer->SetFileName(fileName);
    writer->Write();

    image->Delete();
    writer->Delete();
}


void VTKPipeline::SaveCameraView(const char* fileName) {
    // Open the file for writing
    std::ofstream file;
    file.open(fileName);

    if (!file.good()) {
        std::cout << "Could not open " << fileName << " for writing" << std::endl;
    }

    // Write header
    file << "Position, Focal Point, View Up" << std::endl;

    // Write camera information
    // Quick and dirty, but it gets the job done...
    vtkCamera* c = renderer->GetActiveCamera();

    double* p = c->GetPosition();
    double* f = c->GetFocalPoint();
    double* u = c->GetViewUp();

    file << p[0] << " " << p[1] << " " << p[2] << " " <<
            f[0] << " " << f[1] << " " << f[2] << " " <<
            u[0] << " " << u[1] << " " << u[2] << std::endl;

    file.close();
}

void VTKPipeline::OpenCameraView(const char* fileName) {
    std::ifstream file;
    file.open(fileName);

    if (!file.good()) {
        std::cout << "Could not open " << fileName << " for reading" << std::endl;
    }

    // Skip header
    const int bufferSize = 512;
    char buffer[bufferSize];

    file.getline(buffer, bufferSize);

    // Read data
    double p[3];
    double f[3];
    double u[3];

    file >> p[0] >> p[1] >> p[2] >>
            f[0] >> f[1] >> f[2] >>
            u[0] >> u[1] >> u[2];

    file.close();

    // Set the view
    vtkCamera* c = renderer->GetActiveCamera();
    c->SetPosition(p);
    c->SetFocalPoint(f);
    c->SetViewUp(u);

    renderer->ResetCameraClippingRange();
}


void VTKPipeline::SaveClipSettings(const char* fileName) {
    // Open the file for writing
    std::ofstream file;
    file.open(fileName);

    if (!file.good()) {
        std::cout << "Could not open " << fileName << " for writing" << std::endl;
    }

    // Write header
    file << "Center, Size, Rotation, Type" << std::endl;

    // Write clip settings
    // Quick and dirty, but it gets the job done...
    double c[3];
    double s[3];
    double r;
    int t;

    GetClippingBoxCenter(c);
    GetClippingBoxSize(s);
    r = GetClippingBoxRotation();
    t = clipType;

    file << c[0] << " " << c[1] << " " << c[2] << " " <<
            s[0] << " " << s[1] << " " << s[2] << " " <<
            r << " " << t << std::endl;

    file.close();
}

void VTKPipeline::OpenClipSettings(const char* fileName) {
    std::ifstream file;
    file.open(fileName);

    if (!file.good()) {
        std::cout << "Could not open " << fileName << " for reading" << std::endl;
    }

    // Skip header
    const int bufferSize = 512;
    char buffer[bufferSize];

    file.getline(buffer, bufferSize);

    // Read data
    double c[3];
    double s[3];
    double r;
    int t;

    file >> c[0] >> c[1] >> c[2] >>
            s[0] >> s[1] >> s[2] >>
            r >> t;

    file.close();

    // Set the clip settings
    SetClippingBoxCenter(c[0], c[1], c[2]);
    SetClippingBoxSize(s[0], s[1], s[2]);
    SetClippingBoxRotation(r);
    SetClipType((ClipType)t);

    renderer->ResetCameraClippingRange();
}


bool VTKPipeline::GetShowData() {
    return dataActor->GetVisibility() == 1;
}

void VTKPipeline::SetShowData(bool show) {
    dataActor->SetVisibility(show);
}


double VTKPipeline::GetDataOpacity() {
    return dataActor->GetProperty()->GetOpacity();
}

void VTKPipeline::SetDataOpacity(double opacity) {
    dataActor->GetProperty()->SetOpacity(opacity);
}


bool VTKPipeline::GetShowBuilding() {
    return buildingActor->GetVisibility() == 1;
}

void VTKPipeline::SetShowBuilding(bool show) {
    buildingActor->SetVisibility(show);
}


bool VTKPipeline::HasRoofOffset() {
    return roofOffsetReader->GetFileName() != NULL;
}

bool VTKPipeline::HasMesh() {
    return meshReader->GetFileName() != NULL;
}

bool VTKPipeline::HasBuilding() {
    return buildingReader->GetFileName() != NULL;
}


VTKPipeline::DataSet VTKPipeline::GetDataSet() {
    return dataSet;
}

void VTKPipeline::SetDataSet(VTKPipeline::DataSet which) {
    dataSet = which;

    switch (dataSet) {
        case RoofOffset:    
            SetClipType(AccurateClip);
            dataAttribute->SetInputConnection(roofOffsetReader->GetOutputPort());
            dataMapper->ImmediateModeRenderingOff();
            dataMapper->SetInputConnection(roofOffsetExtrusion->GetOutputPort());
            volumeLabel->VisibilityOff();
            fileNameLabel->SetInput(roofOffsetReader->GetFileName());

            break;

        case Mesh:    
            SetClipType(CutZ);
            dataAttribute->SetInputConnection(meshReader->GetOutputPort());
            dataMapper->ImmediateModeRenderingOn();
            dataMapper->SetInputConnection(dataSurface->GetOutputPort());
            volumeLabel->VisibilityOn();
            fileNameLabel->SetInput(meshReader->GetFileName());

            break;
    }
    dataAttribute->Update();

    // Set the color map
    ResetColorMapRange();

    // Set the clipping box bounds
    ResetClippingBox();
    UpdateClipping();

    renderer->ResetCameraClippingRange();
}

/*
const char* VTKPipeline::GetCurrentFileName() {
    switch (dataSet) {
        case RoofOffset:
            return roofOffsetReader->GetFileName();

        case Mesh:
            return meshReader->GetFileName();
    }

    return NULL;
}
*/


VTKPipeline::VectorData VTKPipeline::GetVectorData() {
    return vectorData;
}

void VTKPipeline::SetVectorData(VTKPipeline::VectorData which) {
    vectorData = which;
            
    colorWheelActor->VisibilityOff();
    colorWheelBorderActor->VisibilityOff();
    contourActor->VisibilityOff();

    switch (vectorData) {
        case XYMagnitude:
            legend->SetTitle("XY Velocity Magnitude (m/s)");
            dataAttribute->Assign("velocityNormXYMag", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);

            break;
            
        case XYAngle:
            legend->SetTitle("XY Velocity Angle (degrees)");
            dataAttribute->Assign("velocityNormXYAngle", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);

            colorWheelActor->VisibilityOn();
            colorWheelBorderActor->VisibilityOn();

            break;

        case ZComponent:            
            legend->SetTitle("Z Velocity Component (m/s)");
            dataAttribute->Assign("velocityNormZ", vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);

//            contourActor->VisibilityOn();

            break;
    }

    // Update the pipeline
    UpdatePipeline();

    // Set the color map range
    ResetColorMapRange();

    // Update statistics
    ComputeStatistics();
}


void VTKPipeline::GetBounds(double bounds[6]) {
    vtkDataSet::SafeDownCast(dataAttribute->GetOutput())->GetBounds(bounds);
}


VTKPipeline::ClipType VTKPipeline::GetClipType() {
    return clipType;
}

void VTKPipeline::SetClipType(VTKPipeline::ClipType type) {
    clipType = type;

    switch (clipType) {
        case Extract:
        case FastClip:
        case AccurateClip:
            clippingCubeSource->SetXLength(1.0);
            clippingCubeSource->SetYLength(1.0);
            clippingCubeSource->SetZLength(1.0);
            break;

        case CutX:
            clippingCubeSource->SetXLength(0.0);
            clippingCubeSource->SetYLength(1.0);
            clippingCubeSource->SetZLength(1.0);
            break;

        case CutY:
            clippingCubeSource->SetXLength(1.0);
            clippingCubeSource->SetYLength(0.0);
            clippingCubeSource->SetZLength(1.0);
            break;

        case CutZ:
            clippingCubeSource->SetXLength(1.0);
            clippingCubeSource->SetYLength(1.0);
            clippingCubeSource->SetZLength(0.0);
            break;
    }
}


void VTKPipeline::GetClippingBoxCenter(double center[3]) {
    clippingBoxActor->GetPosition(center);
}

void VTKPipeline::SetClippingBoxCenter(double x, double y, double z) {    
    clippingBoxActor->SetPosition(x, y, z);
    UpdateClipLabel();
}

void VTKPipeline::GetClippingBoxSize(double size[3]) {
    clippingBoxActor->GetScale(size);
}

void VTKPipeline::SetClippingBoxSize(double x, double y, double z) {
    clippingBoxActor->SetScale(x, y, z);
    UpdateClipLabel();
}

double VTKPipeline::GetClippingBoxRotation() {
    return -clippingBoxActor->GetOrientation()[2];
}

void VTKPipeline::SetClippingBoxRotation(double theta) {
    clippingBoxActor->SetOrientation(0.0, 0.0, -theta);
    UpdateClipLabel();
}


void VTKPipeline::UpdateClipping() {    
    switch (clipType) {
        case Extract:
            dataTriangle->SetInputConnection(extractData->GetOutputPort());
            break;

        case FastClip:
            dataTriangle->SetInputConnection(clipDataBox->GetOutputPort());
            break;

        case AccurateClip:
            clipDataPlanesFirst->SetInputConnection(dataAttribute->GetOutputPort());
            dataTriangle->SetInputConnection(clipDataPlanesLast->GetOutputPort());
            break;

        case CutX:
            cutPlane->SetNormal(1.0, 0.0, 0.0);
            clipDataPlanesFirst->SetInputConnection(cutData->GetOutputPort());
            dataTriangle->SetInputConnection(clipDataPlanesLast->GetOutputPort());
            break;

        case CutY:
            cutPlane->SetNormal(0.0, 1.0, 0.0);
            clipDataPlanesFirst->SetInputConnection(cutData->GetOutputPort());
            dataTriangle->SetInputConnection(clipDataPlanesLast->GetOutputPort());
            break;

        case CutZ:
            cutPlane->SetNormal(0.0, 0.0, 1.0);
            clipDataPlanesFirst->SetInputConnection(cutData->GetOutputPort());
            dataTriangle->SetInputConnection(clipDataPlanesLast->GetOutputPort());
            break;
    }

    clippingBoxTransform->Identity();

    // This needs to be the inverse transform
    clippingBoxTransform->Scale(1.0 / clippingBoxActor->GetScale()[0],
                                1.0 / clippingBoxActor->GetScale()[1],
                                1.0 / clippingBoxActor->GetScale()[2]);

    clippingBoxTransform->RotateZ(-clippingBoxActor->GetOrientation()[2]);

    clippingBoxTransform->Translate(-clippingBoxActor->GetPosition()[0],
                                    -clippingBoxActor->GetPosition()[1],
                                    -clippingBoxActor->GetPosition()[2]);

    ComputeStatistics();
}


bool VTKPipeline::GetShowClippingBox() {
    return clippingBoxActor->GetVisibility() == 1;
}

void VTKPipeline::SetShowClippingBox(bool show) {
    clippingBoxActor->SetVisibility(show);
}


void VTKPipeline::ResetClippingBox() {
    // Set the clipping plane x and y bounds
    double bounds[6];    
    GetBounds(bounds);

    SetClippingBoxCenter((int)((bounds[1] + bounds[0]) / 2), 
                         (int)((bounds[3] + bounds[2]) / 2),
                         (int)((bounds[5] + bounds[4]) / 2));
    SetClippingBoxSize((int)(bounds[1] - bounds[0]), 
                       (int)(bounds[3] - bounds[2]), 
                       (int)(bounds[5] - bounds[4]));
    SetClippingBoxRotation(0.0);
}


int VTKPipeline::GetRoofOffsetThickness() {
    return roofOffsetExtrusion->GetScaleFactor();
}

void VTKPipeline::SetRoofOffsetThickness(int thickness) {
    roofOffsetExtrusion->SetScaleFactor(thickness);
}


void VTKPipeline::GetDataRange(double range[2]) {
    vtkDataSet::SafeDownCast(dataAttribute->GetOutput())->GetScalarRange(range);
}

void VTKPipeline::SetColorMapRange(double min, double max) {
    switch (vectorData) {
        case XYMagnitude:
            // Create a black-body radiation color map
            CreateBlackBody(min, max);
            break;

        case XYAngle:
            // Create a circular color map
            CreateCircular();
            break;

        case ZComponent:
            // If positive and negative values, use a double-ended color map, 
            // otherwise use a black-body radiation color map
            if (min < 0.0 && max > 0.0) {
                CreateDoubleEnded(min, max);
            }
            else {
                CreateBlackBody(min, max);
            }
            break;
        }
}


void VTKPipeline::Render() {
    renderer->ResetCameraClippingRange();
    interactor->Render();
}


bool VTKPipeline::GetShowVolumeAreaStatisticsLabel() {
    return volumeLabel->GetVisibility() == 1;
}

bool VTKPipeline::GetShowClippingBoxLabel() {
    return clipLabel->GetVisibility() == 1;
}

bool VTKPipeline::GetShowCameraLabel() {
    return cameraLabel->GetVisibility() == 1;
}

bool VTKPipeline::GetShowDataStatisticsLabel() {
    return statisticsLabel->GetVisibility() == 1;
}

bool VTKPipeline::GetShowFileNameLabel() {
    return fileNameLabel->GetVisibility() == 1;
}


void VTKPipeline::SetShowVolumeAreaStatisticsLabel(bool show) {
    volumeLabel->SetVisibility(show);
}

void VTKPipeline::SetShowClippingBoxLabel(bool show) {
    clipLabel->SetVisibility(show);
}

void VTKPipeline::SetShowCameraLabel(bool show) {
    cameraLabel->SetVisibility(show);
}

void VTKPipeline::SetShowDataStatisticsLabel(bool show) {
    statisticsLabel->SetVisibility(show);
}

void VTKPipeline::SetShowFileNameLabel(bool show) {
    fileNameLabel->SetVisibility(show);
}



void VTKPipeline::SetCameraPosition(double x, double y, double z, double d) {
    vtkCamera* c = renderer->GetActiveCamera();

    c->SetPosition(x, y, z);
    c->SetDistance(d);

    renderer->ResetCameraClippingRange();
    interactor->Render();
}

void VTKPipeline::SetCameraRotation(double w, double x, double y, double z) {
    vtkCamera* c = renderer->GetActiveCamera();
    
    double p[3];
    c->GetPosition(p);

    double d = c->GetDistance();

    c->SetViewUp(0.0, 1.0, 0.0);
    c->SetFocalPoint(0.0, 0.0, -1.0);
    c->SetPosition(0.0, 0.0, 0.0);

    vtkTransform* t = vtkTransform::New();
    t->Translate(p);
    t->RotateWXYZ(-w, x, y, z);

    c->ApplyTransform(t);

    t->Delete();
    
    c->SetDistance(d);

    renderer->ResetCameraClippingRange();
    interactor->Render();
}


void VTKPipeline::ResetCameraX() {    
    vtkCamera* c = renderer->GetActiveCamera();
    c->SetPosition(1.0, 0.0, 0.0);
    c->SetFocalPoint(0.0, 0.0, 0.0);
    c->SetViewUp(0.0, 0.0, 1.0);
    c->Azimuth(-GetClippingBoxRotation());
    renderer->ResetCamera();
    interactor->Render();
}

void VTKPipeline::ResetCameraY() {    
    vtkCamera* c = renderer->GetActiveCamera();
    c->SetPosition(0.0, 1.0, 0.0);
    c->SetFocalPoint(0.0, 0.0, 0.0);
    c->SetViewUp(0.0, 0.0, 1.0);
    c->Azimuth(-GetClippingBoxRotation());
    renderer->ResetCamera();
    interactor->Render();
}

void VTKPipeline::ResetCameraZ() {    
    vtkCamera* c = renderer->GetActiveCamera();
    c->SetPosition(0.0, 0.0, 1.0);
    c->SetFocalPoint(0.0, 0.0, 0.0);
    c->SetViewUp(0.0, 1.0, 0.0);
    c->Roll(GetClippingBoxRotation());
    renderer->ResetCamera();
    interactor->Render();
}


void VTKPipeline::UpdateCamera() {
    UpdateCameraLabel();

    vtkCamera* c = renderer->GetActiveCamera();

    double* p = c->GetPosition();
    double* o = c->GetOrientationWXYZ();
    double d = c->GetDistance();

    mainWindow->SetCameraPosition(p[0], p[1], p[2], d);
    mainWindow->SetCameraRotation(o[0], o[1], o[2], o[3]);
}


void VTKPipeline::ComputeStatistics() {
    // Make sure data is up-to-date
    dataCellData->Update();
    dataTriangle->Update();

    vtkDataSet* data = dataCellData->GetOutput();
    vtkDataArray* pd = dataTriangle->GetOutput()->GetPointData()->GetScalars();
    vtkDataArray* cd = data->GetCellData()->GetScalars();
    vtkDataArray* cv = data->GetCellData()->GetVectors();

    if (data == NULL || pd == NULL || cd == NULL || cv == NULL) return;

    // Get the min and max from point data
    double min = pd->GetNumberOfTuples() > 0 ? pd->GetTuple1(0) : 0.0;
    double max = min;
    for (int i = 1; i < pd->GetNumberOfTuples(); i++) {
        double v = pd->GetTuple1(i);
        min = v < min ? v : min;
        max = v > max ? v : max;
    }

    // Compute the mean from cell data, accounting for the area/volume of each cell
    // Need to account for angles, since the average of 1 degree and 359 degrees should be 0 (or 360)
    // Do this by using the 2D vector components
    double sum = 0.0;
    double xSum = 0.0;
    double ySum = 0.0;
    double size = 0.0;
    for (int i = 0; i < cd->GetNumberOfTuples(); i++) {
        vtkCell* cell = data->GetCell(i);

        vtkPoints* p;
        double cellSize = 0.0;
        double p0[3], p1[3], p2[3], p3[3];

        switch (cell->GetCellType()) {
            case VTK_TRIANGLE:
                cellSize = vtkTriangle::SafeDownCast(cell)->ComputeArea();                    

                break;

            case VTK_TETRA:
                p = cell->GetPoints();
                
                p->GetPoint(0, p0);
                p->GetPoint(1, p1);
                p->GetPoint(2, p2);
                p->GetPoint(3, p3);

                cellSize = abs(vtkTetra::ComputeVolume(p0, p1, p2, p3));

                break;

            default:
                // Should never be here, due to the vtkDataSetTriangleFilter
                std::cout << "Unknown cell type: " << cell->GetCellType() << std::endl;
        }

        if (vectorData == XYAngle) {
            xSum += cv->GetTuple3(i)[0] * cellSize;
            ySum += cv->GetTuple3(i)[1] * cellSize;
        }
        else {
            sum += cd->GetTuple1(i) * cellSize;
        }
        size += cellSize;
    }

    double mean;
    if (vectorData == XYAngle) {
        double xMean = xSum / size;
        double yMean = ySum / size;

        // Calculate the angle of this vector from 0 to 360 degrees, with positive Y as 0 degrees
        // We want the *incoming* wind angle, which is the direction of the negative wind vector
        double ySign = yMean < 0 ? -1.0 : 1.0;
        mean = -(vtkMath::DegreesFromRadians(acos(-xMean)) * -ySign - 90.0);
        mean = mean < 0.0 ? mean + 360.0 : mean;
    }
    else {
        mean = sum / size;
    }

    // Set the statistics label
    UpdateStatisticsLabel(min, max, mean);

    // Set the volume label
    UpdateVolumeLabel(size);
}


void VTKPipeline::UpdatePipeline() {
    dataMapper->Update();
}


void VTKPipeline::ResetColorMapRange() {
    double dataRange[2];
    vtkDataSet::SafeDownCast(dataAttribute->GetOutput())->GetScalarRange(dataRange);
    SetColorMapRange(dataRange[0], dataRange[1]);
}


void VTKPipeline::CreateBlackBody(double min, double max) {
    // Create a black-body radiation color map
    dataColor->RemoveAllPoints();

    // Normal
/*
    dataColor->AddRGBPoint(min,                           0.0, 0.0, 0.0);
    dataColor->AddRGBPoint(min + (max - min) / 3.0,       1.0, 0.0, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 2.0 / 3.0, 1.0, 1.0, 0.0);
    dataColor->AddRGBPoint(max,                           1.0, 1.0, 1.0);
*/

    // Cool
/*
    dataColor->AddRGBPoint(min,                           0.0, 0.0, 0.0);
    dataColor->AddRGBPoint(min + (max - min) / 3.0,       0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(min + (max - min) * 2.0 / 3.0, 0.0, 1.0, 1.0);
    dataColor->AddRGBPoint(max,                           1.0, 1.0, 1.0);
*/


    // Normal plus some hues
/*
    dataColor->AddRGBPoint(min,                             0.0, 0.0, 0.25);
    dataColor->AddRGBPoint(min + (max - min) * 1.0 / 5.0,   0.0, 0.25, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 2.0 / 5.0,   0.5, 0.0, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 3.0 / 5.0,   0.75, 0.75, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 4.0 / 5.0,   0.0, 1.0, 1.0);
    dataColor->AddRGBPoint(max,                             1.0, 1.0, 1.0);
*/


    // Banded with luminance gradient
    double step = 0.001;

    // Purple
    dataColor->AddRGBPoint(min,                                     0.0625, 0.0, 0.25);
    dataColor->AddRGBPoint(min + (max - min) * 1.0 / 5.0,           0.25, 0.0, 1.0);

    // Red
    dataColor->AddRGBPoint(min + (max - min) * 1.0 / 5.0 + step,    0.25, 0.0, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 2.0 / 5.0,           1.0, 0.0, 0.0);

    // Orange
    dataColor->AddRGBPoint(min + (max - min) * 2.0 / 5.0 + step,    0.25, 0.125, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 3.0 / 5.0,           1.0, 0.5, 0.0);

    // Yellow
    dataColor->AddRGBPoint(min + (max - min) * 3.0 / 5.0 + step,    0.25, 0.25, 0.0);
    dataColor->AddRGBPoint(min + (max - min) * 4.0 / 5.0,           1.0, 1.0, 0.0);

    // White
    dataColor->AddRGBPoint(min + (max - min) * 4.0 / 5.0 + step,    0.25, 0.25, 0.25);
    dataColor->AddRGBPoint(max,                                     1.0, 1.0, 1.0);

    
    // Set an appropriate number of labels for the legend
    legend->SetNumberOfLabels(6);
}

void VTKPipeline::CreateCircular() {
    // Create a circular color map
    dataColor->RemoveAllPoints();

    // Wrapped double-ended
    dataColor->AddRGBPoint(0.0,         1.0, 1.0, 1.0);
    dataColor->AddRGBPoint(45.0,        0.0, 1.0, 1.0);
    dataColor->AddRGBPoint(90.0,        0.0, 0.5, 0.0);
    dataColor->AddRGBPoint(135.0,       0.0, 0.0, 0.5);
    dataColor->AddRGBPoint(180.0,       0.25, 0.0, 0.25);
    dataColor->AddRGBPoint(225.0,       0.5, 0.0, 0.0);
    dataColor->AddRGBPoint(270.0,       1.0, 0.5, 0.0);
    dataColor->AddRGBPoint(315.0,       1.0, 1.0, 0.0);
    dataColor->AddRGBPoint(360.0,       1.0, 1.0, 1.0);



    // Rainbow!!!
/*
    dataColor->AddRGBPoint(0.0,     0.0, 1.0, 1.0);
    dataColor->AddRGBPoint(60.0,    0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(120.0,   1.0, 0.0, 1.0);
    dataColor->AddRGBPoint(180.0,   1.0, 0.0, 0.0);
    dataColor->AddRGBPoint(240.0,   1.0, 1.0, 0.0);
    dataColor->AddRGBPoint(300.0,   0.0, 1.0, 0.0);
    dataColor->AddRGBPoint(360.0,   0.0, 1.0, 1.0);
*/



    // 4 color with luminance gradient
/*
    double step = 0.001;

    // Red
    dataColor->AddRGBPoint(0.0,             0.625, 0.0, 0.0);
    dataColor->AddRGBPoint(45.0,            1.0, 0.0, 0.0);      

    // Green
    dataColor->AddRGBPoint(45.0 + step,     0.0, 0.25, 0.0);
    dataColor->AddRGBPoint(135.0,           0.0, 1.0, 0.0);

    // Cyan
    dataColor->AddRGBPoint(135.0 + step,    0.0, 0.25, 0.25);
    dataColor->AddRGBPoint(225.0,           0.0, 1.0, 1.0);

    // Yellow
    dataColor->AddRGBPoint(225.0 + step,    0.25, 0.25, 0.0);
    dataColor->AddRGBPoint(315.0,           1.0, 1.0, 0.0);

    // Back to red
    dataColor->AddRGBPoint(315.0 + step,    0.25, 0.0, 0.0);
    dataColor->AddRGBPoint(360.0,           0.625, 0.0, 0.0);
*/



    // 8 colors with 2 luminance bands each
/*
    // Red
    dataColor->AddRGBPoint(0.0,         0.5, 0.0, 0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(22.5,        1.0, 0.0, 0.0, 0.0, 1.0);

    // Orange
    dataColor->AddRGBPoint(45.0,        0.5, 0.25, 0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(67.5,        1.0, 0.5, 0.0, 0.0, 1.0);

    // Yellow
    dataColor->AddRGBPoint(90.0,        0.5, 0.5, 0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(112.5,       1.0, 1.0, 0.0, 0.0, 1.0);

    // White
    dataColor->AddRGBPoint(135.0,       0.5, 0.5, 0.5, 0.0, 1.0);
    dataColor->AddRGBPoint(157.5,       1.0, 1.0, 1.0, 0.0, 1.0);

    // Green
    dataColor->AddRGBPoint(180.0,       0.0, 0.5, 0.0, 0.0, 1.0);
    dataColor->AddRGBPoint(202.5,       0.0, 1.0, 0.0, 0.0, 1.0);

    // Cyan
    dataColor->AddRGBPoint(225.0,       0.0, 0.5, 0.5, 0.0, 1.0);
    dataColor->AddRGBPoint(247.5,       0.0, 1.0, 1.0, 0.0, 1.0);

    // Blue
    dataColor->AddRGBPoint(270.0,       0.0, 0.0, 0.5, 0.0, 1.0);
    dataColor->AddRGBPoint(292.5,       0.0, 0.0, 1.0, 0.0, 1.0);

    // Purple
    dataColor->AddRGBPoint(315.0,       0.5, 0.0, 0.5, 0.0, 1.0);
    dataColor->AddRGBPoint(337.5,       1.0, 0.0, 1.0, 0.0, 1.0);

    // Back to red
    dataColor->AddRGBPoint(360.0,       0.5, 0.0, 0.0, 0.0, 1.0);
*/



    // 4 color
/*
    dataColor->AddRGBPoint(0.0,       1.0, 0.0, 0.0);

    dataColor->AddRGBPoint(90.0,      0.0, 1.0, 0.0);

    dataColor->AddRGBPoint(180.0,     0.0, 0.0, 1.0);

    dataColor->AddRGBPoint(270.0,     1.0, 1.0, 0.0);

    dataColor->AddRGBPoint(360.0,     1.0, 0.0, 0.0);
*/

    

    // Set an appropriate number of labels for the legend
    legend->SetNumberOfLabels(9);
}


void VTKPipeline::CreateDoubleEnded(double min, double max) {
    // Create a double-ended color map
/*
    double minScale = 1.0;
    double maxScale = 1.0;

    if (max > abs(min)) {
        minScale = abs(min) / max;
    }
    else if (abs(min) > max) {
        maxScale = max / abs(min);
    }

    dataColor->RemoveAllPoints();

    dataColor->AddRGBPoint(min,           0.5 - 0.5 * minScale, 
                                                0.5 - 0.5 * minScale, 
                                                0.5 + 0.5 * minScale);

    dataColor->AddRGBPoint(0.0,           0.5, 0.5, 0.5);

    dataColor->AddRGBPoint(max,           0.5 + 0.5 * maxScale, 
                                                0.5 - 0.5 * maxScale, 
                                                0.5 - 0.5 * maxScale);
*/

	// Create a Paraview-style double-ended color map
/*
	dataColor->SetColorSpaceToDiverging();

	dataColor->RemoveAllPoints();

	dataColor->AddRGBPoint(min, 58.0 / 255.0, 76.0 / 255.0, 193.0 / 255.0);
	dataColor->AddRGBPoint(max, 180.0 / 255.0, 4.0 / 255.0, 38.0 / 255.0);
*/


	// Double-ended based on black-body
    dataColor->RemoveAllPoints();

    dataColor->AddRGBPoint(min,         0.75, 1.0, 1.0);
    dataColor->AddRGBPoint(min * 0.75,  0.0, 1.0, 1.0);
    dataColor->AddRGBPoint(min * 0.1,   0.0, 0.0, 0.5);
    dataColor->AddRGBPoint(0.0,         0.1, 0.1, 0.1);
    dataColor->AddRGBPoint(max * 0.1,   0.5, 0.0, 0.0);
    dataColor->AddRGBPoint(max * 0.75,  1.0, 1.0, 0.0);
    dataColor->AddRGBPoint(max,         1.0, 1.0, 0.75);

        
    // Set an appropriate number of labels for the legend
    legend->SetNumberOfLabels(5);
}


void VTKPipeline::CreateColorWheel() {
    int numSegments = 256;
    double radius = 30.0;


    vtkDoubleArray* data = vtkDoubleArray::New();
    data->SetNumberOfTuples(numSegments);
    data->SetNumberOfComponents(1);

    for (int i = 0; i < numSegments; i++) {
        double v = -(i * 360.0 / (double)numSegments - 90.0);
        v = v < 0.0 ? v + 360.0 : v;
        data->SetTuple1(i, v);
    }


    vtkDiskSource* disk = vtkDiskSource::New();
    disk->SetRadialResolution(1);
    disk->SetCircumferentialResolution(numSegments);
    disk->SetOuterRadius(radius);
    disk->SetInnerRadius(radius / 4.0);

    disk->Update();
    disk->GetOutput()->GetCellData()->SetScalars(data);

    data->Delete();


    vtkCoordinate* coord = vtkCoordinate::New();
    coord->SetCoordinateSystemToViewport();


    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::New();
    mapper->SetInputConnection(disk->GetOutputPort());
    mapper->SetTransformCoordinate(coord);
    mapper->SetLookupTable(dataColor);
    mapper->ScalarVisibilityOn();

    disk->Delete();
    coord->Delete();


    colorWheelActor = vtkActor2D::New();
    colorWheelActor->SetMapper(mapper);
    colorWheelActor->SetPosition(0.0, 0.0);
    colorWheelActor->SetPosition2(0.94, 0.92);
    colorWheelActor->VisibilityOff();
    mapper->GetTransformCoordinate()->SetReferenceCoordinate(colorWheelActor->GetPosition2Coordinate());

    mapper->Delete();


    // Add a border
    double border = 1.5;
    vtkDiskSource* borderDisk = vtkDiskSource::New();
    borderDisk->SetRadialResolution(1);
    borderDisk->SetCircumferentialResolution(numSegments);
    borderDisk->SetOuterRadius(radius + border);
    borderDisk->SetInnerRadius(radius / 4.0 - border);   
    
    vtkPolyDataMapper2D* borderMapper = vtkPolyDataMapper2D::New();
    borderMapper->SetInputConnection(borderDisk->GetOutputPort());
    borderMapper->SetTransformCoordinate(coord);

    borderDisk->Delete();

    colorWheelBorderActor = vtkActor2D::New();
    colorWheelBorderActor->SetMapper(borderMapper);
    colorWheelBorderActor->SetPosition(0.0, 0.0);
    colorWheelBorderActor->SetPosition2(0.94, 0.92);
    colorWheelBorderActor->VisibilityOff();
    colorWheelBorderActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    borderMapper->GetTransformCoordinate()->SetReferenceCoordinate(colorWheelBorderActor->GetPosition2Coordinate());

    borderMapper->Delete();
}


void VTKPipeline::CreateLabels() {
    // Statistics label
    statisticsLabel = vtkTextActor::New();
    statisticsLabel->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    statisticsLabel->SetPosition(1.0, 0.01);
    statisticsLabel->GetTextProperty()->SetFontFamilyToArial();
    statisticsLabel->GetTextProperty()->BoldOn();
    statisticsLabel->GetTextProperty()->ItalicOn();
    statisticsLabel->GetTextProperty()->ShadowOff();
    statisticsLabel->GetTextProperty()->SetFontSize(16);
    statisticsLabel->GetTextProperty()->SetJustificationToRight();


    // Volume label
    volumeLabel = vtkTextActor::New();
    volumeLabel->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    volumeLabel->SetPosition(0.0, 0.01);
    volumeLabel->GetTextProperty()->SetFontFamilyToArial();
    volumeLabel->GetTextProperty()->BoldOn();
    volumeLabel->GetTextProperty()->ItalicOn();
    volumeLabel->GetTextProperty()->ShadowOff();
    volumeLabel->GetTextProperty()->SetFontSize(16);
    volumeLabel->GetTextProperty()->SetJustificationToLeft();


    // File name label
    fileNameLabel = vtkTextActor::New();
    fileNameLabel->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    fileNameLabel->SetPosition(0.5, 0.11);
    fileNameLabel->SetWidth(1.0);
    fileNameLabel->GetTextProperty()->SetFontFamilyToArial();
    fileNameLabel->GetTextProperty()->BoldOn();
    fileNameLabel->GetTextProperty()->ItalicOn();
    fileNameLabel->GetTextProperty()->ShadowOff();
    fileNameLabel->GetTextProperty()->SetFontSize(16);
    fileNameLabel->GetTextProperty()->SetJustificationToCentered();

    // Clip label
    clipLabel = vtkTextActor::New();
    clipLabel->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    clipLabel->SetPosition(0.25, 0.01);
    clipLabel->SetWidth(1.0);
    clipLabel->GetTextProperty()->SetFontFamilyToArial();
    clipLabel->GetTextProperty()->BoldOn();
    clipLabel->GetTextProperty()->ItalicOn();
    clipLabel->GetTextProperty()->ShadowOff();
    clipLabel->GetTextProperty()->SetFontSize(16);
    clipLabel->GetTextProperty()->SetJustificationToLeft();

    // Camera label
    cameraLabel = vtkTextActor::New();
    cameraLabel->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    cameraLabel->SetPosition(0.45, 0.01);
    cameraLabel->SetWidth(1.0);
    cameraLabel->GetTextProperty()->SetFontFamilyToArial();
    cameraLabel->GetTextProperty()->BoldOn();
    cameraLabel->GetTextProperty()->ItalicOn();
    cameraLabel->GetTextProperty()->ShadowOff();
    cameraLabel->GetTextProperty()->SetFontSize(16);
    cameraLabel->GetTextProperty()->SetJustificationToLeft();
}

void VTKPipeline::UpdateStatisticsLabel(double min, double max, double mean) {
    char buffer[512];

    switch (vectorData) {
        case XYMagnitude:
            sprintf(buffer, "XY Velocity Magnitude Statistics:\nMin: %g m/s\nMax: %g m/s\nMean: %g m/s", min, max, mean);
            break;

        case XYAngle:
            sprintf(buffer, "XY Velocity Angle Statistics:\nMin: %g degrees\nMax: %g degrees\nMean: %g degrees", min, max, mean);
            break;

        case ZComponent:
            sprintf(buffer, "Z Velocity Component Statistics:\nMin: %g m/s\nMax: %g m/s\nMean: %g m/s", min, max, mean);
            break;
    }
    statisticsLabel->SetInput(buffer);
}

void VTKPipeline::UpdateVolumeLabel(double size) {
    char buffer[512];

    double clip[3];
    GetClippingBoxSize(clip);
    double clipSize;

    switch (clipType) {
        case Extract:
        case FastClip:
        case AccurateClip:
            clipSize = clip[0] * clip[1] * clip[2];
            sprintf(buffer, "Volume Statistics:\nClip: %g m^3\nCells: %g m^3\nDifference: %g m^3", clipSize, size, clipSize - size);
            break;

        case CutX:
            clipSize = clip[1] * clip[2];
            sprintf(buffer, "Area Statistics:\n\tClip: %g m^2\nCells: %g m^2\nDifference: %g m^2", clipSize, size, clipSize - size);
            break;

        case CutY:
            clipSize = clip[0] * clip[2];
            sprintf(buffer, "Area Statistics:\nClip: %g m^2\nCells: %g m^2\nDifference: %g m^2", clipSize, size, clipSize - size);
            break;

        case CutZ:
            clipSize = clip[0] * clip[1];
            sprintf(buffer, "Area Statistics:\nClip: %g m^2\nCells: %g m^2\nDifference: %g m^2", clipSize, size, clipSize - size);
            break;
    }
    volumeLabel->SetInput(buffer);
}

void VTKPipeline::UpdateClipLabel() {
    double c[3];
    double s[3];
    double r;

    GetClippingBoxCenter(c);
    GetClippingBoxSize(s);
    r = abs(GetClippingBoxRotation());

    char buffer[512];
    sprintf(buffer, "Clipping Box:\nPosition: %g %g %g\nSize: %g %g %g\nRotation: %g", c[0], c[1], c[2],
                                                                                       s[0], s[1], s[2],
                                                                                       r);
    clipLabel->SetInput(buffer);
}

void VTKPipeline::UpdateCameraLabel() {
    vtkCamera* c = renderer->GetActiveCamera();

    double* p = c->GetPosition();
    double* o = c->GetOrientationWXYZ();
    double d = c->GetDistance();

    char buffer[512];
    sprintf(buffer, "Camera:\nPosition: %g %g %g\nDistance: %g\nRotation: %g %g %g %g", p[0], p[1], p[2],
                                                                                        d,
                                                                                        o[0], o[1], o[2], o[3]);
    cameraLabel->SetInput(buffer);
}
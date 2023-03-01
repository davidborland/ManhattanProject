///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        ManhattanProject.cpp
//
// Author:      David Borland
//
// Description: Program for viewing Manhattan airflow simulation data
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include <vtkArrayCalculator.h>
#include <vtkCamera.h>
#include <vtkClipPolyData.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataIterator.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkDataArraySelection.h>
#include <vtkDataSetMapper.h>
#include <vtkEnSightGoldBinaryReader.h>
#include <vtkExtractSelectedThresholds.h>
#include <vtkImplicitModeller.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProbeFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkSTLReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWin32ProcessOutputWindow.h>
#include <vtkXMLUnstructuredGridReader.h>


int main(int argc, char* argv[]) {
    // Set VTK output window 
    vtkWin32ProcessOutputWindow* outputWindow = vtkWin32ProcessOutputWindow::New();
    vtkOutputWindow::SetInstance(outputWindow);
    outputWindow->Delete();


    // Velocity data
/*
    vtkEnSightGoldBinaryReader* dataReader = vtkEnSightGoldBinaryReader::New();
    dataReader->SetCaseFileName("C:/borland/data/Manhattan/msg_iop2_run3/full_trans_151.ensight.encas");
//    dataReader->SetCaseFileName("C:/borland/data/Manhattan/msg_iop2_run3/full_trans_151.ensight.encas");
    dataReader->ReadAllVariablesOff();
    dataReader->GetPointDataArraySelection()->DisableAllArrays();
    dataReader->GetPointDataArraySelection()->EnableArray("velocity");
    dataReader->Update();
*/
    vtkXMLUnstructuredGridReader* dataReader = vtkXMLUnstructuredGridReader::New();
    dataReader->SetFileName("C:/borland/data/Manhattan/msg_iop2_run3/full_trans_151.ensight.encas.vtu");
    dataReader->GetPointDataArraySelection()->DisableAllArrays();
    dataReader->GetPointDataArraySelection()->EnableArray("velocityNorm");


    // Building geometry
    vtkSTLReader* buildingReader = vtkSTLReader::New();
    buildingReader->SetFileName("C:/borland/data/Manhattan/Manhattan_STL2.stl");

    vtkPolyDataMapper* buildingMapper = vtkPolyDataMapper::New();
    buildingMapper->SetInputConnection(buildingReader->GetOutputPort());

    vtkActor* buildingActor = vtkActor::New();
    buildingActor->SetMapper(buildingMapper);


    // Generate normals for thresholding
    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
    normals->SetInputConnection(buildingReader->GetOutputPort());
    normals->SetFeatureAngle(0.0);
    normals->SplittingOn();
    normals->ConsistencyOff();
    normals->NonManifoldTraversalOn();
    normals->ComputePointNormalsOff();
    normals->ComputeCellNormalsOn();

    // Extract the z-component of the normals
    vtkArrayCalculator* calculator = vtkArrayCalculator::New();
    calculator->SetInputConnection(normals->GetOutputPort());
    calculator->SetAttributeModeToUseCellData();
    calculator->AddVectorArrayName("Normals");
    calculator->SetResultArrayName("ZNormal");
    calculator->SetFunction("Normals . kHat");

    // Threshold the normals so we only have upward facing normals
    vtkThreshold* threshold = vtkThreshold::New();
    threshold->SetInputConnection(calculator->GetOutputPort());
    threshold->ThresholdByUpper(0.44);

    // Translate the extracted roofs
    vtkTransform* t = vtkTransform::New();
    t->Translate(0.0, 0.0, 10.0);

    vtkTransformFilter* transform = vtkTransformFilter::New();
    transform->SetInputConnection(threshold->GetOutputPort());
    transform->SetTransform(t);

    // Sample the velocity field with the roofs
    vtkProbeFilter* probe = vtkProbeFilter::New();
    probe->SetInputConnection(dataReader->GetOutputPort());
    probe->SetSource(transform->GetOutput());

    probe->Update();
    probe->GetOutput()->Print(std::cout);

    vtkColorTransferFunction* color = vtkColorTransferFunction::New();
    color->SetVectorModeToMagnitude();
    color->RemoveAllPoints();
    color->AddRGBPoint(0.0, 0.0, 0.0, 0.25);
    color->AddRGBPoint(5.0, 1.0, 1.0, 1.0);

    vtkDataSetMapper* roofMapper = vtkDataSetMapper::New();
    roofMapper->SetInputConnection(probe->GetOutputPort());
    roofMapper->SetColorModeToMapScalars();
    roofMapper->ScalarVisibilityOn();
    roofMapper->SetLookupTable(color);

    vtkActor* roofActor = vtkActor::New();
    roofActor->SetMapper(roofMapper);


    // Building offset
/*
    vtkImplicitModeller* distance = vtkImplicitModeller::New();
    distance->SetInputConnection(threshold->GetOutputPort());
    distance->SetMaximumDistance(10.0);
    distance->SetSampleDimensions(1000, 1000, 250);
    distance->SetProcessModeToPerVoxel();
    distance->SetNumberOfThreads(7);

    vtkContourFilter* contour = vtkContourFilter::New();
    contour->SetInputConnection(distance->GetOutputPort());
    contour->SetValue(0, 5.0);
    contour->ComputeScalarsOff();
    contour->ComputeGradientsOff();

    vtkPolyDataMapper* contourMapper = vtkPolyDataMapper::New();
    contourMapper->SetInputConnection(contour->GetOutputPort());

    vtkActor* contourActor = vtkActor::New();
    contourActor->SetMapper(contourMapper);
*/


    // Rendering
    vtkRenderer* renderer = vtkRenderer::New();
    renderer->AddViewProp(buildingActor);
    renderer->AddViewProp(roofActor);
//    renderer->AddViewProp(contourActor);

/*
    vtkCompositeDataIterator* iterator = reader->GetOutput()->NewIterator();

    int i = 0;
    while (!iterator->IsDoneWithTraversal()) {
        vtkDataSetMapper* mapper = vtkDataSetMapper::New();
        mapper->SetInputConnection(reader->GetOutput()->GetDataSet(iterator)->GetProducerPort());
        mapper->ScalarVisibilityOff();

        vtkActor* actor = vtkActor::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetRepresentationToWireframe();

        if (i == 0) actor->GetProperty()->SetColor(1, 0, 0);
        else if (i == 1) actor->GetProperty()->SetColor(0, 1, 0);
        else if (i == 2) actor->GetProperty()->SetColor(0, 0, 1);
        else if (i == 3) actor->GetProperty()->SetColor(1, 1, 0);
        i++;
        
        renderer->AddViewProp(actor);

        mapper->Delete();
        actor->Delete();

        iterator->GoToNextItem();
    }
*/

    vtkRenderWindow* window = vtkRenderWindow::New();
    window->AddRenderer(renderer);

    vtkInteractorStyleTrackballCamera* interactorStyle = vtkInteractorStyleTrackballCamera::New();
    
    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::New();
    interactor->SetRenderWindow(window);
    interactor->SetInteractorStyle(interactorStyle);

    interactor->Initialize();
    interactor->Start();

    

    dataReader->Delete();
    buildingReader->Delete();
    buildingMapper->Delete();
    buildingActor->Delete();
    renderer->Delete();
    window->Delete();
    interactorStyle->Delete();
    interactor->Delete();


#if 0

    


/*
    vtkSTLReader* reader = vtkSTLReader::New();
    reader->SetFileName("Data/Manhattan_STL2.stl");

    vtkSTLReader* reader2 = vtkSTLReader::New();
    reader2->SetFileName("Data/Manhattan_terrain.stl");

    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkPolyDataMapper* mapper2 = vtkPolyDataMapper::New();
    mapper2->SetInputConnection(reader2->GetOutputPort());

    vtkLODActor* actor = vtkLODActor::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetAmbientColor(0.5, 0.5, 0.5);
    actor->GetProperty()->SetAmbient(0.1);
    actor->GetProperty()->SetDiffuseColor(0.5, 0.5, 0.5);
    actor->GetProperty()->SetDiffuse(1.0);
    actor->GetProperty()->SetSpecularColor(1.0, 1.0, 1.0);
    actor->GetProperty()->SetSpecular(0.5);
    actor->GetProperty()->SetSpecularPower(50.0);
    actor->GetProperty()->LoadMaterial("perPixelLighting.xml");
    actor->GetProperty()->ShadingOn();

    actor->SetNumberOfCloudPoints(1000000);

    vtkLODActor* actor2 = vtkLODActor::New();
    actor2->SetMapper(mapper2);
    actor2->GetProperty()->SetColor(0.0, 0.2, 0.0);
    actor2->GetProperty()->SetAmbient(0.1);
    actor2->GetProperty()->SetDiffuse(1.0);
    actor->GetProperty()->SetSpecularColor(1.0, 1.0, 1.0);
    actor->GetProperty()->SetSpecular(0.5);
    actor->GetProperty()->SetSpecularPower(50.0);
    actor->GetProperty()->LoadMaterial("perPixelLighting.xml");
    actor2->GetProperty()->ShadingOn();

    vtkRenderer* renderer = vtkRenderer::New();
    renderer->SetBackground(0.0, 0.0, 0.25);
    renderer->AddViewProp(actor);
    renderer->AddViewProp(actor2);    
    
    vtkLight* light = vtkLight::New();
    light->SetLightTypeToSceneLight();
    light->PositionalOn();
    light->SetPosition(50000, 50000, 100000);
    renderer->AddLight(light);

    vtkLight* light2 = vtkLight::New();
    light2->SetLightTypeToHeadlight();
    light2->PositionalOn();
    renderer->AddLight(light2);
*/


    vtkRenderWindow* window = vtkRenderWindow::New();
    window->SetWindowName("ManhattanProject");
//    window->BordersOff();
    window->SetSize(1024, 1024);
    window->AddRenderer(renderer);

//    window->Render();
//    vtkOBJExporter* exporter = vtkOBJExporter::New();
//    vtkVRMLExporter* exporter = vtkVRMLExporter::New();
//    vtkOOGLExporter* exporter = vtkOOGLExporter::New();
//    vtkX3DExporter* exporter = vtkX3DExporter::New();
//    vtkIVExporter* exporter = vtkIVExporter::New();
//    exporter->SetRenderWindow(window);
//    exporter->SetFileName("Data/Manhattan.iv");
//    exporter->Write();
//    exporter->Delete();

//    window->SetDomePitch(-30.0);    
//    window->SetDomeRender(1);

    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::New();
    interactor->SetRenderWindow(window);
    interactor->SetDesiredUpdateRate(10);    

    vtkInteractorStyleTrackballCamera* interactorStyle = vtkInteractorStyleTrackballCamera::New();
    interactor->SetInteractorStyle(interactorStyle);

    interactor->Initialize();
    interactor->Start();


    reader->Delete();
//    reader2->Delete();
//    mapper->Delete();
//    mapper2->Delete();
//    actor->Delete();
//    actor2->Delete();
    renderer->Delete();
//    light->Delete();
//    light2->Delete();
    window->Delete();
    interactor->Delete();
    interactorStyle->Delete();

#endif

    return 0;
}
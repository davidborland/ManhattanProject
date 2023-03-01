/*=========================================================================

  Name:        VTKPipeline.h

  Author:      David Borland, The Renaissance Computing Institute (RENCI)

  Copyright:   The Renaissance Computing Institute (RENCI)

  License:     Licensed under the RENCI Open Source Software License v. 1.0

               See included License.txt or
               http://www.renci.org/resources/open-source-software-license
               for details.

  Description: Container class for all urban wind visualization VTK code.

=========================================================================*/


#ifndef VTKPIPELINE_H
#define VTKPIPELINE_H


class vtkActor;
class vtkActor2D;
class vtkAssignAttribute;
class vtkClipDataSet;
class vtkColorTransferFunction;
class vtkCubeSource;
class vtkCutter;
class vtkDataSetMapper;
class vtkDataSetTriangleFilter;
class vtkDataSetSurfaceFilter;
class vtkExtractGeometry;
class vtkLinearExtrusionFilter;
class vtkPlane;
class vtkPointDataToCellData;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkScalarBarActor;
class vtkSTLReader;
class vtkTextActor;
class vtkTransform;
class vtkXMLPolyDataReader;
class vtkXMLUnstructuredGridReader;

class vtkRendererCallback;

class MainWindow;


class VTKPipeline {
public:
    VTKPipeline(vtkRenderWindowInteractor* rwi, MainWindow* qtWindow);
    ~VTKPipeline();

    // Load data
    void OpenRoofOffsetFile(const char* fileName);
    void OpenMeshFile(const char* fileName);
    void OpenBuildingFile(const char* fileName);

    // Save the clipped data
    void SaveData(const char* fileName);

    // Save a screenshot
    void SaveScreenshot(const char* fileName);

    // Save/open a camera view
    void SaveCameraView(const char* fileName);
    void OpenCameraView(const char* fileName);

    // Save/open clip settings
    void SaveClipSettings(const char* fileName);
    void OpenClipSettings(const char* fileName);

    // Get/set showing data
    bool GetShowData();
    void SetShowData(bool show);

    double GetDataOpacity();
    void SetDataOpacity(double opacity);

    bool GetShowBuilding();
    void SetShowBuilding(bool show);

    // Data loaded?
    bool HasRoofOffset();
    bool HasMesh();
    bool HasBuilding();

    // Data to use
    enum DataSet {
        RoofOffset,
        Mesh
    };
    DataSet GetDataSet();
    void SetDataSet(DataSet which);
//    const char* GetCurrentFileName();

    // Vector data to use
    enum VectorData {
        XYMagnitude,
        XYAngle,
        ZComponent
    };
    VectorData GetVectorData();
    void SetVectorData(VectorData which);

    // Clipping
    void GetBounds(double bounds[6]);

    enum ClipType {
        Extract,
        FastClip,
        AccurateClip,
        CutX,
        CutY,
        CutZ
    };
    ClipType GetClipType();
    void SetClipType(ClipType type);

    void GetClippingBoxCenter(double center[3]);
    void GetClippingBoxSize(double size[3]);
    double GetClippingBoxRotation();

    void SetClippingBoxCenter(double x, double y, double z);
    void SetClippingBoxSize(double x, double y, double z);
    void SetClippingBoxRotation(double theta);

    void UpdateClipping();

    bool GetShowClippingBox();
    void SetShowClippingBox(bool show);

    void ResetClippingBox();

    // Get/set roof offset thickness
    int GetRoofOffsetThickness();
    void SetRoofOffsetThickness(int thickness);

    // Get/set data and color map range
    void GetDataRange(double range[2]);
    void SetColorMapRange(double min, double max);

    // Force a render
    void Render();

    // Get/set show labels
    bool GetShowVolumeAreaStatisticsLabel();
    bool GetShowClippingBoxLabel();
    bool GetShowCameraLabel();
    bool GetShowDataStatisticsLabel();
    bool GetShowFileNameLabel();

    void SetShowVolumeAreaStatisticsLabel(bool show);
    void SetShowClippingBoxLabel(bool show);
    void SetShowCameraLabel(bool show);
    void SetShowDataStatisticsLabel(bool show);
    void SetShowFileNameLabel(bool show);

    // Get/set the camera
    void GetCameraPosition(double position[4]);
    void SetCameraPosition(double x, double y, double z, double d);
    void GetCameraRotation(double rotation[4]);
    void SetCameraRotation(double w, double x, double y, double z);

    // Reset the camera
    void ResetCameraX();
    void ResetCameraY();
    void ResetCameraZ();

    void UpdateCamera();

protected:
    // The Qt main window
    MainWindow* mainWindow;

    // Rendering
    vtkRenderWindowInteractor* interactor;
    vtkRenderer* renderer;

    // Roof offset objects
    vtkXMLPolyDataReader* roofOffsetReader;
    vtkLinearExtrusionFilter* roofOffsetExtrusion;

    // Mesh data objects
    vtkXMLUnstructuredGridReader* meshReader;

    // Data objects  
    vtkAssignAttribute* dataAttribute;
    vtkDataSetTriangleFilter* dataTriangle;
    vtkDataSetSurfaceFilter* dataSurface;
    vtkPointDataToCellData* dataCellData;
    vtkColorTransferFunction* dataColor;
    vtkDataSetMapper* dataMapper;
    vtkActor* dataActor;
    vtkActor* contourActor;

    // Building objects
    vtkSTLReader* buildingReader;
    vtkActor* buildingActor;

    // Clipping
    vtkTransform* clippingBoxTransform;
    vtkCubeSource* clippingCubeSource;
    vtkActor* clippingBoxActor;

    vtkExtractGeometry* extractData;

    vtkPlane* cutPlane;
    vtkCutter* cutData;

    vtkClipDataSet* clipDataBox;

    vtkClipDataSet* clipDataPlanesFirst;
    vtkClipDataSet* clipDataPlanesLast;

    ClipType clipType;

    // Color map legend
    vtkScalarBarActor* legend;
    vtkActor2D* legendBorderActor;
    vtkActor2D* colorWheelActor;
    vtkActor2D* colorWheelBorderActor;

    // Labels
    vtkTextActor* statisticsLabel;
    vtkTextActor* volumeLabel;
    vtkTextActor* fileNameLabel;
    vtkTextActor* clipLabel;
    vtkTextActor* cameraLabel;

    // Callback for rendering
    vtkRendererCallback* rendererCallback;

    // Which data
    DataSet dataSet;
    VectorData vectorData;

    // Compute the mean of the wind velocities
    // XXX: Should move to a VTK filter?
    void ComputeStatistics();

    // Force a pipeline update
    void UpdatePipeline();

    // Reset the color map range to the full data range
    void ResetColorMapRange();

    // Different color maps
    void CreateBlackBody(double min, double max);
    void CreateCircular();
    void CreateDoubleEnded(double min, double max);

    // Legend aid for angles
    void CreateColorWheel();

    // Labels
    void CreateLabels();
    void UpdateStatisticsLabel(double min, double max, double mean);
    void UpdateVolumeLabel(double size);
    void UpdateClipLabel();
    void UpdateCameraLabel();
};


#endif
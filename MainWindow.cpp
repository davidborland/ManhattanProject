/*=========================================================================

  Name:        MainWindow.cpp

  Author:      David Borland

  Description: The main window for the Aeolus urban wind flow
               visualization program

=========================================================================*/


#include "MainWindow.h"

#include <qapplication.h>
#include <qfiledialog.h>

#include "VTKPipeline.h"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Create the GUI from the Qt Designer file
    setupUi(this);
    
    // Data set buttons group
    QButtonGroup* dataSetButtonGroup = new QButtonGroup(this);
    dataSetButtonGroup->addButton(roofOffsetRadioButton);
    dataSetButtonGroup->addButton(meshRadioButton);

    // Vector data  buttons group
    QButtonGroup* vectorDataButtonGroup = new QButtonGroup(this);
    vectorDataButtonGroup->addButton(xyMagnitudeRadioButton);
    vectorDataButtonGroup->addButton(xyAngleRadioButton);
    vectorDataButtonGroup->addButton(zComponentRadioButton);


    // Turn off tracking for slider because we want
    // to do different things when dragged versus released
    dataOpacitySlider->setTracking(false);
    roofOffsetThicknessSlider->setTracking(false);


    // Use int validators for line edits
    clipCenterXValidator = new QIntValidator(0, 1, this);
    clipCenterXLineEdit->setValidator(clipCenterXValidator);

    clipCenterYValidator = new QIntValidator(0, 1, this);
    clipCenterYLineEdit->setValidator(clipCenterXValidator);

    clipCenterZValidator = new QIntValidator(0, 1, this);
    clipCenterZLineEdit->setValidator(clipCenterXValidator);


    clipSizeXValidator = new QIntValidator(0, 1, this);
    clipSizeXLineEdit->setValidator(clipSizeXValidator);

    clipSizeYValidator = new QIntValidator(0, 1, this);
    clipSizeYLineEdit->setValidator(clipSizeXValidator);

    clipSizeZValidator = new QIntValidator(0, 1, this);
    clipSizeZLineEdit->setValidator(clipSizeXValidator);


    roofOffsetThicknessLineEdit->setValidator(new QIntValidator(1, 10, this));

    
    // Label for current file
/*
    statusBarLabel = new QLabel("", this);
    statusBarLabel->setAlignment(Qt::AlignRight);
    statusBar()->addWidget(statusBarLabel, 1);
*/

    // Create the visualization pipeline
    pipeline = new VTKPipeline(qvtkWidget->GetInteractor(), this);

    // Initalize the GUI
    RefreshGUI();
}

MainWindow::~MainWindow() {
    delete pipeline;
}


void MainWindow::SetCameraPosition(double x, double y, double z, double d) {
    cameraPositionXSpinBox->blockSignals(true);
    cameraPositionYSpinBox->blockSignals(true);
    cameraPositionZSpinBox->blockSignals(true);
    cameraDistanceSpinBox->blockSignals(true);

    cameraPositionXSpinBox->setValue(x);
    cameraPositionYSpinBox->setValue(y);
    cameraPositionZSpinBox->setValue(z);
    cameraDistanceSpinBox->setValue(d);
        
    cameraPositionXSpinBox->blockSignals(false);
    cameraPositionYSpinBox->blockSignals(false);
    cameraPositionZSpinBox->blockSignals(false);
    cameraDistanceSpinBox->blockSignals(false);
}

void MainWindow::SetCameraRotation(double w, double x, double y, double z) {
    cameraRotationWSpinBox->blockSignals(true);
    cameraRotationXSpinBox->blockSignals(true);
    cameraRotationYSpinBox->blockSignals(true);
    cameraRotationZSpinBox->blockSignals(true);

    cameraRotationWSpinBox->setValue(w);
    cameraRotationXSpinBox->setValue(x);
    cameraRotationYSpinBox->setValue(y);
    cameraRotationZSpinBox->setValue(z);

    cameraRotationWSpinBox->blockSignals(false);
    cameraRotationXSpinBox->blockSignals(false);
    cameraRotationYSpinBox->blockSignals(false);
    cameraRotationZSpinBox->blockSignals(false);
}


///////////////////////////////////////////////////////////////////////////
// Respond to menu events

void MainWindow::on_actionOpenRoofOffset_triggered() {
    // Open a file dialog to read the VTK XML file
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Roof Offset File",
                                                    "",
                                                    "VTK XML PolyData Files (*.vtp)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Open the roof offset file
    pipeline->OpenRoofOffsetFile(fileName.toLatin1().constData());

    RefreshGUI();
}

void MainWindow::on_actionOpenMesh_triggered() {
    // Open a file dialog to read the VTK XML file
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Mesh",
                                                    "",
                                                    "VTK XML Unstructured Grid Files (*.vtu)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Open the roof offset file
    pipeline->OpenMeshFile(fileName.toLatin1().constData());

    RefreshGUI();
}


void MainWindow::on_actionOpenBuildingGeometry_triggered() {
    // Open a file dialog to read the STL file
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Building Geometry File",
                                                    "",
                                                    "STL Files (*.stl)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Open the building geometry file
    pipeline->OpenBuildingFile(fileName.toLatin1().constData());
  
    // Update the GUI
    RefreshBuilding();
}

void MainWindow::on_actionSaveData_triggered() {
    // Open a file dialog to save the text file
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Data",
                                                    "",
                                                    "Text Files (*.txt)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Save a screenshot
    pipeline->SaveData(fileName.toLatin1().constData());
}

void MainWindow::on_actionSaveScreenshot_triggered() {
    // Open a file dialog to save the PNG image
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Screenshot",
                                                    "",
                                                    "PNG Files (*.png)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Save a screenshot
    pipeline->SaveScreenshot(fileName.toLatin1().constData());
}

void MainWindow::on_actionSaveCameraView_triggered() {
    // Open a file dialog to save the text file
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Camera View",
                                                    "",
                                                    "Text Files (*.txt)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Save the current camera view
    pipeline->SaveCameraView(fileName.toLatin1().constData());
}

void MainWindow::on_actionOpenCameraView_triggered() {
    // Open a file dialog to read the text file
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Camera View",
                                                    "",
                                                    "Text Files (*.txt)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Open a saved camera view
    pipeline->OpenCameraView(fileName.toLatin1().constData());
}

void MainWindow::on_actionSaveClipSettings_triggered() {
    // Open a file dialog to read the text file
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Clip Settings",
                                                    "",
                                                    "Text Files (*.txt)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Save the current clipp settings
    pipeline->SaveClipSettings(fileName.toLatin1().constData());
}

void MainWindow::on_actionOpenClipSettings_triggered() {
    // Open a file dialog to read the text file
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open ClipSettings",
                                                    "",
                                                    "Text Files (*.txt)");

    // Check for file name
    if (fileName == "") {
        return;
    }

    // Open a saved clip settings
    pipeline->OpenClipSettings(fileName.toLatin1().constData());

    RefreshGUI();
}

void MainWindow::on_actionExit_triggered() {
    qApp->exit();
}


void MainWindow::on_showDataCheckBox_toggled(bool checked) {
    pipeline->SetShowData(checked);

    pipeline->Render();
}

void MainWindow::on_dataOpacitySlider_sliderMoved(int value) {
    dataOpacityLineEdit->setText(QString().sprintf("%d", value));
}

void MainWindow::on_dataOpacitySlider_valueChanged(int value) {
    dataOpacityLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetDataOpacity((double)value / 100.0);

    pipeline->Render();
}

void MainWindow::on_dataOpacityLineEdit_editingFinished() {
    dataOpacitySlider->setValue(dataOpacityLineEdit->text().toInt());
}

void MainWindow::on_showBuildingGeometryCheckBox_toggled(bool checked) {
    pipeline->SetShowBuilding(checked);

    pipeline->Render();
}


void MainWindow::on_roofOffsetRadioButton_toggled(bool checked) {
    pipeline->SetDataSet(VTKPipeline::RoofOffset);

    // Need to update GUI
    RefreshGUI();

    pipeline->Render();
}

void MainWindow::on_meshRadioButton_toggled(bool checked) {
    pipeline->SetDataSet(VTKPipeline::Mesh);

    // Need to update GUI
    RefreshGUI();

    pipeline->Render();
}


void MainWindow::on_xyMagnitudeRadioButton_toggled(bool checked) {
    pipeline->SetVectorData(VTKPipeline::XYMagnitude);

    // Need to update color map range
    RefreshColorMap();

    pipeline->Render();
}

void MainWindow::on_xyAngleRadioButton_toggled(bool checked) {
    minColorMapSpinBox->setEnabled(!checked);
    maxColorMapSpinBox->setEnabled(!checked);

    pipeline->SetVectorData(VTKPipeline::XYAngle);

    // Need to update color map range
    RefreshColorMap();

    pipeline->Render();
}

void MainWindow::on_zComponentRadioButton_toggled(bool checked) {
    pipeline->SetVectorData(VTKPipeline::ZComponent);

    // Need to update color map range
    RefreshColorMap();

    pipeline->Render();
}


void MainWindow::on_extractRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::Extract);

    clipSizeXSlider->setEnabled(true);
    clipSizeXLineEdit->setEnabled(true);  
    clipSizeYSlider->setEnabled(true);
    clipSizeYLineEdit->setEnabled(true);
    clipSizeZSlider->setEnabled(true);
    clipSizeZLineEdit->setEnabled(true);

    pipeline->Render();
}

void MainWindow::on_fastClipRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::FastClip);

    clipSizeXSlider->setEnabled(true);
    clipSizeXLineEdit->setEnabled(true);  
    clipSizeYSlider->setEnabled(true);
    clipSizeYLineEdit->setEnabled(true);
    clipSizeZSlider->setEnabled(true);
    clipSizeZLineEdit->setEnabled(true);

    pipeline->Render();
}

void MainWindow::on_accurateClipRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::AccurateClip);

    clipSizeXSlider->setEnabled(true);
    clipSizeXLineEdit->setEnabled(true);  
    clipSizeYSlider->setEnabled(true);
    clipSizeYLineEdit->setEnabled(true);
    clipSizeZSlider->setEnabled(true);
    clipSizeZLineEdit->setEnabled(true);

    pipeline->Render();
}

void MainWindow::on_cutXRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::CutX);

    clipSizeXSlider->setEnabled(false);
    clipSizeXLineEdit->setEnabled(false);  
    clipSizeYSlider->setEnabled(true);
    clipSizeYLineEdit->setEnabled(true);
    clipSizeZSlider->setEnabled(true);
    clipSizeZLineEdit->setEnabled(true);

    pipeline->Render();
}

void MainWindow::on_cutYRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::CutY);

    clipSizeXSlider->setEnabled(true);
    clipSizeXLineEdit->setEnabled(true);  
    clipSizeYSlider->setEnabled(false);
    clipSizeYLineEdit->setEnabled(false);
    clipSizeZSlider->setEnabled(true);
    clipSizeZLineEdit->setEnabled(true);

    pipeline->Render();
}

void MainWindow::on_cutZRadioButton_toggled(bool checked) {
    pipeline->SetClipType(VTKPipeline::CutZ);

    clipSizeXSlider->setEnabled(true);
    clipSizeXLineEdit->setEnabled(true);  
    clipSizeYSlider->setEnabled(true);
    clipSizeYLineEdit->setEnabled(true);
    clipSizeZSlider->setEnabled(false);
    clipSizeZLineEdit->setEnabled(false);

    pipeline->Render();
}


void MainWindow::on_clipCenterXSlider_valueChanged(int value) {
    clipCenterXLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxCenter(clipCenterXSlider->value(),
                                   clipCenterYSlider->value(),
                                   clipCenterZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipCenterXLineEdit_editingFinished() {
    clipCenterXSlider->setValue(clipCenterXLineEdit->text().toInt());
}


void MainWindow::on_clipCenterYSlider_valueChanged(int value) {
    clipCenterYLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxCenter(clipCenterXSlider->value(),
                                   clipCenterYSlider->value(),
                                   clipCenterZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipCenterYLineEdit_editingFinished() {
    clipCenterYSlider->setValue(clipCenterYLineEdit->text().toInt());
}


void MainWindow::on_clipCenterZSlider_valueChanged(int value) {
    clipCenterZLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxCenter(clipCenterXSlider->value(),
                                   clipCenterYSlider->value(),
                                   clipCenterZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipCenterZLineEdit_editingFinished() {
    clipCenterZSlider->setValue(clipCenterZLineEdit->text().toInt());
}


void MainWindow::on_clipSizeXSlider_valueChanged(int value) {    
    clipSizeXLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxSize(clipSizeXSlider->value(),
                                 clipSizeYSlider->value(),
                                 clipSizeZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipSizeXLineEdit_editingFinished() {
    clipSizeXSlider->setValue(clipSizeXLineEdit->text().toInt());
}


void MainWindow::on_clipSizeYSlider_valueChanged(int value) {
    clipSizeYLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxSize(clipSizeXSlider->value(),
                                 clipSizeYSlider->value(),
                                 clipSizeZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipSizeYLineEdit_editingFinished() {
    clipSizeYSlider->setValue(clipSizeYLineEdit->text().toInt());
}


void MainWindow::on_clipSizeZSlider_valueChanged(int value) {    
    clipSizeZLineEdit->setText(QString().sprintf("%d", value));

    // Update the pipeline
    pipeline->SetClippingBoxSize(clipSizeXSlider->value(),
                                 clipSizeYSlider->value(),
                                 clipSizeZSlider->value());

    pipeline->Render();
}

void MainWindow::on_clipSizeZLineEdit_editingFinished() {
    clipSizeZSlider->setValue(clipSizeZLineEdit->text().toInt());
}


void MainWindow::on_clipRotationSpinBox_editingFinished() {
    // Update the pipeline
    pipeline->SetClippingBoxRotation(clipRotationSpinBox->value());

    pipeline->Render();
}


void MainWindow::on_applyClipButton_clicked() {
    pipeline->UpdateClipping();

    pipeline->Render();
}



void MainWindow::on_resetClipButton_clicked() {
    pipeline->ResetClippingBox();

    pipeline->Render();

    RefreshGUI();
}

void MainWindow::on_showClipCheckBox_toggled(bool checked) {
    pipeline->SetShowClippingBox(checked);

    pipeline->Render();
}


void MainWindow::on_roofOffsetThicknessSlider_sliderMoved(int value) {
    roofOffsetThicknessLineEdit->setText(QString().sprintf("%d", value));
}

void MainWindow::on_roofOffsetThicknessSlider_valueChanged(int value) {
    roofOffsetThicknessLineEdit->setText(QString().sprintf("%d", value));

    pipeline->SetRoofOffsetThickness(value);

    pipeline->Render();
}

void MainWindow::on_roofOffsetThicknessLineEdit_editingFinished() {
    int value = roofOffsetThicknessLineEdit->text().toInt();
    roofOffsetThicknessSlider->setValue(value);
}


void MainWindow::on_minColorMapSpinBox_editingFinished() {
    // Make the other spin box play nice
    maxColorMapSpinBox->setMinimum(minColorMapSpinBox->value());

    // Update the color map
    pipeline->SetColorMapRange(minColorMapSpinBox->value(), maxColorMapSpinBox->value());

    pipeline->Render();
}

void MainWindow::on_maxColorMapSpinBox_editingFinished() {
    // Make the other spin box play nice
    minColorMapSpinBox->setMaximum(maxColorMapSpinBox->value());

    // Update the color map
    pipeline->SetColorMapRange(minColorMapSpinBox->value(), maxColorMapSpinBox->value());

    pipeline->Render();
}


void MainWindow::on_volumeAreaStatisticsLabelCheckBox_toggled(bool checked) {
    pipeline->SetShowVolumeAreaStatisticsLabel(checked);
    
    pipeline->Render();
}

void MainWindow::on_clippingBoxLabelCheckBox_toggled(bool checked) {
    pipeline->SetShowClippingBoxLabel(checked);
    
    pipeline->Render();
}

void MainWindow::on_cameraLabelCheckBox_toggled(bool checked) {
    pipeline->SetShowCameraLabel(checked);
    
    pipeline->Render();
}

void MainWindow::on_dataStatisticsLabelCheckBox_toggled(bool checked) {
    pipeline->SetShowDataStatisticsLabel(checked);
    
    pipeline->Render();
}

void MainWindow::on_fileNameLabelCheckBox_toggled(bool checked) {
    pipeline->SetShowFileNameLabel(checked);
    
    pipeline->Render();
}


void MainWindow::on_cameraPositionXSpinBox_editingFinished() {
    pipeline->SetCameraPosition(cameraPositionXSpinBox->value(),
                                cameraPositionYSpinBox->value(),
                                cameraPositionZSpinBox->value(),
                                cameraDistanceSpinBox->value());
}

void MainWindow::on_cameraPositionYSpinBox_editingFinished() {
    pipeline->SetCameraPosition(cameraPositionXSpinBox->value(),
                                cameraPositionYSpinBox->value(),
                                cameraPositionZSpinBox->value(),
                                cameraDistanceSpinBox->value());
}

void MainWindow::on_cameraPositionZSpinBox_editingFinished() {
    pipeline->SetCameraPosition(cameraPositionXSpinBox->value(),
                                cameraPositionYSpinBox->value(),
                                cameraPositionZSpinBox->value(),
                                cameraDistanceSpinBox->value());
}

void MainWindow::on_cameraDistanceSpinBox_editingFinished() {
    pipeline->SetCameraPosition(cameraPositionXSpinBox->value(),
                                cameraPositionYSpinBox->value(),
                                cameraPositionZSpinBox->value(),
                                cameraDistanceSpinBox->value());
}


void MainWindow::on_cameraRotationWSpinBox_editingFinished() {
    pipeline->SetCameraRotation(cameraRotationWSpinBox->value(),
                                cameraRotationXSpinBox->value(),
                                cameraRotationYSpinBox->value(),
                                cameraRotationZSpinBox->value());
}

void MainWindow::on_cameraRotationXSpinBox_editingFinished() {
    pipeline->SetCameraRotation(cameraRotationWSpinBox->value(),
                                cameraRotationXSpinBox->value(),
                                cameraRotationYSpinBox->value(),
                                cameraRotationZSpinBox->value());
}

void MainWindow::on_cameraRotationYSpinBox_editingFinished() {
    pipeline->SetCameraRotation(cameraRotationWSpinBox->value(),
                                cameraRotationXSpinBox->value(),
                                cameraRotationYSpinBox->value(),
                                cameraRotationZSpinBox->value());
}

void MainWindow::on_cameraRotationZSpinBox_editingFinished() {
    pipeline->SetCameraRotation(cameraRotationWSpinBox->value(),
                                cameraRotationXSpinBox->value(),
                                cameraRotationYSpinBox->value(),
                                cameraRotationZSpinBox->value());
}


void MainWindow::on_resetCameraXButton_clicked() {
    pipeline->ResetCameraX();
}

void MainWindow::on_resetCameraYButton_clicked() {
    pipeline->ResetCameraY();
}

void MainWindow::on_resetCameraZButton_clicked() {
    pipeline->ResetCameraZ();
}


void MainWindow::RefreshGUI() {
    bool hasBuilding = pipeline->HasBuilding();
    bool hasRoofOffset = pipeline->HasRoofOffset();
    bool hasMesh = pipeline->HasMesh();
    bool hasData = hasRoofOffset || hasMesh;

    // Enable/disable widgets    
    showDataCheckBox->setEnabled(hasData);
    dataOpacitySlider->setEnabled(hasData);
    dataOpacityLineEdit->setEnabled(hasData);
    showBuildingGeometryCheckBox->setEnabled(hasBuilding);

    showBuildingGeometryCheckBox->blockSignals(true);
    showBuildingGeometryCheckBox->setChecked(hasBuilding && pipeline->GetShowBuilding());
    showBuildingGeometryCheckBox->blockSignals(false);

    roofOffsetRadioButton->setEnabled(hasRoofOffset);
    meshRadioButton->setEnabled(hasMesh);

    xyMagnitudeRadioButton->setEnabled(hasData);
    xyAngleRadioButton->setEnabled(hasData);
    zComponentRadioButton->setEnabled(hasData);

    extractRadioButton->setEnabled(hasData && pipeline->GetDataSet() != VTKPipeline::RoofOffset);
    fastClipRadioButton->setEnabled(hasData && pipeline->GetDataSet() != VTKPipeline::RoofOffset);
    accurateClipRadioButton->setEnabled(hasData);
    cutXRadioButton->setEnabled(hasData && pipeline->GetDataSet() != VTKPipeline::RoofOffset);
    cutYRadioButton->setEnabled(hasData && pipeline->GetDataSet() != VTKPipeline::RoofOffset);
    cutZRadioButton->setEnabled(hasData && pipeline->GetDataSet() != VTKPipeline::RoofOffset);

    clipCenterXSlider->setEnabled(hasData);
    clipCenterXLineEdit->setEnabled(hasData);  
    clipCenterYSlider->setEnabled(hasData);
    clipCenterYLineEdit->setEnabled(hasData);
    clipCenterZSlider->setEnabled(hasData);
    clipCenterZLineEdit->setEnabled(hasData);

    clipSizeXSlider->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutX);
    clipSizeXLineEdit->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutX);  
    clipSizeYSlider->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutY);
    clipSizeYLineEdit->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutY);
    clipSizeZSlider->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutZ);
    clipSizeZLineEdit->setEnabled(hasData && pipeline->GetClipType() != VTKPipeline::CutZ);

    clipRotationSpinBox->setEnabled(hasData);

    applyClipButton->setEnabled(hasData);
    resetClipButton->setEnabled(hasData);
    showClipCheckBox->setEnabled(hasData);

    roofOffsetThicknessSlider->setEnabled(hasRoofOffset && pipeline->GetDataSet() == VTKPipeline::RoofOffset);
    roofOffsetThicknessLineEdit->setEnabled(hasRoofOffset && pipeline->GetDataSet() == VTKPipeline::RoofOffset);

    minColorMapSpinBox->setEnabled(hasData && pipeline->GetVectorData() != VTKPipeline::XYAngle);
    maxColorMapSpinBox->setEnabled(hasData && pipeline->GetVectorData() != VTKPipeline::XYAngle);

    volumeAreaStatisticsLabelCheckBox->setEnabled(hasData);
    clippingBoxLabelCheckBox->setEnabled(hasData);
    cameraLabelCheckBox->setEnabled(hasData || hasBuilding);
    dataStatisticsLabelCheckBox->setEnabled(hasData);
    fileNameLabelCheckBox->setEnabled(hasData);

    cameraPositionXSpinBox->setEnabled(hasData || hasBuilding);
    cameraPositionYSpinBox->setEnabled(hasData || hasBuilding);
    cameraPositionZSpinBox->setEnabled(hasData || hasBuilding);
    cameraDistanceSpinBox->setEnabled(hasData || hasBuilding);

    cameraRotationWSpinBox->setEnabled(hasData || hasBuilding);
    cameraRotationXSpinBox->setEnabled(hasData || hasBuilding);
    cameraRotationYSpinBox->setEnabled(hasData || hasBuilding);
    cameraRotationZSpinBox->setEnabled(hasData || hasBuilding);

    resetCameraXButton->setEnabled(hasData || hasBuilding);
    resetCameraYButton->setEnabled(hasData || hasBuilding);
    resetCameraZButton->setEnabled(hasData || hasBuilding);

    if (hasData) {
        // Add the current file to the status bar
//        statusBarLabel->setText(pipeline->GetCurrentFileName());


        // Data visibility
        showDataCheckBox->blockSignals(true);
        dataOpacitySlider->blockSignals(true);

        showDataCheckBox->setChecked(hasData && pipeline->GetShowData());
        dataOpacitySlider->setValue(pipeline->GetDataOpacity() * 100);
        dataOpacityLineEdit->setText(QString().sprintf("%d", dataOpacitySlider->value()));  

        showDataCheckBox->blockSignals(false);
        dataOpacitySlider->blockSignals(false);


        // Data set
        roofOffsetRadioButton->blockSignals(true);
        meshRadioButton->blockSignals(true);

        roofOffsetRadioButton->setChecked(pipeline->GetDataSet() == VTKPipeline::RoofOffset);
        meshRadioButton->setChecked(pipeline->GetDataSet() == VTKPipeline::Mesh);

        roofOffsetRadioButton->blockSignals(false);
        meshRadioButton->blockSignals(false);


        // Vector data
        xyMagnitudeRadioButton->blockSignals(true);
        xyAngleRadioButton->blockSignals(true);
        zComponentRadioButton->blockSignals(true);

        xyMagnitudeRadioButton->setChecked(pipeline->GetVectorData() == VTKPipeline::XYMagnitude);
        xyAngleRadioButton->setChecked(pipeline->GetVectorData() == VTKPipeline::XYAngle);
        zComponentRadioButton->setChecked(pipeline->GetVectorData() == VTKPipeline::ZComponent);

        xyMagnitudeRadioButton->blockSignals(false);
        xyAngleRadioButton->blockSignals(false);
        zComponentRadioButton->blockSignals(false);


        // Clipping type
        extractRadioButton->blockSignals(true);
        fastClipRadioButton->blockSignals(true);
        accurateClipRadioButton->blockSignals(true);
        cutXRadioButton->blockSignals(true);
        cutYRadioButton->blockSignals(true);
        cutZRadioButton->blockSignals(true);

        extractRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::Extract);
        fastClipRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::FastClip);
        accurateClipRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::AccurateClip);
        cutXRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::CutX);
        cutYRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::CutY);
        cutZRadioButton->setChecked(pipeline->GetClipType() == VTKPipeline::CutZ);

        extractRadioButton->blockSignals(false);
        fastClipRadioButton->blockSignals(false);
        accurateClipRadioButton->blockSignals(false);        
        cutXRadioButton->blockSignals(false);
        cutYRadioButton->blockSignals(false);
        cutZRadioButton->blockSignals(false);


        // Bounds for clipping plane
        double bounds[6];
        pipeline->GetBounds(bounds);

        int newBounds[6];
        RecomputeBounds(bounds, newBounds);

        // Turn off signals
        clipCenterXSlider->blockSignals(true);
        clipCenterYSlider->blockSignals(true);
        clipCenterZSlider->blockSignals(true);

        clipSizeXSlider->blockSignals(true);
        clipSizeYSlider->blockSignals(true);
        clipSizeZSlider->blockSignals(true);

        clipRotationSpinBox->blockSignals(true);

        // Set the ranges and ticks

        clipCenterXSlider->setRange(newBounds[0], newBounds[1]);
        clipCenterYSlider->setRange(newBounds[2], newBounds[3]);
        clipCenterZSlider->setRange(newBounds[4], newBounds[5]);

        clipCenterXSlider->setTickInterval((newBounds[1] - newBounds[0]) / 10);
        clipCenterYSlider->setTickInterval((newBounds[3] - newBounds[2]) / 10);
        clipCenterZSlider->setTickInterval((newBounds[5] - newBounds[4]) / 10);

        clipSizeXSlider->setRange(1, newBounds[1] - newBounds[0]);
        clipSizeYSlider->setRange(1, newBounds[3] - newBounds[2]);
        clipSizeZSlider->setRange(1, newBounds[5] - newBounds[4]);

        clipSizeXSlider->setTickInterval((newBounds[1] - newBounds[0]) / 10);
        clipSizeYSlider->setTickInterval((newBounds[3] - newBounds[2]) / 10);
        clipSizeZSlider->setTickInterval((newBounds[5] - newBounds[4]) / 10);

        // Set the validator ranges
        clipCenterXValidator->setRange(clipCenterXSlider->minimum(), clipCenterXSlider->maximum());
        clipCenterYValidator->setRange(clipCenterYSlider->minimum(), clipCenterYSlider->maximum());
        clipCenterZValidator->setRange(clipCenterZSlider->minimum(), clipCenterZSlider->maximum());

        clipSizeXValidator->setRange(clipSizeXSlider->minimum(), clipSizeXSlider->maximum());
        clipSizeYValidator->setRange(clipSizeYSlider->minimum(), clipSizeYSlider->maximum());
        clipSizeZValidator->setRange(clipSizeZSlider->minimum(), clipSizeZSlider->maximum());

        // Set the values
        double center[3];
        double size[3];

        pipeline->GetClippingBoxCenter(center);
        pipeline->GetClippingBoxSize(size);

        clipCenterXSlider->setValue(center[0]);
        clipCenterYSlider->setValue(center[1]);
        clipCenterZSlider->setValue(center[2]);

        clipSizeXSlider->setValue(size[0]);
        clipSizeYSlider->setValue(size[1]);
        clipSizeZSlider->setValue(size[2]);

        clipRotationSpinBox->setValue(pipeline->GetClippingBoxRotation());

        // Set the line edits
        clipCenterXLineEdit->setText(QString().sprintf("%d", clipCenterXSlider->value()));        
        clipCenterYLineEdit->setText(QString().sprintf("%d", clipCenterYSlider->value()));        
        clipCenterZLineEdit->setText(QString().sprintf("%d", clipCenterZSlider->value()));
        
        clipSizeXLineEdit->setText(QString().sprintf("%d", clipSizeXSlider->value()));        
        clipSizeYLineEdit->setText(QString().sprintf("%d", clipSizeYSlider->value()));        
        clipSizeZLineEdit->setText(QString().sprintf("%d", clipSizeZSlider->value()));

        // Turn on signals
        clipCenterXSlider->blockSignals(false);
        clipCenterYSlider->blockSignals(false);
        clipCenterZSlider->blockSignals(false);

        clipSizeXSlider->blockSignals(false);
        clipSizeYSlider->blockSignals(false);
        clipSizeZSlider->blockSignals(false);

        clipRotationSpinBox->blockSignals(false);


        // Other clipping widgets
        showClipCheckBox->blockSignals(true);

        showClipCheckBox->setChecked(pipeline->GetShowClippingBox());

        showClipCheckBox->blockSignals(false);


        // Roof offset thickness
        roofOffsetThicknessSlider->blockSignals(true);

        roofOffsetThicknessSlider->setValue(pipeline->GetRoofOffsetThickness());        
        roofOffsetThicknessLineEdit->setText(QString().sprintf("%d", roofOffsetThicknessSlider->value()));

        roofOffsetThicknessSlider->blockSignals(false);


        // Labels
        volumeAreaStatisticsLabelCheckBox->blockSignals(true);
        clippingBoxLabelCheckBox->blockSignals(true);
        cameraLabelCheckBox->blockSignals(true);
        dataStatisticsLabelCheckBox->blockSignals(true);
        fileNameLabelCheckBox->blockSignals(true);

        volumeAreaStatisticsLabelCheckBox->setChecked(pipeline->GetShowVolumeAreaStatisticsLabel());
        clippingBoxLabelCheckBox->setChecked(pipeline->GetShowClippingBoxLabel());
        cameraLabelCheckBox->setChecked(pipeline->GetShowCameraLabel());
        dataStatisticsLabelCheckBox->setChecked(pipeline->GetShowDataStatisticsLabel());
        fileNameLabelCheckBox->setChecked(pipeline->GetShowFileNameLabel());

        volumeAreaStatisticsLabelCheckBox->blockSignals(false);
        clippingBoxLabelCheckBox->blockSignals(false);
        cameraLabelCheckBox->blockSignals(false);
        dataStatisticsLabelCheckBox->blockSignals(false);
        fileNameLabelCheckBox->blockSignals(false);


        // Camera
        cameraPositionXSpinBox->blockSignals(true);
        cameraPositionYSpinBox->blockSignals(true);
        cameraPositionZSpinBox->blockSignals(true);
        cameraDistanceSpinBox->blockSignals(true);

        cameraRotationWSpinBox->blockSignals(true);
        cameraRotationXSpinBox->blockSignals(true);
        cameraRotationYSpinBox->blockSignals(true);
        cameraRotationZSpinBox->blockSignals(true);

        cameraPositionXSpinBox->setRange(bounds[0] - (bounds[1] - bounds[0]) * 10.0,
                                         bounds[1] + (bounds[1] - bounds[0]) * 10.0);
        cameraPositionYSpinBox->setRange(bounds[2] - (bounds[3] - bounds[2]) * 10.0,
                                         bounds[3] + (bounds[3] - bounds[2]) * 10.0);
        cameraPositionZSpinBox->setRange(bounds[4] - (bounds[5] - bounds[4]) * 10.0,
                                         bounds[5] + (bounds[5] - bounds[4]) * 10.0);
        cameraDistanceSpinBox->setRange(0.0,
                                        bounds[1] + (bounds[1] - bounds[0]) * 10.0);
        
        cameraPositionXSpinBox->blockSignals(false);
        cameraPositionYSpinBox->blockSignals(false);
        cameraPositionZSpinBox->blockSignals(false);
        cameraDistanceSpinBox->blockSignals(false);

        cameraRotationWSpinBox->blockSignals(false);
        cameraRotationXSpinBox->blockSignals(false);
        cameraRotationYSpinBox->blockSignals(false);
        cameraRotationZSpinBox->blockSignals(false);


        // Refresh the color map
        RefreshColorMap();
    }
}


void MainWindow::RefreshBuilding() {
    bool hasBuilding = pipeline->HasBuilding();

    showBuildingGeometryCheckBox->setEnabled(hasBuilding);
    showBuildingGeometryCheckBox->setChecked(hasBuilding && pipeline->GetShowBuilding());

    cameraLabelCheckBox->setEnabled(hasBuilding);
    cameraLabelCheckBox->setChecked(hasBuilding && pipeline->GetShowCameraLabel());

    // Camera
    cameraPositionXSpinBox->setEnabled(hasBuilding);
    cameraPositionYSpinBox->setEnabled(hasBuilding);
    cameraPositionZSpinBox->setEnabled(hasBuilding);
    cameraDistanceSpinBox->setEnabled(hasBuilding);

    cameraRotationWSpinBox->setEnabled(hasBuilding);
    cameraRotationXSpinBox->setEnabled(hasBuilding);
    cameraRotationYSpinBox->setEnabled(hasBuilding);
    cameraRotationZSpinBox->setEnabled(hasBuilding);

    cameraPositionXSpinBox->blockSignals(true);
    cameraPositionYSpinBox->blockSignals(true);
    cameraPositionZSpinBox->blockSignals(true);
    cameraDistanceSpinBox->blockSignals(true);

    cameraRotationWSpinBox->blockSignals(true);
    cameraRotationXSpinBox->blockSignals(true);
    cameraRotationYSpinBox->blockSignals(true);
    cameraRotationZSpinBox->blockSignals(true);

    cameraPositionXSpinBox->setRange(-10000.0, 10000.0);
    cameraPositionYSpinBox->setRange(-10000.0, 10000.0);
    cameraPositionZSpinBox->setRange(-10000.0, 10000.0);
    cameraDistanceSpinBox->setRange(0.0, 10000.0);
    
    cameraPositionXSpinBox->blockSignals(false);
    cameraPositionYSpinBox->blockSignals(false);
    cameraPositionZSpinBox->blockSignals(false);
    cameraDistanceSpinBox->blockSignals(false);

    cameraRotationWSpinBox->blockSignals(false);
    cameraRotationXSpinBox->blockSignals(false);
    cameraRotationYSpinBox->blockSignals(false);
    cameraRotationZSpinBox->blockSignals(false);
}

void MainWindow::RefreshColorMap() {
    double range[2];

    if (pipeline->GetVectorData() == VTKPipeline::XYAngle) {
        range[0] = 0.0;
        range[1] = 360.0;
    }
    else {
        pipeline->GetDataRange(range);
    }

    minColorMapSpinBox->setRange(range[0], range[1]);
    minColorMapSpinBox->setValue(range[0]);

    maxColorMapSpinBox->setRange(range[0], range[1]);
    maxColorMapSpinBox->setValue(range[1]);
}


void MainWindow::RecomputeBounds(double in[6], int out[6]) {
    // Make the floating point input bounds play nice with 
    // the integer output bounds for integer sliders
    for (int i = 0; i < 6; i += 2) {\
        // Watch out for negative zero...
        if (in[i] == -0.0) out[i] = 0.0;
        else out[i] = floor(in[i]);
    }
    for (int i = 1; i < 6; i += 2) {
        out[i] = ceil(in[i]);
    }
}
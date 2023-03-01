/*=========================================================================

  Name:        MainWindow.h

  Author:      David Borland, The Renaissance Computing Institute (RENCI)

  Copyright:   The Renaissance Computing Institute (RENCI)

  License:     Licensed under the RENCI Open Source Software License v. 1.0

               See included License.txt or
               http://www.renci.org/resources/open-source-software-license
               for details.

  Description: The main window for the Aeolus urban wind flow
               visualization program

=========================================================================*/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <qmainwindow.h>

#include "ui_MainWindow.h"


class VTKPipeline;


class MainWindow : public QMainWindow, private Ui_MainWindow {
    Q_OBJECT

public:
    // Constructor/destructor
    MainWindow(QWidget* parent = NULL);
    virtual ~MainWindow();

    // Called from VTKPipeline
    void SetCameraPosition(double x, double y, double z, double d);
    void SetCameraRotation(double w, double x, double y, double z);

public slots:
    // Use Qt's auto-connect magic to tie GUI widgets to slots,
    // removing the need to call connect() explicitly.
    // Names of the methods must follow the naming convention
    // on_<widget name>_<signal name>(<signal parameters>).

    // Menu events
    virtual void on_actionOpenRoofOffset_triggered();
    virtual void on_actionOpenMesh_triggered();
    virtual void on_actionOpenBuildingGeometry_triggered();
    virtual void on_actionSaveData_triggered();
    virtual void on_actionSaveScreenshot_triggered();
    virtual void on_actionSaveCameraView_triggered();
    virtual void on_actionOpenCameraView_triggered();
    virtual void on_actionSaveClipSettings_triggered();
    virtual void on_actionOpenClipSettings_triggered();
    virtual void on_actionExit_triggered();

    // Widget events
    virtual void on_showDataCheckBox_toggled(bool checked);
    virtual void on_dataOpacitySlider_sliderMoved(int value);
    virtual void on_dataOpacitySlider_valueChanged(int value);
    virtual void on_dataOpacityLineEdit_editingFinished();
    virtual void on_showBuildingGeometryCheckBox_toggled(bool checked);

    virtual void on_roofOffsetRadioButton_toggled(bool checked);
    virtual void on_meshRadioButton_toggled(bool checked);

    virtual void on_xyMagnitudeRadioButton_toggled(bool checked);
    virtual void on_xyAngleRadioButton_toggled(bool checked);
    virtual void on_zComponentRadioButton_toggled(bool checked);

    virtual void on_extractRadioButton_toggled(bool checked);
    virtual void on_fastClipRadioButton_toggled(bool checked);
    virtual void on_accurateClipRadioButton_toggled(bool checked);
    virtual void on_cutXRadioButton_toggled(bool checked);
    virtual void on_cutYRadioButton_toggled(bool checked);
    virtual void on_cutZRadioButton_toggled(bool checked);

    virtual void on_clipCenterXSlider_valueChanged(int value);
    virtual void on_clipCenterXLineEdit_editingFinished();

    virtual void on_clipCenterYSlider_valueChanged(int value);
    virtual void on_clipCenterYLineEdit_editingFinished();

    virtual void on_clipCenterZSlider_valueChanged(int value);
    virtual void on_clipCenterZLineEdit_editingFinished();

    virtual void on_clipSizeXSlider_valueChanged(int value);
    virtual void on_clipSizeXLineEdit_editingFinished();

    virtual void on_clipSizeYSlider_valueChanged(int value);
    virtual void on_clipSizeYLineEdit_editingFinished();

    virtual void on_clipSizeZSlider_valueChanged(int value);
    virtual void on_clipSizeZLineEdit_editingFinished();

    virtual void on_clipRotationSpinBox_editingFinished();

    virtual void on_applyClipButton_clicked();
    virtual void on_resetClipButton_clicked();
    virtual void on_showClipCheckBox_toggled(bool checked);

    virtual void on_roofOffsetThicknessSlider_sliderMoved(int value);
    virtual void on_roofOffsetThicknessSlider_valueChanged(int value);
    virtual void on_roofOffsetThicknessLineEdit_editingFinished();

    virtual void on_minColorMapSpinBox_editingFinished();
    virtual void on_maxColorMapSpinBox_editingFinished();

    virtual void on_volumeAreaStatisticsLabelCheckBox_toggled(bool checked);
    virtual void on_clippingBoxLabelCheckBox_toggled(bool checked);
    virtual void on_cameraLabelCheckBox_toggled(bool checked);
    virtual void on_dataStatisticsLabelCheckBox_toggled(bool checked);
    virtual void on_fileNameLabelCheckBox_toggled(bool checked);
    
    virtual void on_cameraPositionXSpinBox_editingFinished();
    virtual void on_cameraPositionYSpinBox_editingFinished();
    virtual void on_cameraPositionZSpinBox_editingFinished();
    virtual void on_cameraDistanceSpinBox_editingFinished();

    virtual void on_cameraRotationWSpinBox_editingFinished();
    virtual void on_cameraRotationXSpinBox_editingFinished();
    virtual void on_cameraRotationYSpinBox_editingFinished();
    virtual void on_cameraRotationZSpinBox_editingFinished();

    virtual void on_resetCameraXButton_clicked();
    virtual void on_resetCameraYButton_clicked();
    virtual void on_resetCameraZButton_clicked();

protected:
    VTKPipeline* pipeline;

    QIntValidator* clipCenterXValidator;
    QIntValidator* clipCenterYValidator;
    QIntValidator* clipCenterZValidator;
    
    QIntValidator* clipSizeXValidator;
    QIntValidator* clipSizeYValidator;
    QIntValidator* clipSizeZValidator;

//    QLabel* statusBarLabel;

    // Set GUI widget values from the VTK pipeline
    void RefreshGUI();

    void RefreshBuilding();
    void RefreshColorMap();

    void RecomputeBounds(double in[6], int out[6]);
};


#endif

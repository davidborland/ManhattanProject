///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        NormalizeData.cpp
//
// Author:      David Borland
//
// Description: Normalize wind speeds for Manhattan data
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include <vtkAppendFilter.h>
#include <vtkArrayCalculator.h>
#include <vtkCompositeDataIterator.h>
#include <vtkDataArraySelection.h>
#include <vtkDataSetCollection.h>
#include <vtkEnSightGoldBinaryReader.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkWin32ProcessOutputWindow.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <fstream>
#include <string>

#include <stdio.h>


int main(int argc, char* argv[]) {
    std::cout << endl;

    if (argc != 3) {
        std::cout << "Must supply file name and scale factor" << std::endl;
        return -1;
    }

    std::string inputName = argv[1];
    double scale = 1.0 / atof(argv[2]);


    // Set VTK output window 
    vtkWin32ProcessOutputWindow* outputWindow = vtkWin32ProcessOutputWindow::New();
    vtkOutputWindow::SetInstance(outputWindow);
    outputWindow->Delete();


    // Read the data
    vtkEnSightGoldBinaryReader* reader = vtkEnSightGoldBinaryReader::New();
    reader->SetCaseFileName(inputName.c_str());
    reader->ReadAllVariablesOff();
    reader->GetPointDataArraySelection()->DisableAllArrays();
    reader->GetPointDataArraySelection()->EnableArray("velocity");
    std::cout << "Reading " << inputName << std::endl;
    reader->Update();

    // Append multi-block data into one unstructured grid
    vtkAppendFilter* append = vtkAppendFilter::New();
    vtkCompositeDataIterator* iterator = reader->GetOutput()->NewIterator();
    while (!iterator->IsDoneWithTraversal()) {
        append->AddInput(reader->GetOutput()->GetDataSet(iterator));
        iterator->GoToNextItem();
    }
    append->Update();

    // Do the scaling
    vtkArrayCalculator* calculator = vtkArrayCalculator::New();
    calculator->SetInputConnection(append->GetOutputPort());
    calculator->AddVectorArrayName("velocity");
    calculator->SetResultArrayName("velocityNorm");
    char function[128];
    sprintf(function, "velocity * %f", scale);
    calculator->SetFunction(function);

    // Write the scaled data
    vtkXMLUnstructuredGridWriter* writer = vtkXMLUnstructuredGridWriter::New();
    std::string outputName = inputName;
    outputName += ".";
    outputName += writer->GetDefaultFileExtension();
    writer->SetFileName(outputName.c_str());
    writer->SetInputConnection(calculator->GetOutputPort());    
    std::cout << "Writing " << outputName << std::endl << std::endl;
    writer->Write();

    // Clean up
    writer->Delete();
    calculator->Delete();
    append->Delete();
    reader->Delete();

    return 0;
}
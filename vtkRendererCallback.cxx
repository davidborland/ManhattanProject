/*=========================================================================

  Name:        vtkRendererCallback.cxx

  Author:      David Borland, The Renaissance Computing Institute (RENCI)

  Copyright:   The Renaissance Computing Institute (RENCI)

  License:     Licensed under the RENCI Open Source Software License v. 1.0

               See included License.txt or
               http://www.renci.org/resources/open-source-software-license
               for details.

  Description: Callback to do stuff when a new image is rendered.

=========================================================================*/

#include "vtkRendererCallback.h"

#include "VTKPipeline.h"

vtkRendererCallback::vtkRendererCallback() 
{
    pipeline = NULL;
}

vtkRendererCallback* vtkRendererCallback::New() { 
    return new vtkRendererCallback; 
}

void vtkRendererCallback::SetVTKPipeline(VTKPipeline* vtkPipeline) {
    pipeline = vtkPipeline;
}

void vtkRendererCallback::Execute(vtkObject* caller, unsigned long eventId, void* callData) {
    if (!pipeline) return;

    if (eventId != StartEvent) return;

    pipeline->UpdateCamera();
}
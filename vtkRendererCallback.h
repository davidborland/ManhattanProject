/*=========================================================================

  Name:        vtkRendererCallback.h

  Author:      David Borland, The Renaissance Computing Institute (RENCI)

  Copyright:   The Renaissance Computing Institute (RENCI)

  License:     Licensed under the RENCI Open Source Software License v. 1.0

               See included License.txt or
               http://www.renci.org/resources/open-source-software-license
               for details.

  Description: Callback to do stuff when a new image is rendered.

=========================================================================*/


#ifndef __vtkRendererCallback_h
#define __vtkRendererCallback_h

#include <vtkCommand.h>

class VTKPipeline;

class vtkRendererCallback : public vtkCommand {
public:
    vtkRendererCallback();
    static vtkRendererCallback* New();

    void SetVTKPipeline(VTKPipeline* vtkPipeline);

    virtual void Execute(vtkObject* caller, unsigned long eventId, void* callData);

protected:
    VTKPipeline* pipeline;
};

#endif
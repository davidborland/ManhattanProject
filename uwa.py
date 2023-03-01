# Name:        uwa.py
# Author:      David Borland, The Renaissance Computing Institute (RENCI)
#
# Copyright:   The Renaissance Computing Institute (RENCI)
#
# License:     Licensed under the RENCI Open Source Software License v. 1.0
#
#              See included License.txt or
#              http://www.renci.org/resources/open-source-software-license
#              for details.


# Need VTK
from vtk import *


# Normalizes the velocity field and generates useful quantities
def process_ensight(ensight_input_name, vtk_output_name, scale_factor = 1.0):
    """Normalizes the input velocity field by multiplying by the scale_factor and
    generates useful quantities for visualization and analysis

    ensight_input_name:  The input EnSight file containing the wind velocity field
    vtk_output_name:  The output VTK file name (file extension should be .vtu)
    scale_factor:  The scale factor applied to the wind vectors for normalizing
    """
    
    # Load the wind velocity data from the EnSight file
    windReader = vtkEnSightGoldBinaryReader()
    windReader.SetCaseFileName(ensight_input_name)
    windReader.ReadAllVariablesOff()
    windReader.GetPointDataArraySelection().DisableAllArrays()
    windReader.GetPointDataArraySelection().EnableArray("velocity")

    print "Reading " + ensight_input_name
    windReader.Update()
    print "Finished\n"


    # Append multi-block data into one unstructured grid
    append = vtkAppendFilter()
    iterator = windReader.GetOutput().NewIterator()
    while not iterator.IsDoneWithTraversal():
        append.AddInput(windReader.GetOutput().GetDataSet(iterator))
        iterator.GoToNextItem()

    print "Appending data"
    append.Update()
    print "Finished\n"  


    # Make sure we only have triangles in the output
    triangle = vtkDataSetTriangleFilter()
    triangle.SetInputConnection(append.GetOutputPort())
    triangle.TetrahedraOnlyOn()
    

    # Apply the normalizing scale factor
    normalize = vtkArrayCalculator()
    normalize.SetInputConnection(triangle.GetOutputPort())
    normalize.AddVectorArrayName("velocity", 0, 1, 2)
    normalize.SetResultArrayName("velocityNorm")
    function = "velocity * %f" % scale_factor
    normalize.SetFunction(function)

    print "Applying normalizing scale factor"
    normalize.Update()
    print "Finished\n"  


    # Extract the normalized Z component
    z = vtkArrayCalculator()
    z.SetInputConnection(normalize.GetOutputPort())
    z.AddVectorArrayName("velocityNorm", 0, 1, 2)
    z.SetResultArrayName("velocityNormZ")
    z.SetFunction("velocityNorm . kHat")

    print "Extracting normalized z velocity component"
    z.Update()
    print "Finished\n"


    # Extract the normalized XY vector
    xy = vtkArrayCalculator()
    xy.SetInputConnection(z.GetOutputPort());
    xy.AddVectorArrayName("velocityNorm", 0, 1, 2)
    xy.SetResultArrayName("velocityNormXY");
    xy.SetFunction("(velocityNorm . iHat) * iHat + (velocityNorm . jHat) * jHat")

    print "Extracting normalized xy velocity vector"
    xy.Update()
    print "Finished\n"


    # Compute the normalized XY vector magnitude
    xyMagnitude = vtkArrayCalculator()
    xyMagnitude.SetInputConnection(xy.GetOutputPort());
    xyMagnitude.AddVectorArrayName("velocityNormXY", 0, 1, 2)
    xyMagnitude.SetResultArrayName("velocityNormXYMag");
    xyMagnitude.SetFunction("mag(velocityNormXY)")

    print "Computing normalized xy velocity vector magnitude"
    xyMagnitude.Update()
    print "Finished\n"


    # Normalize the normalized XY vector
    xyDirection = vtkArrayCalculator()
    xyDirection.SetInputConnection(xyMagnitude.GetOutputPort())
    xyDirection.AddVectorArrayName("velocityNormXY", 0, 1, 2)
    xyDirection.SetResultArrayName("velocityNormXYDirection")
    xyDirection.SetFunction("norm(velocityNormXY)")

    print "Computing normalized xy velocity vector direction"
    xyDirection.Update()
    print "Finished\n"    
    

    # Compute the normalized XY vector angle clockwise from 0 to 360 degrees, with positive Y as 0 degrees
    # We want the *incoming* wind direction, which is the angle of the negative wind vector
    xyAngle1 = vtkArrayCalculator()
    xyAngle1.SetInputConnection(xyDirection.GetOutputPort())
    xyAngle1.AddVectorArrayName("velocityNormXYDirection", 0, 1, 2)
    xyAngle1.SetResultArrayName("velocityNormXYAngle")
    xyAngle1.SetFunction("-(acos(-(velocityNormXYDirection . iHat)) * 180.0 / 3.14159265 * sign(-(velocityNormXYDirection . jHat)) - 90.0)")

    xyAngle2 = vtkArrayCalculator()
    xyAngle2.SetInputConnection(xyAngle1.GetOutputPort())
    xyAngle2.AddScalarArrayName("velocityNormXYAngle", 0)
    xyAngle2.SetResultArrayName("velocityNormXYAngle")
    xyAngle2.SetFunction("velocityNormXYAngle + 360.0 * (velocityNormXYAngle < 0.0)")
    
    print "Computing normalized xy velocity vector angle"
    xyAngle2.Update()
    print "Finished\n"


    # Remove unnecessary data arrays
    xyAngle2.GetOutput().GetPointData().RemoveArray("velocity")
    xyAngle2.GetOutput().GetPointData().RemoveArray("velocityNorm")
    xyAngle2.GetOutput().GetPointData().RemoveArray("velocityNormXY")


    # Save the normalized data
    writer = vtkXMLUnstructuredGridWriter()
    writer.SetInputConnection(xyAngle2.GetOutputPort())
    writer.SetFileName(vtk_output_name)

    print "Saving " + vtk_output_name
    writer.Write()
    print "Finished\n"


# Extracts the velocity field a given distance from the roofs
def roof_offset(vtk_input_name, stl_input_name, output_name, offset):
    """Extracts the velocity field a given distance from the roofs of the input geometry

    vtk_input_name:  The input VTK file containing the wind velocity field (file extension should be .vtu)
    stl_input_name:  The input STL file containing the building geometry (file extenstion should be .stl)
    output_name:  The output VTK file name (file extension should be .vtp)
    offset:  The distance from the roofs at which to sample the wind velocity data
    """
    
    # Load the wind velocity data from the VTK file
    windReader = vtkXMLUnstructuredGridReader()
    windReader.SetFileName(vtk_input_name)

    print "Reading " + vtk_input_name
    windReader.Update()
    print "Finished\n"                 


    # Load the building data from the STL file
    buildingReader = vtkSTLReader()
    buildingReader.SetFileName(stl_input_name)

    print "Reading " + stl_input_name
    buildingReader.Update()
    print "Finished\n"


    # Generate normals for the building data
    normals = vtkPolyDataNormals()
    normals.SetInputConnection(buildingReader.GetOutputPort())
    normals.SetFeatureAngle(0.0)
    normals.SplittingOn()
    normals.ConsistencyOff()
    normals.NonManifoldTraversalOn()
    normals.ComputePointNormalsOff()
    normals.ComputeCellNormalsOn()


    # Extract the z-component of the normals
    zNormal = vtkArrayCalculator()
    zNormal.SetInputConnection(normals.GetOutputPort())
    zNormal.SetAttributeModeToUseCellData()
    zNormal.AddVectorArrayName("Normals", 0, 1, 2)
    zNormal.SetResultArrayName("ZNormal")
    zNormal.SetFunction("Normals . kHat")


    # Threshold the normals so we only have upward-facing normals, signifying roofs
    threshold = vtkThreshold()
    threshold.SetInputConnection(zNormal.GetOutputPort())
    threshold.ThresholdByUpper(0.44)


    # Translate the extracted roofs vertically
    t = vtkTransform()
    t.Translate(0.0, 0.0, offset)

    transform = vtkTransformFilter()
    transform.SetInputConnection(threshold.GetOutputPort())
    transform.SetTransform(t)


    # Sample the wind velocity field at the translated roofs
    probe = vtkProbeFilter()
    probe.SetInputConnection(transform.GetOutputPort())
    probe.SetSource(windReader.GetOutput())

    print "Sampling wind velocity field"
    probe.Update()
    print "Finished\n"


    # For some reason we can get negative values.  Fix this.
    threshold = vtkArrayCalculator()
    threshold.SetInputConnection(probe.GetOutputPort())
    threshold.AddScalarArrayName("velocityNormXYMag", 0)
    threshold.SetResultArrayName("velocityNormXYMag")
    threshold.SetFunction("max(0.0, velocityNormXYMag)")


    # Convert from unstructured grid to polygonal data
    surface = vtkDataSetSurfaceFilter()
    surface.SetInputConnection(threshold.GetOutputPort())

    print "Converting to polygonal data"
    surface.Update()
    print "Finished\n"


    # Remove unnecessary data array
    surface.GetOutput().GetPointData().RemoveArray("vtkValidPointMask")


    # Save the translated roofs
    writer = vtkXMLPolyDataWriter()
    writer.SetInputConnection(surface.GetOutputPort())
    writer.SetFileName(output_name)

    print "Saving " + output_name
    writer.Write()
    print "Finished\n"


# Extracts a portion of the velocity field
def extract_subset(vtk_input_name, output_name, center, size, rotation = 0.0):
    """Extracts a portion of the velocity field

    vtk_input_name:  The input VTK file containing the wind velocity field (file extension should be .vtu)
    output_name:  The output VTK file name (file extension should be .vtu)
    center:  The center of the box
    size:  The size of the box
    rotation:  The rotation of the box, in clockwise degrees
    """
    
    # Load the wind velocity data from the VTK file
    windReader = vtkXMLUnstructuredGridReader()
    windReader.SetFileName(vtk_input_name)

    print "Reading " + vtk_input_name
    windReader.Update()
    print "Finished\n"


    # Create a transform for the clipping
    transform = vtkTransform()
    transform.Scale(1.0 / size[0], 1.0 / size[1], 1.0 / size[2])
    transform.RotateZ(rotation);
    transform.Translate(-center[0], -center[1], -center[2]);    


    # Use 6 clipping planes instead of a box, as a box has errors at edges
    plane1 = vtkPlane()
    plane1.SetOrigin(-0.5, 0.0, 0.0)
    plane1.SetNormal(1.0, 0.0, 0.0)
    plane1.SetTransform(transform);

    plane2 = vtkPlane()
    plane2.SetOrigin(0.5, 0.0, 0.0)
    plane2.SetNormal(-1.0, 0.0, 0.0)
    plane2.SetTransform(transform);
    
    plane3 = vtkPlane()
    plane3.SetOrigin(0.0, -0.5, 0.0)
    plane3.SetNormal(0.0, 1.0, 0.0)
    plane3.SetTransform(transform);
    
    plane4 = vtkPlane()
    plane4.SetOrigin(0.0, 0.5, 0.0)
    plane4.SetNormal(0.0, -1.0, 0.0)
    plane4.SetTransform(transform);

    plane5 = vtkPlane()
    plane5.SetOrigin(0.0, 0.0, -0.5)
    plane5.SetNormal(0.0, 0.0, 1.0)
    plane5.SetTransform(transform);

    plane6 = vtkPlane()
    plane6.SetOrigin(0.0, 0.0, 0.5)
    plane6.SetNormal(0.0, 0.0, -1.0)
    plane6.SetTransform(transform);


    # Daisy-chain the clipping filters
    clip1 = vtkClipDataSet()
    clip1.SetInputConnection(windReader.GetOutputPort())
    clip1.SetClipFunction(plane1)

    clip2 = vtkClipDataSet()
    clip2.SetInputConnection(clip1.GetOutputPort())
    clip2.SetClipFunction(plane2)

    clip3 = vtkClipDataSet()
    clip3.SetInputConnection(clip2.GetOutputPort())
    clip3.SetClipFunction(plane3)

    clip4 = vtkClipDataSet()
    clip4.SetInputConnection(clip3.GetOutputPort())
    clip4.SetClipFunction(plane4)

    clip5 = vtkClipDataSet()
    clip5.SetInputConnection(clip4.GetOutputPort())
    clip5.SetClipFunction(plane5)

    clip6 = vtkClipDataSet()
    clip6.SetInputConnection(clip5.GetOutputPort())
    clip6.SetClipFunction(plane6)

    print "Clipping data"
    clip6.Update()
    print "Finished\n"
    

    # Save the clipped data
    writer = vtkXMLUnstructuredGridWriter()
    writer.SetInputConnection(clip6.GetOutputPort())
    writer.SetFileName(output_name)

    print "Saving " + output_name
    writer.Write()
    print "Finished\n"


# Scales the velocity components of an input mesh by the scale factor
def scale_velocity_mesh(vtk_input_name, vtk_output_name, scale_factor):
    """Scales the velocity components of an input mesh by the scale factor

    vtk_input_name:  The input VTK file containing the wind velocity field (file extension should be .vtu)
    vtk_output_name: The output VTK file containing the scaled wind velocity field (file extension should be .vtu)
    scale_factor:  The amount to scale by
    """

    # Load the wind velocity data from the VTK file
    windReader = vtkXMLUnstructuredGridReader()
    windReader.SetFileName(vtk_input_name)

    print "Reading " + vtk_input_name
    windReader.Update()
    print "Finished\n"


    # Scale the xy velocity magnitude
    scaleXYMagnitude = vtkArrayCalculator()
    scaleXYMagnitude.SetInputConnection(windReader.GetOutputPort())
    scaleXYMagnitude.AddScalarArrayName("velocityNormXYMag", 0)
    scaleXYMagnitude.SetResultArrayName("velocityNormXYMag")
    function = "velocityNormXYMag * %f" % scale_factor
    scaleXYMagnitude.SetFunction(function)

    print "Applying scale factor to xy magnitude"
    scaleXYMagnitude.Update()
    print "Finished\n"


    # Scale the z component
    scaleZComponent = vtkArrayCalculator()
    scaleZComponent.SetInputConnection(scaleXYMagnitude.GetOutputPort())
    scaleZComponent.AddScalarArrayName("velocityNormZ", 0)
    scaleZComponent.SetResultArrayName("velocityNormZ")
    function = "velocityNormZ * %f" % scale_factor
    scaleZComponent.SetFunction(function)

    print "Applying scale factor to z component"
    scaleZComponent.Update()
    print "Finished\n"  


    # Save the scaled data
    windWriter = vtkXMLUnstructuredGridWriter()
    windWriter.SetInputConnection(scaleZComponent.GetOutputPort())
    windWriter.SetFileName(vtk_output_name)

    print "Saving " + vtk_output_name
    windWriter.Write()
    print "Finished\n"


# Scales the velocity components of an input roof offset by the scale factor
def scale_velocity_roof_offset(vtk_input_name, vtk_output_name, scale_factor):
    """Scales the velocity components of an input roof offset by the scale factor

    vtk_input_name:  The input VTK file containing the wind velocity field (file extension should be .vtu)
    vtk_output_name: The output VTK file containing the scaled wind velocity field (file extension should be .vtu)
    scale_factor:  The amount to scale by
    """

    # Load the wind velocity data from the VTK file
    windReader = vtkXMLPolyDataReader()
    windReader.SetFileName(vtk_input_name)

    print "Reading " + vtk_input_name
    windReader.Update()
    print "Finished\n"


    # Scale the xy velocity magnitude
    scaleXYMagnitude = vtkArrayCalculator()
    scaleXYMagnitude.SetInputConnection(windReader.GetOutputPort())
    scaleXYMagnitude.AddScalarArrayName("velocityNormXYMag", 0)
    scaleXYMagnitude.SetResultArrayName("velocityNormXYMag")
    function = "velocityNormXYMag * %f" % scale_factor
    scaleXYMagnitude.SetFunction(function)

    print "Applying scale factor to xy magnitude"
    scaleXYMagnitude.Update()
    print "Finished\n"


    # Scale the z component
    scaleZComponent = vtkArrayCalculator()
    scaleZComponent.SetInputConnection(scaleXYMagnitude.GetOutputPort())
    scaleZComponent.AddScalarArrayName("velocityNormZ", 0)
    scaleZComponent.SetResultArrayName("velocityNormZ")
    function = "velocityNormZ * %f" % scale_factor
    scaleZComponent.SetFunction(function)

    print "Applying scale factor to z component"
    scaleZComponent.Update()
    print "Finished\n"  


    # Save the scaled data
    windWriter = vtkXMLPolyDataWriter()
    windWriter.SetInputConnection(scaleZComponent.GetOutputPort())
    windWriter.SetFileName(vtk_output_name)

    print "Saving " + vtk_output_name
    windWriter.Write()
    print "Finished\n"


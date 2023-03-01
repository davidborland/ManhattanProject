PROJECT( NormalizeData )

SET( EXECUTABLE_OUTPUT_PATH "${ManhattanProject_BINARY_DIR}/bin" )  

Set( VTK_LIBS vtkGraphics
              vtkIO )

SET( SRC NormalizeData.cpp )
ADD_EXECUTABLE( NormalizeData MACOSX_BUNDLE ${SRC} )
TARGET_LINK_LIBRARIES( NormalizeData ${VTK_LIBS}  ) 
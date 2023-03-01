CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( ManhattanProject )

SET( EXECUTABLE_OUTPUT_PATH "${ManhattanProject_BINARY_DIR}/bin" )


#######################################
# Include VTK
#######################################

FIND_PACKAGE( VTK REQUIRED )

INCLUDE_DIRECTORIES ( ${VTK_INCLUDE_DIRS} )
LINK_DIRECTORIES ( ${VTK_LIBRARY_DIRS} )

SET( VTK_LIBS vtkCommon
              vtkFiltering
              vtkGraphics
              vtkHybrid
              vtkIO
              vtkRendering )     


#######################################
# Include ManhattanProject code
#######################################

SET( SRC ManhattanProject.cpp )
ADD_EXECUTABLE( ManhattanProject MACOSX_BUNDLE ${SRC} )
TARGET_LINK_LIBRARIES( ManhattanProject ${VTK_LIBS}  ) 


#######################################
# Include NormalizeData
#######################################

ADD_SUBDIRECTORY( NormalizeData )
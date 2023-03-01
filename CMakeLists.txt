CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )


PROJECT( uwv )


#######################################
# Include VTK
#######################################

FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

SET( VTK_LIBS QVTK ) 


#######################################
# Include Qt
#######################################

# Use what QVTK built with
SET( QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "" )
SET( QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "" )
SET( QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "" )
FIND_PACKAGE( Qt )
IF( QT_USE_FILE )
  INCLUDE( ${QT_USE_FILE} )
ELSE( QT_USE_FILE )
  SET( QT_LIBRARIES   ${QT_QT_LIBRARY} )
ENDIF( QT_USE_FILE )

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set up variables for moc
SET( QT_UI MainWindow.ui )
SET( QT_HEADER MainWindow.h )
SET( QT_SRC uwv.cpp MainWindow.cpp )

# Do moc stuff
QT4_WRAP_UI( QT_UI_HEADER ${QT_UI} )
QT4_WRAP_CPP( QT_MOC_SRC ${QT_HEADER} )
ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT )
SET_SOURCE_FILES_PROPERTIES( ${QT_SRC} PROPERTIES OBJECT_DEPENDS "${QT_UI_HEADER}" )


#######################################
# Include uwv code
#######################################

SET( SRC VTKPipeline.h VTKPipeline.cpp 
         vtkRendererCallback.h vtkRendererCallback.cxx )

ADD_EXECUTABLE( uwv ${QT_HEADER} ${QT_SRC} ${QT_MOC_SRC} ${SRC} )
TARGET_LINK_LIBRARIES( uwv ${VTK_LIBS} ${QT_LIBRARIES} )
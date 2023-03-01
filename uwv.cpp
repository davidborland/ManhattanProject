/*=========================================================================

  Name:        uwv.cpp

  Author:      David Borland

  Description: Contains the main function for the uwv (Urban Wind 
               Visualization) program.  Just instantiates a QApplication 
               object and the application main window using Qt.

=========================================================================*/


#include "MainWindow.h"

#include <qapplication.h>


int main(int argc, char** argv) {
    // Initialize Qt
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}


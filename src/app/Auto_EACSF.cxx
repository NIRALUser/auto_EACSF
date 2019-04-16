#include <stdio.h>
#include <stdlib.h>
#include "csfwindow.h"
#include <QApplication>


int main (int argc, char *argv[])
{

  QApplication app( argc , argv );
  Q_INIT_RESOURCE(AutoEACSF_Resources);

  CSFWindow csfw;
  csfw.show();

  return app.exec();
  
}

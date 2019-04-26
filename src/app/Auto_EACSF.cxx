#include <stdio.h>
#include <stdlib.h>
#include "csfwindow.h"
#include "Auto_EACSFCLP.h"
#include <QApplication>


int main (int argc, char *argv[])
{    
    PARSE_ARGS;

    QApplication app( argc , argv );
    Q_INIT_RESOURCE(AutoEACSF_Resources);

    CSFWindow csfw;

    if (!parameters.empty())
    {
        QString qparam = QString::fromStdString(parameters);
        csfw.runNoGUI(qparam);
    }

    else
    {
        csfw.show();
    }

    return app.exec();
  
}

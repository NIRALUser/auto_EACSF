#include <stdio.h>
#include <stdlib.h>
#include "csfwindow.h"
#include "Auto_EACSFCLP.h"
#include <QApplication>
#include <QFile>


int main (int argc, char *argv[])
{    
    PARSE_ARGS;

    QApplication app( argc , argv );
    Q_INIT_RESOURCE(AutoEACSF_Resources);

    CSFWindow csfw;

    if (!Testfile.empty())
    {
        QFile tfile(QString::fromStdString(Testfile));
        tfile.open(QIODevice::ReadOnly);
        QString tcontent = tfile.readAll();
        std::cout<<Testfile<<std::endl;
        std::cout<<tcontent.toStdString()<<std::endl;
        //csfw.setTesttext(tcontent);
        csfw.setTesttext(QString::fromStdString(Testfile));
    }

    if (!parameters.empty())
    {
        QString qparam = QString::fromStdString(parameters);
        QString cli_T1 = QString::fromStdString(T1);
        QString cli_T2 = QString::fromStdString(T2);
        QString cli_BrainMask = QString::fromStdString(BrainMask);
        QString cli_TissueSeg = QString::fromStdString(TissueSeg);
        QString cli_subjectVMask = QString::fromStdString(SubjectVentricleMask);
        QString cli_CerebMask = QString::fromStdString(CerebMask);
        QString cli_output_dir = QString::fromStdString(OutputDir);
        csfw.runNoGUI(qparam, cli_T1, cli_T2, cli_BrainMask, cli_TissueSeg, cli_subjectVMask, cli_CerebMask, cli_output_dir);
    }

    else
    {
        csfw.show();
    }


    return app.exec();
  
}

#include <stdio.h>
#include <stdlib.h>
#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QString>
#include <QFileInfo>
#include <QProcess>
#include <QObject>

#include "csfwindow.h"
#include "Auto_EACSFCLP.h"


#include "csfscripts.h"

QJsonObject readConfig(QString filename)
{
    QString config_qstr;
    QFile config_qfile;
    config_qfile.setFileName(filename);
    config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    config_qstr = config_qfile.readAll();
    config_qfile.close();

    QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8());
    QJsonObject root_obj = config_doc.object();

    return root_obj;

}

QString checkStringValue(string str_value, QJsonValue str_default){
    if(str_value.compare("") == 0){
        if(str_default.isUndefined()){
            return QString("");
        }
        return str_default.toString();
    }else{
        return QString(str_value.c_str());
    }
}


int main (int argc, char *argv[])
{    
    PARSE_ARGS;

    QJsonObject root_obj = readConfig(QString::fromStdString(parameters));
    QJsonObject data_obj = root_obj["data"].toObject();
    data_obj["T1img"] = checkStringValue(T1, data_obj["T1img"]);
    data_obj["T2img"] = checkStringValue(T2, data_obj["T2img"]);
    data_obj["BrainMask"] = checkStringValue(BrainMask, data_obj["BrainMask"]);
    data_obj["CerebellumMask"] = checkStringValue(CerebMask, data_obj["CerebellumMask"]);
    data_obj["SubjectVentricleMask"] = checkStringValue(SubjectVentricleMask, data_obj["SubjectVentricleMask"]);
    data_obj["TissueSeg"] = checkStringValue(TissueSeg, data_obj["TissueSeg"]);
    data_obj["output_dir"] = checkStringValue(OutputDir,  data_obj["output_dir"]);
    root_obj["data"]=data_obj;

    QString output_dir = QDir::cleanPath(data_obj["output_dir"].toString());
    QFileInfo info = QFileInfo(output_dir);
    if(!info.exists()){
        QDir out_dir = QDir();
        out_dir.mkpath(output_dir);
    }

    

    if (noGUI)
    {
        CSFScripts csfscripts;
        csfscripts.setConfig(root_obj);
        csfscripts.run_AutoEACSF();

        QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));

        QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
        
        QStringList params = QStringList() << main_script;

        QProcess* prc =  new QProcess;
        prc->setWorkingDirectory(output_dir);

        QObject::connect(prc, &QProcess::readyReadStandardOutput, [prc](){
            QString output = prc->readAllStandardOutput();
            cout<< output.toStdString() <<endl;
        });

        QObject::connect(prc, &QProcess::readyReadStandardError, [prc](){
            QString err = prc->readAllStandardError();
            cerr << err.toStdString() <<endl;
        });

        QObject::connect(prc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            cout<<"Finished!"<<endl;
        });

        QMap<QString, QString> executables = csfscripts.GetExecutablesMap();

        cout<<executables["python3"].toStdString()<<" "<<params.join(" ").toStdString()<<endl;
        
        prc->start(executables["python3"], params);

        prc->waitForFinished(-1);

        prc->close();

        return EXIT_SUCCESS;
    }

    else
    {
        QApplication app( argc , argv );
        Q_INIT_RESOURCE(AutoEACSF_Resources);

        CSFWindow csfw;
        if(parameters.compare("") != 0){
            csfw.setConfig(root_obj);    
        }
        csfw.show();

        return app.exec();
    }


    
  
}

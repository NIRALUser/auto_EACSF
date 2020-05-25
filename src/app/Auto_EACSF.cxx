#include <stdio.h>
#include <stdlib.h>
#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QString>

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

    if (noGUI)
    {
        CSFScripts csfscripts;
        csfscripts.setConfig(root_obj);
        csfscripts.run_AutoEACSF();

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

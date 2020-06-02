#include "csfwindow.h"

#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QtCore>
#include <QScrollBar>
#include <QRegExp>

#ifndef Auto_EACSF_TITLE
#define Auto_EACSF_TITLE "Auto_EACSF"
#endif

#ifndef Auto_EACSF_CONTRIBUTORS
#define Auto_EACSF_CONTRIBUTORS "Han Bit Yoon, Arthur Le Maout, Martin Styner"
#endif

#ifndef Auto_EACSF_VERSION
#define Auto_EACSF_VERSION "1.7"
#endif

using std::endl;
using std::cout;

const QString CSFWindow::m_github_url = "https://github.com/NIRALUser/auto_EACSF";

CSFWindow::CSFWindow(QWidget *m_parent):
    QMainWindow(m_parent)
{

    // this->m_CSFScripts = CSFScripts();

    m_GUI = true;

    // dataSeemsAligned=false;
    // script_running=false;
    // outlog_file_created=false;


    this->setupUi(this);

    checkBox_VentricleRemoval->setChecked(true);
    label_dataAlignmentMessage->setVisible(false);
    listWidget_SSAtlases->setSelectionMode(QAbstractItemView::NoSelection);
    prc = new QProcess;
    QJsonObject root_obj = readConfig(QString(":/config/default_config.json"));

#if 0
    QString config_qstr;
    QFile config_qfile;
    config_qfile.setFileName("/work/alemaout/sources/Projects/auto_EACSF-Project/test1.json");
    config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    config_qstr = config_qfile.readAll();
    config_qfile.close();

    QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8());
    QJsonObject root_obj = config_doc.object();

    AutomaticInterface *TW = new AutomaticInterface();
    TW->buildTabs(root_obj["tabs"].toArray());
    tabWidget->addTab(TW->widget(0),TW->tabText(0));
#else

    QJsonArray exe_array = root_obj["executables"].toArray();
    QMap<QString, QString> executables;

    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    QBoxLayout* exe_layout = new QBoxLayout(QBoxLayout::LeftToRight, tab_executables);
    m_exeWidget = new ExtExecutablesWidget();

    if (!executables.keys().isEmpty())
    {

        m_exeWidget->buildInterface(executables);
        m_exeWidget->setExeDir(QDir::currentPath());
        exe_layout->addWidget(m_exeWidget,Qt::AlignCenter);
        connect(m_exeWidget, SIGNAL(m_exeWidget->newExePath(QString,QString)), this, SLOT(updateExecutables(QString,QString)));
    }


    tabWidget->removeTab(1);
    tabWidget->removeTab(3);
#endif

    checkBox_CSFDensity->setChecked(false);
}

CSFWindow::~CSFWindow()
{
}

void CSFWindow::setConfig(QJsonObject root_obj)
{
    
    QJsonObject data_obj = root_obj["data"].toObject();
    
    lineEdit_BrainMask->setText(!data_obj["BrainMask"].isUndefined()? data_obj["BrainMask"].toString() : "");
    lineEdit_CerebellumMask->setText(!data_obj["CerebellumMask"].isUndefined()?data_obj["CerebellumMask"].toString() : "");
    lineEdit_T1img->setText(!data_obj["T1img"].isUndefined()?data_obj["T1img"].toString() : "");
    lineEdit_T2img->setText(!data_obj["T2img"].isUndefined()?data_obj["T2img"].toString() : "");
    lineEdit_TissueSeg->setText(!data_obj["TissueSeg"].isUndefined()?data_obj["TissueSeg"].toString() : "");
    lineEdit_VentricleMask->setText(!data_obj["SubjectVentricleMask"].isUndefined()?data_obj["SubjectVentricleMask"].toString() : "");
    lineEdit_OutputDir->setText(!data_obj["output_dir"].isUndefined()?data_obj["output_dir"].toString() : "");

    QJsonObject param_obj = root_obj["parameters"].toObject();

    if(param_obj["ACPC_UNIT"].toString().compare(QString("index")) == 0){
        radioButton_Index->setChecked(true);
        spinBox_Index->setEnabled(true);

        radioButton_mm->setChecked(false);
        doubleSpinBox_mm->setEnabled(false);

        spinBox_Index->setValue(param_obj["ACPC_VAL"].toInt());
    }else{
        radioButton_Index->setChecked(false);
        spinBox_Index->setEnabled(false);
        radioButton_mm->setChecked(true);
        doubleSpinBox_mm->setEnabled(true);

        doubleSpinBox_mm->setValue(param_obj["ACPC_VAL"].toDouble());
    }
    spinBox_T1Weight->setValue(param_obj["ANTS_T1_weight"].toInt());
    doubleSpinBox_GaussianSigma->setValue(param_obj["ANTS_gaussian_sig"].toDouble());
    lineEdit_Iterations->setText(param_obj["ANTS_iterations_val"].toString());
    comboBox_RegType->setCurrentText(param_obj["ANTS_reg_type"].toString());
    comboBox_Metric->setCurrentText(param_obj["ANTS_sim_metric"].toString());
    spinBox_SimilarityParameter->setValue(param_obj["ANTS_sim_param"].toInt());
    doubleSpinBox_TransformationStep->setValue(param_obj["ANTS_transformation_step"].toDouble());


    radioButton_rigidRegistration->setChecked(param_obj["PERFORM_REG"].toBool());
    checkBox_SkullStripping->setChecked(param_obj["PERFORM_SS"].toBool());
    checkBox_TissueSeg->setChecked(param_obj["PERFORM_TSEG"].toBool());
    checkBox_VentricleRemoval->setChecked(param_obj["PERFORM_VR"].toBool());
    checkBox_CSFDensity->setChecked(param_obj["COMPUTE_CSFDENS"].toBool());
    
    lineEdit_templateT1Ventricle->setText(param_obj["templateT1Ventricle"].toString());
    lineEdit_templateInvMaskVentricle->setText(param_obj["templateInvMaskVentricle"].toString());
    lineEdit_ReferenceAtlasFile->setText(param_obj["registrationAtlas"].toString());
    lineEdit_SSAtlasFolder->setText(param_obj["skullStrippingAtlasesDirectory"].toString());
    lineEdit_TissueSegAtlas->setText(param_obj["tissueSegAtlasDirectory"].toString());
    spinBox_CSFLabel->setValue(param_obj["CSFLabel"].toInt());
    lineEdit_LH_inner->setText(param_obj["LH_innerSurface"].toString());
    lineEdit_RH_inner->setText(param_obj["RH_innerSurface"].toString());

    QJsonArray exe_array = root_obj["executables"].toArray();

    displayAtlases(lineEdit_SSAtlasFolder->text(),!lineEdit_isEmpty(lineEdit_T2img));

    m_atlases = param_obj["atlas_obj"].toObject();

    for (int i = 0 ; i < listWidget_SSAtlases->count(); i++){
        QListWidgetItem *item = listWidget_SSAtlases->item(i);

        QString name = item->text().split(":")[0];

        QJsonObject atlas = m_atlases[name].toObject();
    
        if(atlas["selected"].toBool()){
            item->setCheckState(Qt::Checked);
        }else{
            item->setCheckState(Qt::Unchecked);
        }
    } 

    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        emit m_exeWidget->newExePath(exe_obj["name"].toString(), exe_obj["path"].toString());
    }

    // QJsonArray script_array = root_obj["scripts"].toArray();
    // foreach (const QJsonValue script_val, script_array)
    // {
    //     QJsonObject script_obj = script_val.toObject();
    //     foreach (const QJsonValue exe_name, script_obj["executables"].toArray())
    //     {
    //         // script_exe[script_obj["name"].toString()].append(exe_name.toString());
    //         // emit newExePath(exe_obj["name"].toString(), exe_obj["path"].toString());
    //     }
    // }

}

QJsonObject CSFWindow::readConfig(QString filename)
{

    QJsonObject root_obj;

    if(!filename.isEmpty()){
        QString config_qstr;
        QFile config_qfile;
        config_qfile.setFileName(filename);
        config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
        config_qstr = config_qfile.readAll();
        config_qfile.close();

        QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8());
        root_obj = config_doc.object();

    }
    return root_obj;
}

QJsonObject CSFWindow::getConfig(){

    QJsonObject data_obj;
    data_obj["BrainMask"] = lineEdit_BrainMask->text();
    data_obj["T1img"] = lineEdit_T1img->text();
    data_obj["T2img"] = lineEdit_T2img->text();
    data_obj["CerebellumMask"] = lineEdit_CerebellumMask->text();
    data_obj["SubjectVentricleMask"] = lineEdit_VentricleMask->text();
    data_obj["TissueSeg"] = lineEdit_TissueSeg->text();
    data_obj["output_dir"] = lineEdit_OutputDir->text();

    QJsonObject param_obj;
    
    if(radioButton_Index->isChecked()){
        param_obj["ACPC_UNIT"] = QString("index");    
        param_obj["ACPC_VAL"] = spinBox_Index->value();
    }else{
        param_obj["ACPC_UNIT"] = QString("mm");
        param_obj["ACPC_VAL"] = doubleSpinBox_mm->value();
    }
    
    param_obj["ANTS_T1_weight"] = spinBox_T1Weight->value();
    param_obj["ANTS_gaussian_sig"] = doubleSpinBox_GaussianSigma->value();
    param_obj["ANTS_iterations_val"] = lineEdit_Iterations->text();
    param_obj["ANTS_reg_type"] = comboBox_RegType->currentText();
    param_obj["ANTS_sim_metric"] = comboBox_Metric->currentText();
    param_obj["ANTS_sim_param"] = spinBox_SimilarityParameter->value();
    param_obj["ANTS_transformation_step"] = doubleSpinBox_TransformationStep->value();
    param_obj["templateT1Ventricle"] = lineEdit_templateT1Ventricle->text();
    param_obj["templateInvMaskVentricle"] = lineEdit_templateInvMaskVentricle->text();
    param_obj["registrationAtlas"] = lineEdit_ReferenceAtlasFile->text();
    param_obj["skullStrippingAtlasesDirectory"] = lineEdit_SSAtlasFolder->text();
    param_obj["tissueSegAtlasDirectory"] = lineEdit_TissueSegAtlas->text();
    param_obj["CSFLabel"] = spinBox_CSFLabel->value();
    param_obj["LH_innerSurface"] = lineEdit_LH_inner->text();
    param_obj["RH_innerSurface"] = lineEdit_RH_inner->text();
    param_obj["USE_DCM"] = lineEdit_isEmpty(lineEdit_CerebellumMask);
    param_obj["PERFORM_REG"] = radioButton_rigidRegistration->isChecked();
    param_obj["PERFORM_SS"] = checkBox_SkullStripping->isChecked();
    param_obj["PERFORM_TSEG"] = checkBox_TissueSeg->isChecked();
    param_obj["PERFORM_VR"] = checkBox_VentricleRemoval->isChecked();
    param_obj["COMPUTE_CSFDENS"] = checkBox_CSFDensity->isChecked();
    
    param_obj["atlas_obj"] = m_atlases;

    QJsonObject root_obj;
    root_obj["data"] = data_obj;
    root_obj["parameters"] = param_obj;
    root_obj["executables"] = m_exeWidget->GetExeArray();

    return root_obj;
}

bool CSFWindow::writeConfig(QString filename)
{
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        cout<<"Couldn't open save file."<<endl;
        return false;
    }

    QJsonDocument saveDoc(getConfig());

    saveFile.write(saveDoc.toJson());

    cout<<"Saved configuration : "<<filename.toStdString()<<endl;
    return true;
}

QString CSFWindow::OpenFile(){
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "C://",
                "All files (*.*);; NIfTI File (*.nii *.nii.gz);; NRRD File  (*.nrrd);; Json File (*.json)"
                );
    return filename;
}

QString CSFWindow::OpenDir(){
    QString dirname = QFileDialog::getExistingDirectory(
                this,
                tr("Open Directory"),
                "C://",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );
    return dirname;
}

QString CSFWindow::SaveFile()
  {
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Save Document"),
        QDir::currentPath(),
        tr("JSON file (*.json)") );
    return filename;
  }

bool CSFWindow::lineEdit_isEmpty(QLineEdit*& le)
{
    if (le->text().isEmpty()){return true;}
    else
    {
        foreach (const QChar c, le->text())
        {
            if (c != ' ')
            {
                return false;
            }
        }
        return true;
    }
}

bool CSFWindow::checkOptionalMasks()
{
    if (lineEdit_isEmpty(lineEdit_BrainMask)
            && lineEdit_isEmpty(lineEdit_VentricleMask)
            && lineEdit_isEmpty(lineEdit_TissueSeg)
            && lineEdit_isEmpty(lineEdit_CerebellumMask))

    {
        return false;
    }
    return true;
}

void CSFWindow::setBestDataAlignmentOption()
{
    radioButton_rigidRegistration->setChecked(!checkOptionalMasks());
    radioButton_rigidRegistration->setEnabled(!checkOptionalMasks());
    pushButton_ReferenceAtlasFile->setEnabled(!checkOptionalMasks());
    lineEdit_ReferenceAtlasFile->setEnabled(!checkOptionalMasks());
    label_dataAlignmentMessage->setVisible(checkOptionalMasks());
    radioButton_preAligned->setChecked(checkOptionalMasks());
}

int CSFWindow::questionMsgBox(bool checkState, QString maskType, QString action)
{
    QString selection;
    QString verb;
    QString qm;
    if (checkState)
    {
        selection=QString("have");
        verb=QString("Do ");
        qm=QString(" anyway?");
    }
    else
    {
        selection=QString("haven't");
        verb=QString("Don't");
        qm=QString("?");
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowOpacity(1.0);
    msgBox.setText(QString("It seems you "+selection+" selected a "+maskType+" file."));
    msgBox.setInformativeText(QString(verb+" you want to proceed with automatic "+action+qm));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (checkState)
    {
        msgBox.setDefaultButton(QMessageBox::No);
    }
    else
    {
        msgBox.setDefaultButton(QMessageBox::Yes);
    }
    return msgBox.exec();
}

void CSFWindow::infoMsgBox(QString message, QMessageBox::Icon type)
{
    QMessageBox mb;
    mb.setIcon(type);
    mb.setText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}

void CSFWindow::displayAtlases(QString folder_path, bool T2_provided)
{
    listWidget_SSAtlases->clear();
    
    QDir folder(folder_path);
    QStringList itemsList;
    QStringList invalidItems=QStringList();
    
    // m_atlases = QMap<QString,QStringList>();
    m_atlases = QJsonObject();
    
    QStringList splittedName=QStringList();
    QString fileType=QString();
    QStringList fileSplit=QStringList();
    QString fileNameBase=QString();

    foreach (const QString fileName, folder.entryList())
    {
        QFileInfo atlas_file_info = QFileInfo(fileName);

        QString ext = atlas_file_info.completeSuffix();

        QRegularExpression re("nii.gz|nrrd");
        QRegularExpressionMatch match = re.match(ext);

        if (match.hasMatch())
        {

            QString atlasFileName = atlas_file_info.baseName();
            QString atlasName = atlas_file_info.baseName().split("_")[0];

            if(m_atlases[atlasName].isUndefined()){
                m_atlases[atlasName] = QJsonObject();
            }

            QJsonObject atlas = m_atlases[atlasName].toObject();

            if(QRegularExpression("brainmask").match(atlasFileName).hasMatch())
            {
                atlas["brainmask"] = QDir::cleanPath(folder_path + QString("/") + fileName) ;
            }
            else if(QRegularExpression("T1").match(atlasFileName).hasMatch())
            {
                atlas["T1"] = QDir::cleanPath(folder_path + QString("/") + fileName) ;;
            }
            else if(QRegularExpression("T2").match(atlasFileName).hasMatch())
            {
                atlas["T2"] = QDir::cleanPath(folder_path + QString("/") + fileName) ;;
            }

            m_atlases[atlasName] = atlas;
        }

    }

    // QJsonDocument doc(m_atlases);
    // cout<<doc.toJson(QJsonDocument::Compact).toStdString()<<endl;

    foreach (const QString name, m_atlases.keys())
    {
        
        QJsonObject atlas = m_atlases[name].toObject();

        QString itemLabel = name + QString(":");

        QListWidgetItem* item = new QListWidgetItem();

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);

        atlas["selected"] = true;

        if(!atlas["brainmask"].isUndefined() && atlas["brainmask"].toString().compare("") != 0){
            itemLabel += QString(" brainmask ");
        }

        if(!atlas["T1"].isUndefined() && atlas["T1"].toString().compare("") != 0){
            itemLabel += QString(" T1 ");
        }

        if(!atlas["T2"].isUndefined() && atlas["T2"].toString().compare("") != 0){
            itemLabel += QString(" T2 ");
            if(!T2_provided){
                item->setFlags((item->flags() | Qt::ItemIsUserCheckable) & !Qt::ItemIsEnabled);
                item->setCheckState(Qt::Unchecked);
            }
        }
        
        item->setText(itemLabel);

        m_atlases[name] = atlas;
        listWidget_SSAtlases->addItem(item);
    }

    QObject::connect(listWidget_SSAtlases, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selectAtlases(QListWidgetItem*)));
    
    // for (int i = 0 ; i < listWidget_SSAtlases->count(); i++)
    // {
    //     QListWidgetItem *item = listWidget_SSAtlases->item(i);
    //     if (!item->text().endsWith(")"))
    //     {
    //         item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    //         item->setCheckState(Qt::Checked);
    //     }
    //     else
    //     {
    //         item->setFlags((item->flags() | Qt::ItemIsUserCheckable) & !Qt::ItemIsEnabled);
    //         item->setCheckState(Qt::Unchecked);
    //     }

    //     if (i==0)
    //     {
    //         QFont b_font=QFont();
    //         b_font.setBold(true);
    //         item->setFont(b_font);
    //     }
    // }

    // QObject::connect(listWidget_SSAtlases, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selectAtlases(QListWidgetItem*)));
}

// void CSFWindow::write_main_script()
// {
//     QString CSFLabel=QString::number(spinBox_CSFLabel->value());

//     QString LH_inner=lineEdit_LH_inner->text();
//     QString RH_inner=lineEdit_RH_inner->text();

//     QString ACPC_UNIT;
//     QString ACPC_VAL;

//     if(radioButton_Index->isChecked()){
//         ACPC_UNIT = QString("index");    
//         ACPC_VAL = spinBox_Index->value();
//     }else{
//         ACPC_UNIT = QString("mm");
//         ACPC_VAL = doubleSpinBox_mm->value();
//     }

//     bool USE_DCM = lineEdit_isEmpty(lineEdit_CerebellumMask);
//     bool PERFORM_REG = radioButton_rigidRegistration->isChecked();
//     bool PERFORM_SS = checkBox_SkullStripping->isChecked();
//     bool PERFORM_TSEG = checkBox_TissueSeg->isChecked();
//     bool PERFORM_VR = checkBox_VentricleRemoval->isChecked();
//     bool COMPUTE_CSFDENS = checkBox_CSFDensity->isChecked();

//     QFile file(QString(":/PythonScripts/main_script.py"));
//     file.open(QIODevice::ReadOnly);
//     QString script = file.readAll();
//     file.close();

//     script.replace("@T1IMG@", T1img);
//     if (lineEdit_isEmpty(lineEdit_T2img))
//     {
//         script.replace("@T2IMG@", QString(""));
//     }
//     else
//     {
//         script.replace("@T2IMG@", T2img);
//     }
//     script.replace("@BRAIN_MASK@", BrainMask);
//     script.replace("@CEREB_MASK@", CerebMask);
//     script.replace("@TISSUE_SEG@", TissueSeg);
//     script.replace("@CSF_LABEL@",CSFLabel);
//     script.replace("@LH_INNER@", LH_inner);
//     script.replace("@RH_INNER@", RH_inner);

//     script.replace("@ACPC_UNIT@", ACPC_UNIT);
//     script.replace("@ACPC_VAL@", ACPC_VAL);

//     script.replace("@USE_DCM@", QString::number(USE_DCM));
//     script.replace("@PERFORM_REG@", QString::number(PERFORM_REG));
//     script.replace("@PERFORM_SS@", QString::number(PERFORM_SS));
//     script.replace("@PERFORM_TSEG@", QString::number(PERFORM_TSEG));
//     script.replace("@PERFORM_VR@", QString::number(PERFORM_VR));
//     script.replace("@COMPUTE_CSFDENS@", QString::number(COMPUTE_CSFDENS));

//     foreach (const QString exe_name, script_exe[QString("main_script")])
//     {
//         QString str_code = QString("@")+exe_name+QString("_PATH@");
//         QString exe_path = executables[exe_name];
//         script.replace(str_code,exe_path);
//     }

//     script.replace("@OUTPUT_DIR@", output_dir);

//     QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
//     QFile outfile(main_script);
//     outfile.open(QIODevice::WriteOnly);
//     QTextStream outstream(&outfile);
//     outstream << script;
//     outfile.close();
// }

// void CSFWindow::write_rigid_align()
// {
//     QString RefAtlasFile=lineEdit_ReferenceAtlasFile->text();

//     QFile rgd_file(QString(":/PythonScripts/rigid_align.py"));
//     rgd_file.open(QIODevice::ReadOnly);
//     QString rgd_script = rgd_file.readAll();
//     rgd_file.close();

//     rgd_script.replace("@T1IMG@", T1img);
//     if (lineEdit_isEmpty(lineEdit_T2img))
//     {
//         rgd_script.replace(QString("@T2IMG@"), QString(""));
//     }
//     else
//     {
//         rgd_script.replace(QString("@T2IMG@"), T2img);
//     }
//     rgd_script.replace("@ATLAS@", RefAtlasFile);

//     foreach (const QString exe_name, script_exe[QString("rigid_align")])
//     {
//         QString str_code = QString("@")+exe_name+QString("_PATH@");
//         QString exe_path = executables[exe_name];
//         rgd_script.replace(str_code,exe_path);
//         //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
//     }

//     rgd_script.replace("@OUTPUT_DIR@", output_dir);

//     QString rigid_align_script = QDir::cleanPath(scripts_dir + QString("/rigid_align_script.py"));
//     QFile rgd_outfile(rigid_align_script);
//     rgd_outfile.open(QIODevice::WriteOnly);
//     QTextStream rgd_outstream(&rgd_outfile);
//     rgd_outstream << rgd_script;
//     rgd_outfile.close();
// }

// void CSFWindow::write_make_mask()
// {
//     QString SSAtlases_dir=lineEdit_SSAtlasFolder->text();
//     QStringList SSAtlases_list;
//     QString SSAtlases_selected;
//     QListWidgetItem *item=0;
//     for (int i=0; i<listWidget_SSAtlases->count(); i++)
//     {
//         item=listWidget_SSAtlases->item(i);
//         if ((item->flags() != (item->flags() & !Qt::ItemIsEnabled)) && i!=0)
//         {
//             if (item->checkState() == Qt::Checked)
//             {
//                 QStringList itemSplit = item->text().split(' ');
//                 SSAtlases_list.append(itemSplit[0]);
//                 if (itemSplit[4] == QString("T1"))
//                 {
//                     SSAtlases_list.append(QString("1"));
//                 }
//                 else if (itemSplit[4] == QString("T2"))
//                 {
//                     SSAtlases_list.append(QString("2"));
//                 }
//                 else
//                 {
//                     SSAtlases_list.append(QString("3"));
//                 }
//             }
//         }
//     }
//     SSAtlases_selected=SSAtlases_list.join(',');

//     QFile msk_file(QString(":/PythonScripts/make_mask.py"));
//     msk_file.open(QIODevice::ReadOnly);
//     QString msk_script = msk_file.readAll();
//     msk_file.close();

//     msk_script.replace("@T1IMG@", T1img);
//     if (lineEdit_isEmpty(lineEdit_T2img))
//     {
//         msk_script.replace("@T2IMG@", QString(""));
//     }
//     else
//     {
//         msk_script.replace("@T2IMG@", T2img);
//     }
//     msk_script.replace("@ATLASES_DIR@", SSAtlases_dir);
//     msk_script.replace("@ATLASES_LIST@", SSAtlases_selected);

//     foreach (const QString exe_name, script_exe[QString("make_mask")])
//     {
//         QString str_code = QString("@")+exe_name+QString("_PATH@");
//         QString exe_path = executables[exe_name];
//         msk_script.replace(str_code,exe_path);
//         //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
//     }

//     msk_script.replace("@OUTPUT_DIR@", output_dir);

//     QString make_mask_script = QDir::cleanPath(scripts_dir + QString("/make_mask_script.py"));
//     QFile msk_outfile(make_mask_script);
//     msk_outfile.open(QIODevice::WriteOnly);
//     QTextStream msk_outstream(&msk_outfile);
//     msk_outstream << msk_script;
//     msk_outfile.close();
// }

// void CSFWindow::write_tissue_seg()
// {
//     QString TS_Atlas_dir=lineEdit_TissueSegAtlas->text();
//     QFile seg_file(QString(":/PythonScripts/tissue_seg.py"));
//     seg_file.open(QIODevice::ReadOnly);
//     QString seg_script = seg_file.readAll();
//     seg_file.close();

//     seg_script.replace("@T1IMG@", T1img);
//     seg_script.replace("@T2IMG@", T2img);
//     seg_script.replace("@ATLASES_DIR@", TS_Atlas_dir);

//     foreach (const QString exe_name, script_exe[QString("tissue_seg")])
//     {
//         QString str_code = QString("@")+exe_name+QString("_PATH@");
//         QString exe_path = executables[exe_name];
//         seg_script.replace(str_code,exe_path);
//         //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
//     }

//     seg_script.replace("@OUTPUT_DIR@", output_dir);

//     QString tissue_seg_script = QDir::cleanPath(scripts_dir + QString("/tissue_seg_script.py"));
//     QFile seg_outfile(tissue_seg_script);
//     seg_outfile.open(QIODevice::WriteOnly);
//     QTextStream seg_outstream(&seg_outfile);
//     seg_outstream << seg_script;
//     seg_outfile.close();
// }

// void CSFWindow::write_ABCxmlfile(bool T2provided)
// {
//     QFile xmlFile(QDir::cleanPath(output_dir+QString("/ABCparam.xml")));
//     xmlFile.open(QIODevice::WriteOnly);
//     QTextStream xmlStream(&xmlFile);
    
//     //check number of classes in atlas dir
//     // check number of files that match *.mha where * is a number in 
//     QString atlasFolder = lineEdit_TissueSegAtlas->text();
//     QDir atlasfolderEntries(atlasFolder,"?.mha",QDir::Name, QDir::Files);
//     int numExtraClasses = atlasfolderEntries.count() - 4;

//     std::cout << "numClasses " << numExtraClasses << std::endl;

//     xmlStream << "<?xml version=\"1.0\"?>" << '\n' << "<!DOCTYPE SEGMENTATION-PARAMETERS>" << '\n' << "<SEGMENTATION-PARAMETERS>" << '\n';
//     xmlStream << "<SUFFIX>EMS</SUFFIX>" << '\n' << "<ATLAS-DIRECTORY>" << QDir::cleanPath(output_dir+QString("/TissueSegAtlas")) 
//         << "</ATLAS-DIRECTORY>" << '\n' << "<ATLAS-ORIENTATION>file</ATLAS-ORIENTATION>" <<'\n';
//     xmlStream << "<OUTPUT-DIRECTORY>" << QDir::cleanPath(output_dir+QString("/ABC_Segmentation")) << "</OUTPUT-DIRECTORY>" 
//         << '\n' << "<OUTPUT-FORMAT>Nrrd</OUTPUT-FORMAT>" << '\n';
//     xmlStream << "<IMAGE>" << '\n' << "\t<FILE>@T1_INSEG_IMG@</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' 
//         << "</IMAGE>" << '\n';
//     if (T2provided)
//     {
//         xmlStream << "<IMAGE>" << '\n' << "\t<FILE>@T2_INSEG_IMG@</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' 
//         << "</IMAGE>" << '\n';
//     }
//     xmlStream << "<FILTER-ITERATIONS>10</FILTER-ITERATIONS>" << '\n' << "<FILTER-TIME-STEP>0.01</FILTER-TIME-STEP>" << '\n' 
//         << "<FILTER-METHOD>Curvature flow</FILTER-METHOD>" << '\n' << "<MAX-BIAS-DEGREE>4</MAX-BIAS-DEGREE>" << '\n';
    
//     xmlStream << "<PRIOR>1.3</PRIOR>" << '\n' << "<PRIOR>1</PRIOR>" << '\n' << "<PRIOR>0.7</PRIOR>" << '\n';

//     for (int extraclass = 0; extraclass < numExtraClasses; extraclass++ )
//     {
//         xmlStream << "<PRIOR>1</PRIOR>" << '\n';
//     }
//     xmlStream << "<PRIOR>0.8</PRIOR>" << '\n' ;

//     xmlStream << "<INITIAL-DISTRIBUTION-ESTIMATOR>robust</INITIAL-DISTRIBUTION-ESTIMATOR>" << '\n';
//     xmlStream << "<DO-ATLAS-WARP>0</DO-ATLAS-WARP>" << '\n' << "<ATLAS-WARP-FLUID-ITERATIONS>50</ATLAS-WARP-FLUID-ITERATIONS>" << '\n';
//     xmlStream << "<ATLAS-WARP-FLUID-MAX-STEP>0.1</ATLAS-WARP-FLUID-MAX-STEP>" << '\n' << "<ATLAS-LINEAR-MAP-TYPE>id</ATLAS-LINEAR-MAP-TYPE>" << '\n';
//     xmlStream << "<IMAGE-LINEAR-MAP-TYPE>id</IMAGE-LINEAR-MAP-TYPE>" << '\n' << "</SEGMENTATION-PARAMETERS>" << endl;
//     xmlFile.close();
// }

// void CSFWindow::write_vent_mask()
// {
//     QString tenplateT1Ventricle = lineEdit_templateT1Ventricle->text();
//     QString templateT1invMask = lineEdit_templateInvMaskVentricle->text();

//     QFile v_file(QString(":/PythonScripts/vent_mask.py"));
//     v_file.open(QIODevice::ReadOnly);
//     QString v_script = v_file.readAll();
//     v_file.close();

//     v_script.replace("@T1IMG@", T1img);
//     v_script.replace("@TEMPT1VENT@", tenplateT1Ventricle);
//     v_script.replace("@TEMP_INV_VMASK@", templateT1invMask);
//     v_script.replace("@TISSUE_SEG@", TissueSeg);
//     v_script.replace("@SUB_VMASK@" , subjectVentricleMask);
//     v_script.replace("@REG_TYPE@", Registration_Type);
//     v_script.replace("@TRANS_STEP@", Transformation_Step);
//     v_script.replace("@ITERATIONS@", Iterations);
//     v_script.replace("@SIM_METRIC@", Sim_Metric);
//     v_script.replace("@SIM_PARAMETER@", Sim_Parameter);
//     v_script.replace("@GAUSSIAN@", Gaussian);
//     v_script.replace("@T1_WEIGHT@", T1_Weight);

//     foreach (const QString exe_name, script_exe[QString("vent_mask")])
//     {
//         QString str_code = QString("@")+exe_name+QString("_PATH@");
//         QString exe_path = executables[exe_name];
//         v_script.replace(str_code,exe_path);
//         //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
//     }

//     v_script.replace("@OUTPUT_DIR@", output_dir);
//     v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

//     QString vent_mask_script = QDir::cleanPath(scripts_dir + QString("/vent_mask_script.py"));
//     QFile v_outfile(vent_mask_script);
//     v_outfile.open(QIODevice::WriteOnly);
//     QTextStream v_outstream(&v_outfile);
//     v_outstream << v_script;
//     v_outfile.close();
// }


//SLOTS
// Logs
void CSFWindow::disp_output()
{
    QString output(prc->readAllStandardOutput());
    out_log->append(output);
    if (!m_GUI)
    {
        cout<<output.toStdString()<<endl;
    }
}

void CSFWindow::disp_err()
{

    QMap<QString, QString> executables = m_exeWidget->GetExeMap();
    QString errors(prc->readAllStandardError());

    err_log->append(errors);
}


void CSFWindow::prc_finished(int exitCode, QProcess::ExitStatus exitStatus){

    QJsonObject root_obj = getConfig();

    QJsonObject data_obj = root_obj["data"].toObject();
    QString output_dir = data_obj["output_dir"].toString();

    QString exit_message;
    exit_message = QString("Auto_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed"));
    if (exitCode==0)
    {
        exit_message="<font color=\"green\"><b>"+exit_message+"</b></font>";
    }
    else
    {
        exit_message="<font color=\"red\"><b>"+exit_message+"</b></font>";
    }
    out_log->append(exit_message);
    cout<<exit_message.toStdString()<<endl;

    QString outlog_filename = QDir::cleanPath(output_dir + QString("/output_log.txt"));
    QFile outlog_file(outlog_filename);
    outlog_file.open(QIODevice::WriteOnly);
    QTextStream outlog_stream(&outlog_file);
    outlog_stream << out_log->toPlainText();
    outlog_file.close();

    QString errlog_filename = QDir::cleanPath(output_dir + QString("/errors_log.txt"));
    QFile errlog_file(errlog_filename);
    errlog_file.open(QIODevice::WriteOnly);
    QTextStream errlog_stream(&errlog_file);
    errlog_stream << err_log->toPlainText();
    errlog_file.close();

    if (!m_GUI)
    {
        this->close();
    }
}

//File menu
void CSFWindow::on_actionLoad_Configuration_File_triggered()
{
    QString filename= OpenFile();
    if (!filename.isEmpty())
    {
        setConfig(readConfig(filename));
    }
}

void CSFWindow::on_actionSave_Configuration_triggered(){
    QString filename=SaveFile();
    if (!filename.isEmpty())
    {
        if (!filename.endsWith(QString(".json")))
        {
            filename+=QString(".json");
        }
        bool success=writeConfig(filename);
        if (success)
        {
            infoMsgBox(QString("Configuration saved with success : ")+filename,QMessageBox::Information);
        }
        else
        {
            infoMsgBox(QString("Couldn't save configuration file at this location. Try somewhere else."),QMessageBox::Warning);
        }
    }

}

//Window menu
void CSFWindow::on_actionShow_executables_toggled(bool toggled)
{
    if (toggled)
    {
        tabWidget->insertTab(1,tab_executables,QString("Executables"));
        tabWidget->insertTab(4,tab_ANTSregistration,QString("ANTS Parameters"));
    }
    else
    {
        tabWidget->removeTab(1);
        tabWidget->removeTab(3);
    }

}

//About
void CSFWindow::on_actionAbout_triggered()
{
    QString messageBoxTitle = "About " + QString( Auto_EACSF_TITLE );
        QString aboutFADTTS;
        aboutFADTTS = "<b>Version:</b> " + QString( Auto_EACSF_VERSION ) + "<br>"
                "<b>Contributors:</b> " + QString( Auto_EACSF_CONTRIBUTORS ) + "<br>"
                "<b>License:</b> Apache 2.0<br>" +
                "<b>Github:</b> <a href=" + m_github_url + ">Click here</a><br>";
    QMessageBox::information( this, tr( qPrintable( messageBoxTitle ) ), tr( qPrintable( aboutFADTTS ) ), QMessageBox::Ok );
}

// 1st Tab - Inputs
void CSFWindow::on_pushButton_T1img_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_T1img->setText(path);
    }
}

void CSFWindow::on_pushButton_T2img_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_T2img->setText(path);
    }
}

void CSFWindow::on_pushButton_BrainMask_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_BrainMask->setText(path);
    }
}

void CSFWindow::on_lineEdit_BrainMask_textChanged()
{
    setBestDataAlignmentOption();
    checkBox_SkullStripping->setChecked(lineEdit_isEmpty(lineEdit_BrainMask));
}

void CSFWindow::on_pushButton_VentricleMask_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_VentricleMask->setText(path);
    }
}

void CSFWindow::on_pushButton_TissueSeg_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_TissueSeg->setText(path);
    }
}

void CSFWindow::on_lineEdit_TissueSeg_textChanged()
{
    setBestDataAlignmentOption();
    checkBox_TissueSeg->setChecked(lineEdit_isEmpty(lineEdit_TissueSeg));
}

void CSFWindow::on_CerebellumMask_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_CerebellumMask->setText(path);
    }
}

void CSFWindow::on_lineEdit_CerebellumMask_textChanged()
{
    setBestDataAlignmentOption();
}

void CSFWindow::on_pushButton_OutputDir_clicked()
{
    QString path=OpenDir();
    if (!path.isEmpty())
    {
        lineEdit_OutputDir->setText(path);
    }
}

void CSFWindow::on_radioButton_Index_clicked(const bool checkState)
{
    doubleSpinBox_mm->setEnabled(!checkState);
    spinBox_Index->setEnabled(checkState);
}

void CSFWindow::on_radioButton_mm_clicked(const bool checkState)
{
    doubleSpinBox_mm->setEnabled(checkState);
    spinBox_Index->setEnabled(!checkState);
}

// Executables tab
void CSFWindow::updateExecutables(QString exeName, QString path)
{
    emit m_exeWidget->newExePath(exeName, path);
}

// 3rd Tab - 1.Reference Alignment, 2.Skull Stripping

void CSFWindow::on_pushButton_ReferenceAtlasFile_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_ReferenceAtlasFile->setText(path);
    }
}


void CSFWindow::on_checkBox_SkullStripping_clicked(const bool checkState)
{
    if (checkState != lineEdit_isEmpty(lineEdit_BrainMask))
    {
        int ret=questionMsgBox(checkState,QString("brain mask"),QString("skull stripping"));
        if ((checkState && ret==QMessageBox::No) || (!checkState && ret==QMessageBox::Yes))
        {
            checkBox_SkullStripping->setChecked(!checkState);
        }
    }
}

void CSFWindow::on_checkBox_SkullStripping_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    lineEdit_SSAtlasFolder->setEnabled(enab);
    pushButton_SSAtlasFolder->setEnabled(enab);
    listWidget_SSAtlases->setEnabled(enab);
}

void CSFWindow::on_pushButton_SSAtlasFolder_clicked()
{
    QString path=OpenDir();
    if (!path.isEmpty())
    {
    lineEdit_SSAtlasFolder->setText(path);
    }
    displayAtlases(lineEdit_SSAtlasFolder->text(),!lineEdit_isEmpty(lineEdit_T2img));
}

void CSFWindow::selectAtlases(QListWidgetItem *item)
{
    QString atlasName = item->text().split(":")[0];
    QJsonObject atlas = m_atlases[atlasName].toObject();
    
    atlas["selected"] = item->checkState();
    m_atlases[atlasName] = atlas;
}

// 4th Tab - 3.Tissue Seg
void CSFWindow::on_checkBox_TissueSeg_clicked(const bool checkState)
{
    if (checkState != lineEdit_isEmpty(lineEdit_TissueSeg))
    {
        int ret=questionMsgBox(checkState,QString("tissue segmentation"),QString("tissue segmentation"));
        if ((checkState && ret==QMessageBox::No) || (!checkState && ret==QMessageBox::Yes))
        {
            checkBox_TissueSeg->setChecked(!checkState);
        }
    }
}

void CSFWindow::on_checkBox_TissueSeg_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    label_TissueSeg->setEnabled(enab);
    pushButton_TissueSegAtlas->setEnabled(enab);
    lineEdit_TissueSegAtlas->setEnabled(enab);
}


void CSFWindow::on_pushButton_TissueSegAtlas_clicked()
{
    QString path=OpenDir();
    if (!path.isEmpty())
    {
        lineEdit_TissueSegAtlas->setText(path);
    }
}

// 5th Tab - 4.Ventricle Masking
void CSFWindow::on_checkBox_VentricleRemoval_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    label_VentricleRemoval->setEnabled(enab);
    pushButton_templateT1Ventricle->setEnabled(enab);
    lineEdit_templateT1Ventricle->setEnabled(enab);
    pushButton_templateInvMaskVentricle->setEnabled(enab);
    lineEdit_templateInvMaskVentricle->setEnabled(enab);
}

void CSFWindow::on_pushButton_templateT1Ventricle_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_templateT1Ventricle->setText(path);
    }
}

void CSFWindow::on_pushButton_templateInvMaskVentricle_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_templateInvMaskVentricle->setText(path);
    }
}

// 7th Tap - CSF Density
void CSFWindow::on_pushButton_LH_inner_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_inner->setText(path);
    }
}

void CSFWindow::on_pushButton_RH_inner_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_RH_inner->setText(path);
    }
}

void CSFWindow::on_checkBox_CSFDensity_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    lineEdit_LH_inner->setEnabled(enab);
    lineEdit_RH_inner->setEnabled(enab);
    pushButton_LH_inner->setEnabled(enab);
    pushButton_RH_inner->setEnabled(enab);
}

// 8th Tab - Execution
// Execute
void CSFWindow::on_pushButton_execute_clicked()
{
    run_AutoEACSF();
}

void CSFWindow::run_AutoEACSF(){

    QJsonObject root_obj = getConfig();

    CSFScripts csfscripts;
    csfscripts.setConfig(root_obj);
    csfscripts.run_AutoEACSF();

    QJsonObject data_obj = root_obj["data"].toObject();

    QString output_dir = data_obj["output_dir"].toString();
    QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));

    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    QStringList params = QStringList() << main_script;

    connect(prc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(prc_finished(int, QProcess::ExitStatus)));
    connect(prc, SIGNAL(started()), this, SLOT(prc_started()));
    connect(prc, SIGNAL(readyReadStandardOutput()), this, SLOT(disp_output()));
    connect(prc, SIGNAL(readyReadStandardError()), this, SLOT(disp_err()));
    prc->setWorkingDirectory(output_dir);

    QMap<QString, QString> executables = m_exeWidget->GetExeMap();
    
    prc->start(executables[QString("python3")], params);
    
    QMessageBox::information(
        this,
        tr("Auto EACSF"),
        tr("Is running.")
    );

}

void CSFWindow::setTesttext(QString txt)
{
    lineEdit_T1img->setText(txt);
}

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

#define DEFAULT_PARAM 1

using std::endl;
using std::cout;

CSFWindow::CSFWindow(QWidget *parent):
    QMainWindow(parent),exec_names({"ABC","ANTS","BRAINSFit","bet","ImageMath","ImageStat","convertITKformats","WarpImageMultiTransform","python3"})
{
    this->setupUi(this);
    this->initializeMenuBar();
    checkBox_VentricleRemoval->setChecked(true);
    label_dataAlignmentMessage->setVisible(false);
    listWidget_SSAtlases->setSelectionMode(QAbstractItemView::NoSelection);
    prc= new QProcess;

#if DEFAULT_PARAM
    readDataConfiguration_d(QDir::cleanPath(QDir::currentPath()+QString("/data_configuration_martin.json")));
    readDataConfiguration_p(QDir::cleanPath(QDir::currentPath()+QString("/parameter_configuration_SH.json")));
    //readDataConfiguration_sw(QString("/NIRAL/work/alemaout/sources/Projects/auto_EACSF-no-sb/auto_EACSF/data/software_ex.json"));
#endif
    for (QString name : exec_names)
    {
        executables[name]=QString();
    }

    find_executables();
    /*
    for (QString s : executables.keys())
    {
        cout<<s.toStdString()<<" : "<<executables[s].toStdString()<<endl;
    }
    */

    lineEdit_ABC->setText(executables[exec_names[0]]);
    lineEdit_ANTS->setText(executables[exec_names[1]]);
    lineEdit_BRAINSFit->setText(executables[exec_names[2]]);
    lineEdit_FSLBET->setText(executables[exec_names[3]]);
    lineEdit_ImageMath->setText(executables[exec_names[4]]);
    lineEdit_ImageStat->setText(executables[exec_names[5]]);
    lineEdit_convertITKformats->setText(executables[exec_names[6]]);
    lineEdit_WarpImageMultiTransform->setText(executables[exec_names[7]]);
    lineEdit_Python->setText(executables[exec_names[8]]);
}

CSFWindow::~CSFWindow()
{

}

void CSFWindow::disp_output()
{
    QString output(prc->readAllStandardOutput());
    out_log->append(output);
}

void CSFWindow::disp_err()
{
    QString errors(prc->readAllStandardError());
    err_log->append(errors);
}

// File
void CSFWindow::initializeMenuBar(){
    //Load and Save
    connect( action_LoadData, SIGNAL( triggered() ), SLOT( OnLoadDataConfiguration() ) );
    connect( action_SaveData, SIGNAL( triggered() ), SLOT( OnSaveDataConfiguration() ) );

    connect( action_LoadParameter, SIGNAL( triggered() ), SLOT( OnLoadParameterConfiguration() ) );
    connect( action_SaveParameter, SIGNAL( triggered() ), SLOT( OnSaveParameterConfiguration() ) );

    connect( action_LoadSoftware, SIGNAL( triggered() ), SLOT( OnLoadSoftwareConfiguration() ) );
    connect( action_SaveSoftware, SIGNAL( triggered() ), SLOT( OnSaveSoftwareConfiguration() ) );
}

void CSFWindow::check_exe_in_folder(QString name, QString path)
{
    QFileInfo check_exe(path);
    if (check_exe.exists() && check_exe.isExecutable())
    {
        //cout<<name.toStdString()<<" found"<<endl;
        executables[name]=path;
    }
    else
    {
        //cout<<name.toStdString()<<" not found"<<endl;
    }
}

void CSFWindow::find_executables(){
    QString env_PATH(qgetenv("PATH"));
    /*
    cout<<"**************************************"<<endl;
    cout<<"VAR PATH : "<<env_PATH.toStdString()<<endl;
    cout<<"**************************************"<<endl;
    */
    QStringList path_split=env_PATH.split(":");
#ifdef Q_OS_LINUX
    QDir CD=QDir::current();
    CD.cdUp();
    //cout<<CD.absolutePath().toStdString()<<endl;
    //{"ABC", "ANTS", "BRAINSFit", "bet", "ImageMath", "ImageStat", "convertITKformats", "WarpImageMultiTransform", "python3"}

    // Manual checking for exe in superbuild
    check_exe_in_folder(exec_names[0],QDir::cleanPath(CD.absolutePath()+QString("/ABC-build/StandAloneCLI/ABC_CLI")));

    check_exe_in_folder(exec_names[1],QDir::cleanPath(CD.absolutePath()+QString("/ANTs-build/bin/ANTS")));

    check_exe_in_folder(exec_names[2],QDir::cleanPath(CD.absolutePath()+QString("/BRAINSTools-build/bin/BRAINSFit")));

    check_exe_in_folder(exec_names[4],QDir::cleanPath(CD.absolutePath()+QString("/niral_utilities-build/bin/ImageMath")));

    check_exe_in_folder(exec_names[5],QDir::cleanPath(CD.absolutePath()+QString("/niral_utilities-build/bin/ImageStat")));

    check_exe_in_folder(exec_names[6],QDir::cleanPath(CD.absolutePath()+QString("/niral_utilities-build/bin/convertITKformats")));

    check_exe_in_folder(exec_names[7],QDir::cleanPath(CD.absolutePath()+QString("/ANTs-build/bin/WarpImageMultiTransform")));


#endif
#ifdef Q_OS_MACOS
    cout<<"macos"<<endl;
#endif

    //for unfound exe, look in path

    for (QString exe : executables.keys())
    {
        if (executables[exe].isEmpty()) //if exe has not been found
        {
            for (QString p : path_split)
            {
                QString p_exe = QDir::cleanPath(p + QString("/") + exe);
                QFileInfo check_p_exe(p_exe);
                if (check_p_exe.exists() && check_p_exe.isExecutable())
                {
                    executables[exe]=p_exe;
                    break;
                }
                if (exe==QString("ABC"))
                {
                    p_exe=QDir::cleanPath(p + QString("/ABC_CLI"));
                    check_p_exe=QFileInfo(p_exe);
                    if (check_p_exe.exists() && check_p_exe.isExecutable())
                    {
                        executables[exe]=p_exe;
                        break;
                    }
                }
            }
        }
    }

}

void CSFWindow::readDataConfiguration_d(QString filename)
{
    QString settings;
    QFile file;
    QJsonObject dataFile;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    settings = file.readAll();
    file.close();

    QJsonDocument data = QJsonDocument::fromJson(settings.toUtf8());
    dataFile = data.object();

    lineEdit_T1img->setText(dataFile.value(QString("T1img")).toString());
    lineEdit_T2img->setText(dataFile.value(QString("T2img")).toString());
    lineEdit_BrainMask->setText(dataFile.value(QString("BrainMask")).toString());
    lineEdit_VentricleMask->setText(dataFile.value(QString("VentricleMask")).toString());
    lineEdit_TissueSeg->setText(dataFile.value(QString("TissueSeg")).toString());
    lineEdit_CerebellumMask->setText(dataFile.value(QString("CerebellumMask")).toString());
    lineEdit_OutputDir->setText(dataFile.value(QString("output_dir")).toString());
}

void CSFWindow::readDataConfiguration_p(QString filename)
{
    QString settings;
    QFile file;
    QJsonObject paramFile;


    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    settings = file.readAll();
    file.close();

    QJsonDocument data = QJsonDocument::fromJson(settings.toUtf8());
    paramFile = data.object();

    //PARAM
    spinBox_Index->setValue(paramFile.value(QString("ACPC_index")).toInt());
    doubleSpinBox_mm->setValue(paramFile.value(QString("ACPC_mm")).toDouble());

    lineEdit_ReferenceAtlasFile->setText(paramFile.value(QString("Reference_Atlas_dir")).toString());
    lineEdit_TissueSegAtlas->setText(paramFile.value(QString("TissueSeg_Atlas_dir")).toString());

    lineEdit_SSAtlasFolder->setText(paramFile.value(QString("SkullStripping_Atlases_dir")).toString());
    displayAtlases(lineEdit_SSAtlasFolder->text());

    lineEdit_ROIAtlasT1->setText(paramFile.value(QString("ROI_Atlas_T1")).toString());

    comboBox_RegType->setCurrentText(paramFile.value(QString("ANTS_reg_type")).toString());
    doubleSpinBox_TransformationStep->setValue(paramFile.value(QString("ANTS_transformation_step")).toDouble());
    lineEdit_Iterations->setText(paramFile.value(QString("ANTS_iterations_val")).toString());
    comboBox_Metric->setCurrentText(paramFile.value(QString("ANTS_sim_metric")).toString());
    spinBox_SimilarityParameter->setValue(paramFile.value(QString("ANTS_sim_param")).toInt());
    doubleSpinBox_GaussianSigma->setValue(paramFile.value("ANTS_gaussian_sig").toDouble());
    spinBox_T1Weight->setValue(paramFile.value(QString("ANTS_T1_weight")).toInt());
}

void CSFWindow::readDataConfiguration_sw(QString filename)
{
    QString settings;
    QFile file;
    QJsonObject swFile;


    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    settings = file.readAll();
    file.close();

    QJsonDocument data = QJsonDocument::fromJson(settings.toUtf8());
    swFile = data.object();

    //PARAM
    lineEdit_ABC->setText(swFile.value(QString("ABC")).toString());
    lineEdit_ANTS->setText(swFile.value(QString("ANTS")).toString());
    lineEdit_BRAINSFit->setText(swFile.value(QString("BRAINSFit")).toString());
    lineEdit_FSLBET->setText(swFile.value(QString("FSLBET")).toString());
    lineEdit_ImageMath->setText(swFile.value(QString("ImageMath")).toString());
    lineEdit_ImageStat->setText(swFile.value(QString("ImageStat")).toString());
    lineEdit_convertITKformats->setText(swFile.value(QString("convertITKformats")).toString());
    lineEdit_WarpImageMultiTransform->setText(swFile.value(QString("WarpImageMultiTransform")).toString());
    lineEdit_Python->setText(swFile.value(QString("Python")).toString());
}

void CSFWindow::writeDataConfiguration_d(QJsonObject &json)
{
    json["T1img"] = lineEdit_T1img->text();
    json["T2img"] = lineEdit_T2img->text();
    json["BrainMask"] = lineEdit_BrainMask->text();
    json["VentricleMask"] = lineEdit_VentricleMask->text();
    json["TissueSeg"] = lineEdit_TissueSeg->text();
    json["CerebellumMask"] = lineEdit_CerebellumMask->text();
    json["output_dir"] = lineEdit_OutputDir->text();

    cout<<"Save Data Configuration"<<endl;
}

void CSFWindow::writeDataConfiguration_p(QJsonObject &json)
{
    json["ACPC_index"] = spinBox_Index->value();
    json["ACPC_mm"] = doubleSpinBox_mm->value();

    json["Reference_Atlas_dir"] = lineEdit_ReferenceAtlasFile->text();
    json["TissueSeg_Atlas_dir"] = lineEdit_TissueSegAtlas->text();

    json["SkullStripping_Atlases_dir"] = lineEdit_SSAtlasFolder->text();

    json["ROI_Atlas_T1"] = lineEdit_ROIAtlasT1->text();

    json["ANTS_reg_type"] = comboBox_RegType->currentText();
    json["ANTS_transformation_step"] = doubleSpinBox_TransformationStep->value();
    json["ANTS_iterations_val"] = lineEdit_Iterations->text();
    json["ANTS_sim_metric"] = comboBox_Metric->currentText();
    json["ANTS_sim_param"] = spinBox_SimilarityParameter->value();
    json["ANTS_gaussian_sig"] = doubleSpinBox_GaussianSigma->value();
    json["ANTS_T1_weight"] = spinBox_T1Weight->value();

    cout<<"Save Parameter Configuration"<<endl;
}

void CSFWindow::writeDataConfiguration_sw(QJsonObject &json)
{
    json["ABC"] = lineEdit_ABC->text();
    json["ANTS"] = lineEdit_ANTS->text();
    json["BRAINSFit"] = lineEdit_BRAINSFit->text();
    json["FSLBET"] = lineEdit_FSLBET->text();
    json["ImageMath"] = lineEdit_ImageMath->text();
    json["ImageStat"] = lineEdit_ImageStat->text();
    json["ITK"] = lineEdit_convertITKformats->text();
    json["WarpImageMultiTransform"] = lineEdit_WarpImageMultiTransform->text();
    json["Python"] = lineEdit_Python->text();
    cout<<"Save Software Configuration"<<endl;
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

bool CSFWindow::lineEdit_isEmpty(QLineEdit*& le)
{
    if (le->text().isEmpty()){return true;}
    else
    {
        for (QChar c : le->text())
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

void CSFWindow::displayAtlases(QString folder_path)
{
    listWidget_SSAtlases->clear();
    const QString T1=QString("T1");
    const QString T2=QString("T2");
    const QString mask=QString("brainmask");
    QDir folder(folder_path);
    QStringList itemsList=QStringList(QString("Select all"));
    QStringList invalidItems=QStringList();
    QMap<QString,QStringList> atlases=QMap<QString,QStringList>();
    QStringList splittedName=QStringList();
    QString atlasName=QString();
    QString fileType=QString();
    QStringList fileSplit=QStringList();
    QString fileNameBase=QString();

    for (QString fileName : folder.entryList())
    {
        fileSplit=fileName.split('.');
        fileNameBase=fileSplit.at(0);

        if (fileSplit.size()>3 || fileSplit.size()<2)
        {
            invalidItems.append(fileName);
        }
        else
        {
            QString ext=QString();
            QString ext_ref=QString();
            if (fileSplit.size()==3)
            {
                ext=fileSplit.at(1)+fileSplit.at(2);
                ext_ref=QString("niigz");
            }
            else
            {
                ext=fileSplit.at(1);
                ext_ref=QString("nrrd");
            }

            if (ext == ext_ref)
            {
                splittedName=fileNameBase.split('_');
                fileType=splittedName.at(splittedName.size()-1);
                splittedName.pop_back();
                atlasName=QString(splittedName.join('_'));


                if(fileType == mask)
                {
                    atlases.insert(atlasName,{mask});
                }
                else if((fileType == T1) && atlases.contains(atlasName))
                {
                    atlases[atlasName].append(T1);
                }
                else if((fileType == T2) && atlases.contains(atlasName))
                {
                    atlases[atlasName].append(T2);
                }
            }
            else if((fileName != QString(".")) && (fileName != QString("..")))
            {
                invalidItems.append(fileName);
            }
        }


    }

    for (QString atlas : atlases.keys())
    {
        QString itemLabel=QString();
        if (atlases[atlas].size() == 2)
        {
            itemLabel = atlas+QString(" : ")+atlases[atlas].at(0)+QString(" and ")+atlases[atlas].at(1)+QString(" image detected");
        }
        else if(atlases[atlas].size() == 3)
        {
            itemLabel = atlas+QString(" : ")+atlases[atlas].at(0)+QString(", ")+atlases[atlas].at(1)+QString(" and ")+atlases[atlas].at(2)+QString(" images detected");
        }
        itemsList.append(itemLabel);
    }

    listWidget_SSAtlases->addItems(itemsList);


    QListWidgetItem *item=0;
    for (int i = 0 ; i < listWidget_SSAtlases->count(); i++)
    {
        item = listWidget_SSAtlases->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        if (i==0)
        {
            QFont font=QFont();
            font.setBold(true);
            item->setFont(font);
        }
    }
    int count=listWidget_SSAtlases->count();

    listWidget_SSAtlases->addItems(invalidItems);
    for (int i = count ; i < listWidget_SSAtlases->count(); i++)
    {
        item = listWidget_SSAtlases->item(i);
        item->setTextColor(QColor(150,150,150));
    }

    QObject::connect(listWidget_SSAtlases, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selectAtlases(QListWidgetItem*)));
}

void CSFWindow::write_main_script()
{
    QFile file(QString(":/PythonScripts/main_script.py"));
    file.open(QIODevice::ReadOnly);
    QString script = file.readAll();
    file.close();

    script.replace("@T1IMG@", T1img);
    if (lineEdit_isEmpty(lineEdit_T2img))
    {
        script.replace("@T2IMG@", QString(""));
    }
    else
    {
        script.replace("@T2IMG@", T2img);
    }
    script.replace("@VENTRICLE_MASK@", VentricleMask);
    script.replace("@CEREB_MASK@", CerebMask);
    script.replace("@TISSUE_SEG@", TissueSeg);
    if(radioButton_Index->isChecked())
    {
        script.replace("@ACPC_UNIT@", QString("index"));
        script.replace("@ACPC_VAL@", QString::number(spinBox_Index->value()));
    }
    else
    {
        script.replace("@ACPC_UNIT@", QString("mm"));
        script.replace("@ACPC_VAL@", QString::number(doubleSpinBox_mm->value()));
    }

    script.replace("@PERFORM_REG@", QString(radioButton_rigidRegistration->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_SS@", QString(checkBox_SkullStripping->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_TSEG@", QString(checkBox_TissueSeg->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_VR@", QString(checkBox_VentricleRemoval->isChecked() ? "true" : "false"));
    script.replace("@PY3_PATH@", Python);
    script.replace("@IMAGEMATH_PATH@",ImageMath);
    script.replace("@IMAGESTAT_PATH@",ImageStat);
    script.replace("@ABC@",ABC);
    script.replace("@OUTPUT_DIR@", output_dir);

    QDir dir=QDir();
    dir.mkdir(output_dir);
    QString main_script = QDir::cleanPath(output_dir + QString("/main_script.py"));
    QFile outfile(main_script);
    outfile.open(QIODevice::WriteOnly);
    QTextStream outstream(&outfile);
    outstream << script;
    outfile.close();
}

void CSFWindow::write_rigid_align()
{
    QString RefAtlasFile=lineEdit_ReferenceAtlasFile->text();

    QFile rgd_file(QString(":/PythonScripts/rigid_align.py"));
    rgd_file.open(QIODevice::ReadOnly);
    QString rgd_script = rgd_file.readAll();
    rgd_file.close();

    rgd_script.replace("@T1IMG@", T1img);
    if (lineEdit_isEmpty(lineEdit_T2img))
    {
        rgd_script.replace(QString("<IMAGE><FILE>@T2IMG@</FILE><ORIENTATION>file</ORIENTATION></IMAGE>"), QString(""));
    }
    else
    {
        rgd_script.replace(QString("@T2IMG@"), T2img);
    }
    rgd_script.replace("@ATLAS@", RefAtlasFile);
    rgd_script.replace("@BRAINSFIT_PATH@", BRAINSFit);
    rgd_script.replace("@OUTPUT_DIR@", output_dir);

    QString rigid_align_script = QDir::cleanPath(output_dir + QString("/rigid_align_script.py"));
    QFile rgd_outfile(rigid_align_script);
    rgd_outfile.open(QIODevice::WriteOnly);
    QTextStream rgd_outstream(&rgd_outfile);
    rgd_outstream << rgd_script;
    rgd_outfile.close();
}

void CSFWindow::write_make_mask()
{
    QString SSAtlases_dir=lineEdit_SSAtlasFolder->text();
    QStringList SSAtlases_list;
    QString SSAtlases_selected;
    QListWidgetItem *item=0;
    for (int i=0; i<listWidget_SSAtlases->count(); i++)
    {
        item=listWidget_SSAtlases->item(i);
        if (item->textColor() != QColor(150,150,150) && i!=0)
        {
            if (item->checkState() == Qt::Checked)
            {
                QStringList itemSplit = item->text().split(' ');
                SSAtlases_list.append(itemSplit[0]);
                if (itemSplit[4] == QString("T1"))
                {
                    SSAtlases_list.append(QString("1"));
                }
                else if (itemSplit[4] == QString("T2"))
                {
                    SSAtlases_list.append(QString("2"));
                }
                else
                {
                    SSAtlases_list.append(QString("3"));
                }
            }
        }
    }
    SSAtlases_selected=SSAtlases_list.join(',');

    QFile msk_file(QString(":/PythonScripts/make_mask.py"));
    msk_file.open(QIODevice::ReadOnly);
    QString msk_script = msk_file.readAll();
    msk_file.close();

    msk_script.replace("@T1IMG@", T1img);
    if (lineEdit_isEmpty(lineEdit_T2img))
    {
        msk_script.replace("@T2IMG@", QString(""));
    }
    else
    {
        msk_script.replace("@T2IMG@", T2img);
    }
    msk_script.replace("@ATLASES_DIR@", SSAtlases_dir);
    msk_script.replace("@ATLASES_LIST@", SSAtlases_selected);
    msk_script.replace("@IMAGEMATH_PATH@", ImageMath);
    msk_script.replace("@BET_PATH@", FSLBET);
    msk_script.replace("@CITKF_PATH@",convertITKformats);
    msk_script.replace("@ANTS_PATH@",ANTS);
    msk_script.replace("@WIMT_PATH@",WarpImageMultiTransform);
    msk_script.replace("@OUTPUT_DIR@", output_dir);

    QString make_mask_script = QDir::cleanPath(output_dir + QString("/make_mask_script.py"));
    QFile msk_outfile(make_mask_script);
    msk_outfile.open(QIODevice::WriteOnly);
    QTextStream msk_outstream(&msk_outfile);
    msk_outstream << msk_script;
    msk_outfile.close();
}

void CSFWindow::write_ABCxmlfile(bool T2provided)
{
    QString TissueSegAtlas=lineEdit_TissueSegAtlas->text();
    QFile xmlFile(QDir::cleanPath(output_dir+QString("/ABCparam.xml")));
    xmlFile.open(QIODevice::WriteOnly);
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\"?>" << '\n' << "<!DOCTYPE SEGMENTATION-PARAMETERS>" << '\n' << "<SEGMENTATION-PARAMETERS>" << '\n';
    xmlStream << "<SUFFIX>EMS</SUFFIX>" << '\n' << "<ATLAS-DIRECTORY>" << TissueSegAtlas << "</ATLAS-DIRECTORY>" << '\n' << "<ATLAS-ORIENTATION>file</ATLAS-ORIENTATION>" <<'\n';
    xmlStream << "<OUTPUT-DIRECTORY>" << QDir::cleanPath(output_dir+QString("/ABC_Segmentation")) << "</OUTPUT-DIRECTORY>" << '\n' << "<OUTPUT-FORMAT>Nrrd</OUTPUT-FORMAT>" << '\n';
    xmlStream << "<IMAGE>" << '\n' << "\t<FILE>" << T1img << "</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' << "</IMAGE>" << '\n';
    if (T2provided)
    {
        xmlStream << "<IMAGE>" << '\n' << "\t<FILE>" << T2img << "</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' << "</IMAGE>" << '\n';
    }
    xmlStream << "<FILTER-ITERATIONS>10</FILTER-ITERATIONS>" << '\n' << "<FILTER-TIME-STEP>0.01</FILTER-TIME-STEP>" << '\n' << "<FILTER-METHOD>Curvature flow</FILTER-METHOD>" << '\n';
    xmlStream << "<MAX-BIAS-DEGREE>4</MAX-BIAS-DEGREE>" << '\n' << "<PRIOR>1.3</PRIOR>" << '\n' << "<PRIOR>1</PRIOR>" << '\n' << "<PRIOR>0.7</PRIOR>" << '\n';
    xmlStream << "<PRIOR>1</PRIOR>" << '\n' << "<PRIOR>0.8</PRIOR>" << '\n' << "<INITIAL-DISTRIBUTION-ESTIMATOR>robust</INITIAL-DISTRIBUTION-ESTIMATOR>" << '\n';
    xmlStream << "<DO-ATLAS-WARP>0</DO-ATLAS-WARP>" << '\n' << "<ATLAS-WARP-FLUID-ITERATIONS>50</ATLAS-WARP-FLUID-ITERATIONS>" << '\n';
    xmlStream << "<ATLAS-WARP-FLUID-MAX-STEP>0.1</ATLAS-WARP-FLUID-MAX-STEP>" << '\n' << "<ATLAS-LINEAR-MAP-TYPE>id</ATLAS-LINEAR-MAP-TYPE>" << '\n';
    xmlStream << "<IMAGE-LINEAR-MAP-TYPE>id</IMAGE-LINEAR-MAP-TYPE>" << '\n' << "</SEGMENTATION-PARAMETERS>" << endl;
    xmlFile.close();
}

void CSFWindow::write_vent_mask()
{
    QString ROI_AtlasT1=lineEdit_ROIAtlasT1->text();
    QString brainMask=lineEdit_BrainMask->text();

    QDir CD=QDir::current();
    CD.cdUp();
    CD.cdUp();
    QString ventRegMask=QDir::cleanPath(CD.absolutePath()+ QString("/auto_EACSF/data/masks/Vent_CSF-BIN-RAI-Fusion_INV.nrrd"));


    QFile v_file(QString(":/PythonScripts/vent_mask.py"));
    v_file.open(QIODevice::ReadOnly);
    QString v_script = v_file.readAll();
    v_file.close();

    v_script.replace("@T1IMG@", T1img);
    v_script.replace("@ATLAS@", ROI_AtlasT1);
    v_script.replace("@REG_TYPE@", Registration_Type);
    v_script.replace("@TRANS_STEP@", Transformation_Step);
    v_script.replace("@ITERATIONS@", Iterations);
    v_script.replace("@SIM_METRIC@", Sim_Metric);
    v_script.replace("@SIM_PARAMETER@", Sim_Parameter);
    v_script.replace("@GAUSSIAN@", Gaussian);
    v_script.replace("@T1_WEIGHT@", T1_Weight);
    v_script.replace("@TISSUE_SEG@", TissueSeg);
    v_script.replace("@VENTRICLE_MASK@" ,brainMask);
    v_script.replace("@VENT_REG_MASK@",ventRegMask);
    v_script.replace("@IMAGEMATH_PATH@",ImageMath);
    v_script.replace("@ANTS_PATH@", ANTS);
    v_script.replace("@WIMT_PATH@",WarpImageMultiTransform);
    v_script.replace("@OUTPUT_DIR@", output_dir);
    v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

    QString vent_mask_script = QDir::cleanPath(output_dir + QString("/vent_mask_script.py"));
    QFile v_outfile(vent_mask_script);
    v_outfile.open(QIODevice::WriteOnly);
    QTextStream v_outstream(&v_outfile);
    v_outstream << v_script;
    v_outfile.close();
}

//SLOTS

void CSFWindow::OnLoadDataConfiguration(){
    QString filename= OpenFile();
    readDataConfiguration_d(filename);
}

void CSFWindow::OnLoadParameterConfiguration(){
    QString filename= OpenFile();
    readDataConfiguration_p(filename);
}

void CSFWindow::OnLoadSoftwareConfiguration(){
    QString filename= OpenFile();
    readDataConfiguration_sw(filename);
}

bool CSFWindow::OnSaveDataConfiguration(){
    QFile saveFile(QStringLiteral("data_configuration.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject jsonObject;
    writeDataConfiguration_d(jsonObject);
    QJsonDocument saveDoc(jsonObject);
    saveFile.write(saveDoc.toJson());

    return true;
}

bool CSFWindow::OnSaveParameterConfiguration(){
    QFile saveFile(QStringLiteral("parameter_configuration.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }
    QJsonObject jsonObject;
    writeDataConfiguration_p(jsonObject);
    QJsonDocument saveDoc(jsonObject);
    saveFile.write(saveDoc.toJson());

    return true;
}

bool CSFWindow::OnSaveSoftwareConfiguration(){
    QFile saveFile(QStringLiteral("software_configuration.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject jsonObject;
    writeDataConfiguration_sw(jsonObject);
    QJsonDocument saveDoc(jsonObject);
    saveFile.write(saveDoc.toJson());

    return true;
}


// 1st Tab - Inputs
void CSFWindow::on_pushButton_T1img_clicked()
{
    lineEdit_T1img->setText(OpenFile());
}

void CSFWindow::on_pushButton_T2img_clicked()
{
    lineEdit_T2img->setText(OpenFile());
}

void CSFWindow::on_pushButton_BrainMask_clicked()
{
    lineEdit_BrainMask->setText(OpenFile());
}

void CSFWindow::on_lineEdit_BrainMask_textChanged()
{
    setBestDataAlignmentOption();
    checkBox_SkullStripping->setChecked(lineEdit_isEmpty(lineEdit_BrainMask));
}

void CSFWindow::on_pushButton_VentricleMask_clicked()
{
    lineEdit_VentricleMask->setText(OpenFile());
}

void CSFWindow::on_pushButton_TissueSeg_clicked()
{
    lineEdit_TissueSeg->setText(OpenFile());
}

void CSFWindow::on_lineEdit_TissueSeg_textChanged()
{
    setBestDataAlignmentOption();
    checkBox_TissueSeg->setChecked(lineEdit_isEmpty(lineEdit_TissueSeg));
}

void CSFWindow::on_CerebellumMask_clicked()
{
    lineEdit_CerebellumMask->setText(OpenFile());
}

void CSFWindow::on_lineEdit_CerebellumMask_textChanged()
{
    setBestDataAlignmentOption();
}

void CSFWindow::on_pushButton_OutputDir_clicked()
{
     lineEdit_OutputDir->setText(OpenDir());
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

// 2nd Tab - Executables
void CSFWindow::on_pushButton_ABC_clicked()
{
     lineEdit_ABC->setText(OpenFile());
}

void CSFWindow::on_pushButton_ANTS_clicked()
{
     lineEdit_ANTS->setText(OpenFile());
}

void CSFWindow::on_pushButton_BRAINSFit_clicked()
{
     lineEdit_BRAINSFit->setText(OpenFile());
}

void CSFWindow::on_pushButton_FSLBET_clicked()
{
     lineEdit_FSLBET->setText(OpenFile());
}

void CSFWindow::on_pushButton_ImageMath_clicked()
{
     lineEdit_ImageMath->setText(OpenFile());
}

void CSFWindow::on_pushButton_ImageStat_clicked()
{
     lineEdit_ImageStat->setText(OpenFile());
}

void CSFWindow::on_pushButton_convertITKformats_clicked()
{
     lineEdit_convertITKformats->setText(OpenFile());
}

void CSFWindow::on_pushButton_WarpImageMultiTransform_clicked()
{
     lineEdit_WarpImageMultiTransform->setText(OpenFile());
}

void CSFWindow::on_pushButton_Python_clicked()
{
     lineEdit_Python->setText(OpenFile());
}

// 3rd Tab - 1.Reference Alginment, 2.Skull Stripping

void CSFWindow::on_pushButton_ReferenceAtlasFile_clicked()
{
     lineEdit_ReferenceAtlasFile->setText(OpenFile());
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
    lineEdit_SSAtlasFolder->setText(OpenDir());
    if (!lineEdit_isEmpty(lineEdit_SSAtlasFolder))
    {
        displayAtlases(lineEdit_SSAtlasFolder->text());
    }
}

void CSFWindow::selectAtlases(QListWidgetItem *item)
{
    if (item->text() == QString("Select all"))
    {
        QListWidgetItem *it=0;
        for (int i = 0; i < listWidget_SSAtlases->count(); i++)
        {
            it=listWidget_SSAtlases->item(i);
            if (it->textColor() != QColor(150,150,150))
            {
                it->setCheckState(item->checkState());
            }
        }
    }
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
     lineEdit_TissueSegAtlas->setText(OpenDir());
}

// 5th Tab - 4.Ventricle Masking
void CSFWindow::on_checkBox_VentricleRemoval_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    label_VentricleRemoval->setEnabled(enab);
    pushButton_ROIAtlasT1->setEnabled(enab);
    lineEdit_ROIAtlasT1->setEnabled(enab);
}

void CSFWindow::on_pushButton_ROIAtlasT1_clicked()
{
    lineEdit_ROIAtlasT1->setText(OpenFile());
}

// 6th Tab - ANTS Registration
void CSFWindow::on_comboBox_RegType_currentTextChanged(const QString &val)
{
    Registration_Type=val;
}

void CSFWindow::on_doubleSpinBox_TransformationStep_valueChanged(const double val)
{

}

void CSFWindow::on_comboBox_Metric_currentTextChanged(const QString &val)
{
    Sim_Metric=val;
}

void CSFWindow::on_lineEdit_Iterations_textChanged(const QString &val)
{
    Iterations=val;
}

void CSFWindow::on_spinBox_SimilarityParameter_valueChanged(const int val)
{
    Sim_Parameter=val;
}

void CSFWindow::on_doubleSpinBox_GaussianSigma_valueChanged(const double val)
{
    Gaussian=val;
}

void CSFWindow::on_spinBox_T1Weight_valueChanged(const QString &val)
{
    T1_Weight=val;
}

// 7th Tab - Execution
// Execute
void CSFWindow::on_pushButton_execute_clicked()
{
    //0. WRITE MAIN_SCRIPT

    //MAIN_KEY WORDS
    T1img=lineEdit_T1img->text();
    T2img=lineEdit_T2img->text();
    VentricleMask=lineEdit_VentricleMask->text();
    CerebMask=lineEdit_CerebellumMask->text();
    TissueSeg=lineEdit_TissueSeg->text();
    output_dir=lineEdit_OutputDir->text();

    //EXECUTABLES
    ABC=lineEdit_ABC->text();
    ANTS=lineEdit_ANTS->text();
    BRAINSFit=lineEdit_BRAINSFit->text();
    FSLBET=lineEdit_FSLBET->text();
    ImageMath=lineEdit_ImageMath->text();
    ImageStat=lineEdit_ImageStat->text();
    convertITKformats=lineEdit_convertITKformats->text();
    WarpImageMultiTransform=lineEdit_WarpImageMultiTransform->text();
    Python=lineEdit_Python->text();

    write_main_script();

    //1. WRITE RIGID_ALIGN_SCRIPT
    write_rigid_align();

    //2. WRITE MAKE_MASK_SCRIPT
    write_make_mask();

    //3. WRITE Auto_SEG XML
    write_ABCxmlfile(!lineEdit_isEmpty(lineEdit_T2img));

    //4. WRITE VENT_MASK_SCRIPT
    write_vent_mask();

    //Notification
    QMessageBox::information(
        this,
        tr("Auto EACSF"),
        tr("Python scripts are running. It may take up to 24 hours to process.")
    );

    // RUN PYTHON    

    QString main_script = QDir::cleanPath(output_dir + QString("/main_script.py"));
    QStringList params = QStringList() << main_script;

    connect(prc, SIGNAL(finished(int)), this, SLOT(prc_finished()));
    connect(prc, SIGNAL(started()), this, SLOT(prc_started()));
    connect(prc, SIGNAL(readyReadStandardOutput()), this, SLOT(disp_output()));
    connect(prc, SIGNAL(readyReadStandardError()), this, SLOT(disp_err()));
    prc->setWorkingDirectory(output_dir);
    prc->start(Python, params);
}

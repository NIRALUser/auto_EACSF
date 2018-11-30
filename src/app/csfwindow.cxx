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

CSFWindow::CSFWindow(QWidget *m_parent):
    QMainWindow(m_parent)
{
    this->setupUi(this);
    checkBox_VentricleRemoval->setChecked(true);
    label_dataAlignmentMessage->setVisible(false);
    listWidget_SSAtlases->setSelectionMode(QAbstractItemView::NoSelection);
    prc= new QProcess;

#if DEFAULT_PARAM
    readConfig(QString("/NIRAL/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/default_config.json"));
#endif
    if (!executables.keys().isEmpty())
    {
        find_executables();

        for (QString exe_name : executables.keys()) //create the buttons/lineEdit for each executable
        {
            QWidget *containerWidget = new QWidget;
            QLayout *horizontalLayout = new QHBoxLayout();
            verticalLayout_exe->addWidget(containerWidget);
            QPushButton *qpb =  new QPushButton();
            qpb->setText(exe_name);
            qpb->setMinimumWidth(181);
            qpb->setMaximumWidth(181);
            qpb->setMinimumHeight(31);
            qpb->setMaximumHeight(31);
            QObject::connect(qpb,SIGNAL(clicked()),this,SLOT(exe_qpb_triggered()));
            QLineEdit *lined = new QLineEdit();
            lined->setMinimumHeight(31);
            lined->setMaximumHeight(31);
            lined->setText(executables[exe_name]);
            QObject::connect(lined,SIGNAL(textChanged(QString)),this,SLOT(exe_lined_textChanged(QString)));
            horizontalLayout->addWidget(qpb);
            horizontalLayout->addWidget(lined);
            containerWidget->setLayout(horizontalLayout);
        }
    }
}

CSFWindow::~CSFWindow()
{

}

void CSFWindow::check_exe_in_folder(QString name, QString path, QString tree_type)
{
    QFileInfo check_exe(path);
    if (check_exe.exists() && check_exe.isExecutable())
    {
        //cout<<name.toStdString()<<" found in "<<tree_type.toStdString()<<" tree"<<endl;
        executables[name]=path;
    }
    else
    {
        if (tree_type==QString("superbuild"))
        {
            executables[name]=QString("");
        }
        //cout<<name.toStdString()<<" not found in "<<tree_type.toStdString()<<" tree"<<endl;
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
    // If Auto_EACSF has not been installed, look in superbuild tree

    for (QString exe_name : executables.keys())
    {
        QString dirpath = QDir::cleanPath(CD.absolutePath()+executables[exe_name]+exe_name);
        check_exe_in_folder(exe_name,dirpath,QString("superbuild"));
    }

    // If it has been installed, look in install tree
    CD=QDir::current();
    for (QString exe_name : executables.keys())
    {
        QString dirpath = QDir::cleanPath(CD.absolutePath()+QString("/")+exe_name);
        check_exe_in_folder(exe_name,dirpath,QString("install"));
    }




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

void CSFWindow::readConfig(QString filename)
{
    QString config_qstr;
    QFile config_qfile;
    config_qfile.setFileName(filename);
    config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    config_qstr = config_qfile.readAll();
    config_qfile.close();

    QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8());
    QJsonArray config_array = config_doc.array();
    QJsonObject data_obj = config_array[0].toObject();
    lineEdit_BrainMask->setText(data_obj.value(QString("BrainMask")).toString());
    lineEdit_CerebellumMask->setText(data_obj.value(QString("CerebellumMask")).toString());
    lineEdit_T1img->setText(data_obj.value(QString("T1img")).toString());
    lineEdit_T2img->setText(data_obj.value(QString("T2img")).toString());
    lineEdit_TissueSeg->setText(data_obj.value(QString("TissueSeg")).toString());
    lineEdit_VentricleMask->setText(data_obj.value(QString("VentricleMask")).toString());
    lineEdit_OutputDir->setText(data_obj.value(QString("output_dir")).toString());

    QJsonObject param_obj = config_array[1].toObject();
    spinBox_Index->setValue(param_obj.value(QString("ACPC_index")).toInt());
    doubleSpinBox_mm->setValue(param_obj.value(QString("ACPC_mm")).toDouble());
    spinBox_T1Weight->setValue(param_obj.value(QString("ANTS_T1_weight")).toInt());
    doubleSpinBox_GaussianSigma->setValue(param_obj.value(QString("ANTS_gaussian_sig")).toDouble());
    lineEdit_Iterations->setText(param_obj.value(QString("ANTS_iterations_val")).toString());
    comboBox_RegType->setCurrentText(param_obj.value(QString("ANTS_reg_type")).toString());
    comboBox_Metric->setCurrentText(param_obj.value(QString("ANTS_sim_metric")).toString());
    spinBox_SimilarityParameter->setValue(param_obj.value(QString("ANTS_sim_param")).toInt());
    doubleSpinBox_TransformationStep->setValue(param_obj.value(QString("ANTS_transformation_step")).toDouble());
    lineEdit_ROIAtlasT1->setText(param_obj.value(QString("ROI_Atlas_T1")).toString());
    lineEdit_ReferenceAtlasFile->setText(param_obj.value(QString("Reference_Atlas_dir")).toString());
    lineEdit_SSAtlasFolder->setText(param_obj.value(QString("SkullStripping_Atlases_dir")).toString());
    lineEdit_TissueSegAtlas->setText(param_obj.value(QString("TissueSeg_Atlas_dir")).toString());
    spinBox_CSFLabel->setValue(param_obj.value(QString("TissueSeg_csf")).toInt());
    displayAtlases(lineEdit_SSAtlasFolder->text());

    QJsonObject exe_obj = config_array[2].toObject();
    for (QString exe_name : exe_obj.keys())
    {
        executables[exe_name]=exe_obj[exe_name].toString();
    }

    QJsonObject script_obj = config_array[3].toObject();
    for (QString script_name : script_obj.keys())
    {
        QJsonArray sc_array = script_obj[script_name].toArray();
        for (QJsonValue exe_name : sc_array)
        {
            script_exe[script_name].append(exe_name.toString());
        }
    }

//    for (QString script_name : script_exe.keys())
//    {
//        cout<<script_name.toStdString()<<" : [";
//        for (QString exe_name : script_exe[script_name])
//        {
//            cout<<exe_name.toStdString()<<", ";
//        }
//        cout<<"]"<<endl;;
//    }
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
    const QString bmask=QString("brainmask");
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


                if(fileType == bmask)
                {
                    atlases.insert(atlasName,{bmask});
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
            QFont b_font=QFont();
            b_font.setBold(true);
            item->setFont(b_font);
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
    QString CSFLabel=QString::number(spinBox_CSFLabel->value());
    QString CerebMask=lineEdit_CerebellumMask->text();
    QString BrainMask=lineEdit_BrainMask->text();

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
    script.replace("@BRAIN_MASK@", BrainMask);
    script.replace("@CEREB_MASK@", CerebMask);
    script.replace("@TISSUE_SEG@", TissueSeg);
    script.replace("@CSF_LABEL@",CSFLabel);
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

    script.replace("@USE_DCM@", QString(lineEdit_isEmpty(lineEdit_CerebellumMask) ? "true" : "false"));
    script.replace("@PERFORM_REG@", QString(radioButton_rigidRegistration->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_SS@", QString(checkBox_SkullStripping->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_TSEG@", QString(checkBox_TissueSeg->isChecked() ? "true" : "false"));
    script.replace("@PERFORM_VR@", QString(checkBox_VentricleRemoval->isChecked() ? "true" : "false"));

    for (QString exe_name : script_exe[QString("main_script")])
    {
        QString str_code = QString("@")+exe_name+QString("_PATH@");
        QString exe_path = executables[exe_name];
        script.replace(str_code,exe_path);
        cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
    }

    script.replace("@OUTPUT_DIR@", output_dir);

    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
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

    for (QString exe_name : script_exe[QString("rigid_align")])
    {
        QString str_code = QString("@")+exe_name+QString("_PATH@");
        QString exe_path = executables[exe_name];
        rgd_script.replace(str_code,exe_path);
        //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
    }

    rgd_script.replace("@OUTPUT_DIR@", output_dir);

    QString rigid_align_script = QDir::cleanPath(scripts_dir + QString("/rigid_align_script.py"));
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

    for (QString exe_name : script_exe[QString("make_mask")])
    {
        QString str_code = QString("@")+exe_name+QString("_PATH@");
        QString exe_path = executables[exe_name];
        msk_script.replace(str_code,exe_path);
        //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
    }

    msk_script.replace("@OUTPUT_DIR@", output_dir);

    QString make_mask_script = QDir::cleanPath(scripts_dir + QString("/make_mask_script.py"));
    QFile msk_outfile(make_mask_script);
    msk_outfile.open(QIODevice::WriteOnly);
    QTextStream msk_outstream(&msk_outfile);
    msk_outstream << msk_script;
    msk_outfile.close();
}

void CSFWindow::write_tissue_seg()
{
    QString TS_Atlas_dir=lineEdit_TissueSegAtlas->text();
    QFile seg_file(QString(":/PythonScripts/tissue_seg.py"));
    seg_file.open(QIODevice::ReadOnly);
    QString seg_script = seg_file.readAll();
    seg_file.close();

    seg_script.replace("@T1IMG@", T1img);
    seg_script.replace("@T2IMG@", T2img);
    seg_script.replace("@ATLASES_DIR@", TS_Atlas_dir);

    for (QString exe_name : script_exe[QString("tissue_seg")])
    {
        QString str_code = QString("@")+exe_name+QString("_PATH@");
        QString exe_path = executables[exe_name];
        seg_script.replace(str_code,exe_path);
        //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
    }

    seg_script.replace("@OUTPUT_DIR@", output_dir);

    QString tissue_seg_script = QDir::cleanPath(scripts_dir + QString("/tissue_seg_script.py"));
    QFile seg_outfile(tissue_seg_script);
    seg_outfile.open(QIODevice::WriteOnly);
    QTextStream seg_outstream(&seg_outfile);
    seg_outstream << seg_script;
    seg_outfile.close();
}

void CSFWindow::write_ABCxmlfile(bool T2provided)
{
    QFile xmlFile(QDir::cleanPath(output_dir+QString("/ABCparam.xml")));
    xmlFile.open(QIODevice::WriteOnly);
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\"?>" << '\n' << "<!DOCTYPE SEGMENTATION-PARAMETERS>" << '\n' << "<SEGMENTATION-PARAMETERS>" << '\n';
    xmlStream << "<SUFFIX>EMS</SUFFIX>" << '\n' << "<ATLAS-DIRECTORY>" << QDir::cleanPath(output_dir+QString("/TissueSegAtlas")) << "</ATLAS-DIRECTORY>" << '\n' << "<ATLAS-ORIENTATION>file</ATLAS-ORIENTATION>" <<'\n';
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
    QString VentricleMask=lineEdit_VentricleMask->text();

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
    v_script.replace("@VENTRICLE_MASK@" ,VentricleMask);

    for (QString exe_name : script_exe[QString("vent_mask")])
    {
        QString str_code = QString("@")+exe_name+QString("_PATH@");
        QString exe_path = executables[exe_name];
        v_script.replace(str_code,exe_path);
        //cout<<"exe name : "<<exe_name.toStdString()<<" ; "<<str_code.toStdString()<<" ; "<<exe_path.toStdString()<<endl;
    }

    v_script.replace("@OUTPUT_DIR@", output_dir);
    v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

    QString vent_mask_script = QDir::cleanPath(scripts_dir + QString("/vent_mask_script.py"));
    QFile v_outfile(vent_mask_script);
    v_outfile.open(QIODevice::WriteOnly);
    QTextStream v_outstream(&v_outfile);
    v_outstream << v_script;
    v_outfile.close();
}

//SLOTS
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

void CSFWindow::prc_finished(int exitCode, QProcess::ExitStatus exitStatus){
    QString exit_message;
    exit_message = QString("Auto_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed"));
    out_log->append(exit_message);
    cout<<exit_message.toStdString()<<endl;
}

void CSFWindow::on_actionLoad_Configuration_File_triggered()
{
    QString filename= OpenFile();
    if (!filename.isEmpty())
    {
        readConfig(filename);
    }
}

bool CSFWindow::on_actionSave_Configuration_triggered(){
    QFile saveFile(SaveFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonArray saved_array;
    QJsonObject data_obj;
    data_obj["BrainMask"] = lineEdit_BrainMask->text();
    data_obj["T1img"] = lineEdit_T1img->text();
    data_obj["T2img"] = lineEdit_T2img->text();
    data_obj["CerebellumMask"] = lineEdit_CerebellumMask->text();
    data_obj["VentricleMask"] = lineEdit_VentricleMask->text();
    data_obj["TissueSeg"] = lineEdit_TissueSeg->text();
    data_obj["output_dir"] = lineEdit_OutputDir->text();
    cout<<"Save Data Configuration"<<endl;
    saved_array.append(data_obj);

    QJsonObject param_obj;
    param_obj["ACPC_index"] = spinBox_Index->value();
    param_obj["ACPC_mm"] = doubleSpinBox_mm->value();
    param_obj["ANTS_T1_weight"] = spinBox_T1Weight->value();
    param_obj["ANTS_gaussian_sig"] = doubleSpinBox_GaussianSigma->value();
    param_obj["ANTS_iterations_val"] = lineEdit_Iterations->text();
    param_obj["ANTS_reg_type"] = comboBox_RegType->currentText();
    param_obj["ANTS_sim_metric"] = comboBox_Metric->currentText();
    param_obj["ANTS_sim_param"] = spinBox_SimilarityParameter->value();
    param_obj["ANTS_transformation_step"] = doubleSpinBox_TransformationStep->value();
    param_obj["ROI_Atlas_T1"] = lineEdit_ROIAtlasT1->text();
    param_obj["Reference_Atlas_dir"] = lineEdit_ReferenceAtlasFile->text();
    param_obj["SkullStripping_Atlases_dir"] = lineEdit_SSAtlasFolder->text();
    param_obj["TissueSeg_Atlas_dir"] = lineEdit_TissueSegAtlas->text();
    param_obj["TissueSeg_csf"] = spinBox_CSFLabel->value();
    cout<<"Save Parameter Configuration"<<endl;
    saved_array.append(param_obj);

    /*
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
    */

    QJsonDocument saveDoc(saved_array);
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

void CSFWindow::exe_qpb_triggered()
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QLineEdit *le;
    for (QObject *ch : par->children())
    {
        le = qobject_cast<QLineEdit*>(ch);
        if (le)
        {
            break;
        }
    }
    le->setText(OpenFile());
}

void CSFWindow::exe_lined_textChanged(QString new_text)
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QPushButton *bt;
    for (QObject *ch : par->children())
    {
        bt =qobject_cast<QPushButton*>(ch);
        if (bt)
        {
            break;
        }
    }
    executables[bt->text()] = new_text;
}


// 3rd Tab - 1.Reference Alignment, 2.Skull Stripping

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

void CSFWindow::on_doubleSpinBox_TransformationStep_valueChanged()
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
    TissueSeg=lineEdit_TissueSeg->text();
    output_dir=lineEdit_OutputDir->text();
    scripts_dir=QDir::cleanPath(output_dir+QString("/PythonScripts"));

    QDir out_dir=QDir();
    out_dir.mkdir(output_dir);

    QDir sc_dir=QDir();
    sc_dir.mkdir(scripts_dir);

    //0. WRITE MAIN_SCRIPT
    write_main_script();

    //1. WRITE RIGID_ALIGN_SCRIPT
    write_rigid_align();

    //2. WRITE MAKE_MASK_SCRIPT
    write_make_mask();

    //3. WRITE TISSUE_SEG_SCRIPT
    write_tissue_seg();

    //4. WRITE Auto_SEG XML
    write_ABCxmlfile(!lineEdit_isEmpty(lineEdit_T2img));

    //5. WRITE VENT_MASK_SCRIPT
    write_vent_mask();

    //Notification
    QMessageBox::information(
        this,
        tr("Auto EACSF"),
        tr("Python scripts are running. It may take up to 24 hours to process.")
    );

    // RUN PYTHON    

    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    QStringList params = QStringList() << main_script;

    connect(prc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(prc_finished(int, QProcess::ExitStatus)));
    connect(prc, SIGNAL(started()), this, SLOT(prc_started()));
    connect(prc, SIGNAL(readyReadStandardOutput()), this, SLOT(disp_output()));
    connect(prc, SIGNAL(readyReadStandardError()), this, SLOT(disp_err()));
    prc->setWorkingDirectory(output_dir);
    //prc->start(Python, params);
}

#include "csfscripts.h"

CSFScripts::CSFScripts(){
    m_PythonScripts = QString("/PythonScripts");
}

CSFScripts::~CSFScripts(){

}

void CSFScripts::run_AutoEACSF()
{
    //0. WRITE MAIN_SCRIPT
    QJsonObject data_obj = m_Root_obj["data"].toObject();

    QDir out_dir = QDir();

    out_dir.mkdir(QDir::cleanPath(checkStringValue(data_obj["output_dir"].toString())));
    out_dir.mkdir(QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts));

    // Transformation_Step="0.25";
    // Iterations="100x50x25";
    // Sim_Metric="CC";
    // Sim_Parameter="4";
    // Gaussian="3";
    // T1_Weight="1";
    
    QFile saveFile(QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + "/Auto_EACSF_config.json"));
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(m_Root_obj);
    saveFile.write(saveDoc.toJson());
    saveFile.close();


    //0. WRITE MAIN_SCRIPT
    write_main_script();

    //1. WRITE RIGID_ALIGN_SCRIPT
    write_rigid_align();

    //2. WRITE MAKE_MASK_SCRIPT
    write_make_mask();

    //2.1 SKULL STRIP
    write_skull_strip();

    // //3. WRITE TISSUE_SEG_SCRIPT
    write_tissue_seg();

    // //5. WRITE VENT_MASK_SCRIPT
    write_vent_mask();

    // RUN PYTHON

    // QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    // QStringList params = QStringList() << main_script;

    // connect(prc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(prc_finished(int, QProcess::ExitStatus)));
    // connect(prc, SIGNAL(started()), this, SLOT(prc_started()));
    // connect(prc, SIGNAL(readyReadStandardOutput()), this, SLOT(disp_output()));
    // connect(prc, SIGNAL(readyReadStandardError()), this, SLOT(disp_err()));
    // prc->setWorkingDirectory(output_dir);
    // prc->start(executables[QString("python3")], params);

    // //Notification
    // if (m_GUI)
    // {
    //     QMessageBox::information(
    //         this,
    //         tr("Auto EACSF"),
    //         tr("Python scripts are running. It may take up to 24 hours to process.")
    //     );
    // }
}

void CSFScripts::setConfig(QJsonObject config){
    m_Root_obj = config;
}

void CSFScripts::write_main_script()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile file(QString(":/PythonScripts/main_script.py"));
    file.open(QIODevice::ReadOnly);
    QString script = file.readAll();
    file.close();

    script.replace("@T1IMG@", checkStringValue(data_obj["T1img"]));
    script.replace("@T2IMG@", checkStringValue(data_obj["T2img"]));
    script.replace("@BRAIN_MASK@", checkStringValue(data_obj["BrainMask"]));
    script.replace("@CEREB_MASK@", checkStringValue(data_obj["CerebMask"]));
    script.replace("@TISSUE_SEG@", checkStringValue(data_obj["TissueSeg"]));
    script.replace("@CSF_LABEL@",checkIntValue(data_obj["CSFLabel"]));
    script.replace("@LH_INNER@", checkStringValue(data_obj["LH_innerSurface"]));
    script.replace("@RH_INNER@", checkStringValue(data_obj["RH_innerSurface"]));

    script.replace("@ACPC_UNIT@", checkStringValue(param_obj["ACPC_UNIT"]));
    script.replace("@ACPC_VAL@", checkDoubleValue(param_obj["ACPC_VAL"]));

    script.replace("@USE_DCM@", checkBoolValue(param_obj["USE_DCM"]));
    script.replace("@PERFORM_REG@", checkBoolValue(param_obj["PERFORM_REG"]));
    script.replace("@PERFORM_SS@", checkBoolValue(param_obj["PERFORM_SS"]));
    script.replace("@PERFORM_TSEG@", checkBoolValue(param_obj["PERFORM_TSEG"]));
    script.replace("@PERFORM_VR@", checkBoolValue(param_obj["PERFORM_VR"]));
    script.replace("@COMPUTE_CSFDENS@", checkBoolValue(param_obj["COMPUTE_CSFDENS"]));

    QJsonArray exe_array = m_Root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        m_Executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    script.replace("@python3_PATH@", checkStringValue(m_Executables["python3"]));
    script.replace("@ImageMath_PATH@", checkStringValue(m_Executables["ImageMath"]));
    script.replace("@ImageStat_PATH@", checkStringValue(m_Executables["ImageStat"]));
    script.replace("@ABC_CLI_PATH@", checkStringValue(m_Executables["ABC_CLI"]));

    script.replace("@OUTPUT_DIR@", checkStringValue(data_obj["output_dir"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);

    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    QFile outfile(main_script);
    outfile.open(QIODevice::WriteOnly);
    QTextStream outstream(&outfile);
    outstream << script;
    outfile.close();
}

void CSFScripts::write_rigid_align()
{
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();
    QJsonObject data_obj = m_Root_obj["data"].toObject();


    QFile rgd_file(QString(":/PythonScripts/rigid_align.py"));
    rgd_file.open(QIODevice::ReadOnly);
    QString rgd_script = rgd_file.readAll();
    rgd_file.close();

    rgd_script.replace("@T1IMG@", checkStringValue(data_obj["T1img"]));
    rgd_script.replace("@T2IMG@", checkStringValue(data_obj["T2img"]));
    
    rgd_script.replace("@ATLAS@", checkStringValue(param_obj["registrationAtlas"]));
    
    rgd_script.replace("@BRAINSFit_PATH@", checkStringValue(m_Executables["BRAINSFit"]));

    rgd_script.replace("@OUTPUT_DIR@", QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration")));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);


    QString rigid_align_script = QDir::cleanPath(scripts_dir + QString("/rigid_align.py"));
    QFile rgd_outfile(rigid_align_script);
    rgd_outfile.open(QIODevice::WriteOnly);
    QTextStream rgd_outstream(&rgd_outfile);
    rgd_outstream << rgd_script;
    rgd_outfile.close();
}

void CSFScripts::write_make_mask()
{
    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();
    QJsonObject selected_atlas = param_obj["atlas_obj"].toObject();

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);


    QString selected_atlas_filename = QDir::cleanPath(scripts_dir + QString("/selected_atlas.json"));
    cout<<selected_atlas_filename.toStdString()<<endl;
    QFile selected_atlas_outfile(selected_atlas_filename);
    selected_atlas_outfile.open(QIODevice::WriteOnly);
    selected_atlas_outfile.write(QJsonDocument(selected_atlas).toJson());
    selected_atlas_outfile.close();


    QString SSAtlases_dir = param_obj["skullStrippingAtlasesDirectory"].toString();

    QFile msk_file(QString(":/PythonScripts/make_mask.py"));
    msk_file.open(QIODevice::ReadOnly);
    QString msk_script = msk_file.readAll();
    msk_file.close();

    QString T1 = checkStringValue(data_obj["T1img"]);
    QString T2 = checkStringValue(data_obj["T2img"]);

    if(param_obj["PERFORM_REG"].toBool()){
        QFileInfo t1_info(T1);
        if(t1_info.exists()){
            T1 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t1_info.baseName() + QString("_stx.nrrd"));
        }

        QFileInfo t2_info(T2);
        if(t2_info.exists()){
            T2 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t2_info.baseName() + QString("_stx.nrrd"));
        }
    }

    msk_script.replace("@T1IMG@", T1);
    msk_script.replace("@T2IMG@", T2);
    msk_script.replace("@SELECTED_ATLAS@", selected_atlas_filename);

    msk_script.replace("@ImageMath_PATH@", checkStringValue(m_Executables["ImageMath"]));
    msk_script.replace("@bet_PATH@", checkStringValue(m_Executables["bet"]));
    msk_script.replace("@convertITKformats_PATH@", checkStringValue(m_Executables["convertITKformats"]));
    msk_script.replace("@ANTS_PATH@", checkStringValue(m_Executables["ANTS"]));
    msk_script.replace("@WarpImageMultiTransform_PATH@", checkStringValue(m_Executables["WarpImageMultiTransform"]));

    msk_script.replace("@OUTPUT_DIR@", QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/SkullStripping")));

    QString make_mask_script = QDir::cleanPath(scripts_dir + QString("/make_mask.py"));
    QFile msk_outfile(make_mask_script);
    msk_outfile.open(QIODevice::WriteOnly);
    QTextStream msk_outstream(&msk_outfile);
    msk_outstream << msk_script;
    msk_outfile.close();
}

void CSFScripts::write_skull_strip(){
    QFile skull_script_file(QString(":/PythonScripts/skull_strip.py"));
    skull_script_file.open(QIODevice::ReadOnly);
    QString skull_script = skull_script_file.readAll();
    skull_script_file.close();

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QString T1 = checkStringValue(data_obj["T1img"]);
    QString T2 = checkStringValue(data_obj["T2img"]);
    QString BrainMask = checkStringValue(data_obj["BrainMask"]);

    if(param_obj["PERFORM_REG"].toBool()){
        QFileInfo t1_info(T1);
        if(t1_info.exists()){
            T1 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t1_info.baseName() + QString("_stx.nrrd"));
        }

        QFileInfo t2_info(T2);
        if(t2_info.exists()){
            T2 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t2_info.baseName() + QString("_stx.nrrd"));
        }
    }

    if(param_obj["PERFORM_SS"].toBool()){
        QFileInfo t1_info(T1);
        BrainMask = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/SkullStripping/") + t1_info.baseName() + QString("_FinalBrainMask.nrrd"));
    }

    skull_script.replace("@T1IMG@", T1);
    skull_script.replace("@T2IMG@", T2);
    skull_script.replace("@BRAINMASKIMG@", BrainMask);
    skull_script.replace("@OUTPUT_DIR@", QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/SkullStripping")));

    skull_script.replace("@ImageMath_PATH@", checkStringValue(m_Executables["ImageMath"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);


    QString skull_script_filename = QDir::cleanPath(scripts_dir + QString("/skull_strip.py"));
    QFile skull_script_outfile(skull_script_filename);
    skull_script_outfile.open(QIODevice::WriteOnly);
    QTextStream skull_script_outstream(&skull_script_outfile);
    skull_script_outstream << skull_script;
    skull_script_outfile.close();

}

void CSFScripts::write_tissue_seg()
{
    
    QFile seg_file(QString(":/PythonScripts/tissue_seg.py"));
    seg_file.open(QIODevice::ReadOnly);
    QString seg_script = seg_file.readAll();
    seg_file.close();

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QString T1 = checkStringValue(data_obj["T1img"]);
    QString T2 = checkStringValue(data_obj["T2img"]);

    if(param_obj["PERFORM_REG"].toBool()){
        QFileInfo t1_info(T1);
        if(t1_info.exists()){
            T1 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t1_info.baseName() + QString("_stx.nrrd"));
        }

        QFileInfo t2_info(T2);
        if(t2_info.exists()){
            T2 = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/RigidRegistration/") + t2_info.baseName() + QString("_stx.nrrd"));
        }
    }
    
    QFileInfo t1_info(T1);

    if(t1_info.exists()){
        QString T1_stripped = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/SkullStripping/") + t1_info.baseName() + QString("_stripped.nrrd"));
        if(QFileInfo(T1_stripped).exists()){
            T1 = T1_stripped;
        }
    }

    QFileInfo t2_info(T2);

    if(t2_info.exists()){
        QString T2_stripped = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/SkullStripping/") + t2_info.baseName() + QString("_stripped.nrrd"));
        if(QFileInfo(T2_stripped).exists()){
            T2 = T2_stripped;
        }
    }

    seg_script.replace("@T1IMG@", T1);
    seg_script.replace("@T2IMG@", T2);

    seg_script.replace("@ATLASES_DIR@", checkStringValue(param_obj["tissueSegAtlasDirectory"]));
    seg_script.replace("@OUTPUT_DIR@", QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/ABC_Segmentation")));

    seg_script.replace("@BRAINSFit_PATH@", checkStringValue(m_Executables["BRAINSFit"]));
    seg_script.replace("@ANTS_PATH@", checkStringValue(m_Executables["ANTS"]));
    seg_script.replace("@WarpImageMultiTransform_PATH@", checkStringValue(m_Executables["WarpImageMultiTransform"]));
    seg_script.replace("@ABC_CLI_PATH@", checkStringValue(m_Executables["ABC_CLI"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);

    QString tissue_seg_script = QDir::cleanPath(scripts_dir + QString("/tissue_seg.py"));
    QFile seg_outfile(tissue_seg_script);
    seg_outfile.open(QIODevice::WriteOnly);
    QTextStream seg_outstream(&seg_outfile);
    seg_outstream << seg_script;
    seg_outfile.close();
    
}

void CSFScripts::write_vent_mask()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile v_file(QString(":/PythonScripts/vent_mask.py"));
    v_file.open(QIODevice::ReadOnly);
    QString v_script = v_file.readAll();
    v_file.close();


    QString T1 = checkStringValue(data_obj["T1img"]);
    QString output_dir = checkStringValue(data_obj["output_dir"]);

    if(param_obj["PERFORM_REG"].toBool()){
        QFileInfo t1_info(T1);
        if(t1_info.exists()){
            T1 = QDir::cleanPath(output_dir + QString("/RigidRegistration/") + t1_info.baseName() + QString("_stx.nrrd"));
        }
    }

    QFileInfo t1_info(T1);

    if(t1_info.exists()){
        QString T1_stripped = QDir::cleanPath(output_dir + QString("/SkullStripping/") + t1_info.baseName() + QString("_stripped.nrrd"));
        if(QFileInfo(T1_stripped).exists()){
            T1 = T1_stripped;
        }
    }

    QString tissue_seg = checkStringValue(data_obj["TissueSeg"]);
    // findFile("_labels_EMS.", { QDir::cleanPath(output_dir + QString("/ABC_Segmentation")) });

    v_script.replace("@T1IMG@", T1);
    v_script.replace("@SUB_VMASK@" , checkStringValue(data_obj["SubjectVentricleMask"]));

    v_script.replace("@TEMPT1VENT@", checkStringValue(param_obj["templateT1Ventricle"]));
    v_script.replace("@TEMP_INV_VMASK@", checkStringValue(param_obj["templateInvMaskVentricle"]));
    v_script.replace("@TISSUE_SEG@", tissue_seg);
    
    v_script.replace("@REG_TYPE@", checkStringValue(param_obj["ANTS_reg_type"]));
    v_script.replace("@TRANS_STEP@", checkDoubleValue(param_obj["ANTS_transformation_step"]));
    v_script.replace("@ITERATIONS@", checkStringValue(param_obj["ANTS_iterations_val"]));
    v_script.replace("@SIM_METRIC@", checkStringValue(param_obj["ANTS_sim_metric"]));
    v_script.replace("@SIM_PARAMETER@", checkStringValue(param_obj["ANTS_sim_param"]));
    v_script.replace("@GAUSSIAN@", checkDoubleValue(param_obj["ANTS_gaussian_sig"]));
    v_script.replace("@T1_WEIGHT@", checkDoubleValue(param_obj["ANTS_T1_weight"]));

    v_script.replace("@ANTS_PATH@", checkStringValue(m_Executables["ANTS"]));
    v_script.replace("@WarpImageMultiTransform_PATH@", checkStringValue(m_Executables["WarpImageMultiTransform"]));
    v_script.replace("@ImageMath_PATH@", checkStringValue(m_Executables["ImageMath"]));


    v_script.replace("@OUTPUT_DIR@", QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + QString("/VentricleMasking")));
    v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);

    QString vent_mask_script = QDir::cleanPath(scripts_dir + QString("/vent_mask.py"));
    QFile v_outfile(vent_mask_script);
    v_outfile.open(QIODevice::WriteOnly);
    QTextStream v_outstream(&v_outfile);
    v_outstream << v_script;
    v_outfile.close();
}

QString CSFScripts::findFile(QString pattern, QStringList hints){

    QString found("");

    foreach(QString hint, hints){
        QFileInfoList list = QDir(hint).entryInfoList();
        foreach(QFileInfo fileInfo, list){
            QRegularExpression reg(pattern);
            QRegularExpressionMatch match = reg.match(fileInfo.fileName());

            if (match.hasMatch()){
                found = fileInfo.absoluteFilePath();
            }
        }
    }

    return found;
}
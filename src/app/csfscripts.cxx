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

    // //3. WRITE TISSUE_SEG_SCRIPT
    write_tissue_seg();

    // //4. WRITE Auto_SEG XML
    write_ABCxmlfile();

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

    script.replace("@USE_DCM@", checkIntValue(param_obj["USE_DCM"]));
    script.replace("@PERFORM_REG@", checkIntValue(param_obj["PERFORM_REG"]));
    script.replace("@PERFORM_SS@", checkIntValue(param_obj["PERFORM_SS"]));
    script.replace("@PERFORM_TSEG@", checkIntValue(param_obj["PERFORM_TSEG"]));
    script.replace("@PERFORM_VR@", checkIntValue(param_obj["PERFORM_VR"]));
    script.replace("@COMPUTE_CSFDENS@", checkIntValue(param_obj["COMPUTE_CSFDENS"]));

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

    rgd_script.replace("@OUTPUT_DIR@", checkStringValue(data_obj["output_dir"]));

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

    QString SSAtlases_dir = param_obj["skullStrippingAtlasesDirectory"].toString();

    QStringList SSAtlases_list;

    foreach(QJsonValue val, m_Root_obj["atlas_list"].toArray()){
        SSAtlases_list.append(val.toString());
    }
    
    QString SSAtlases_selected = SSAtlases_list.join(',');

    QFile msk_file(QString(":/PythonScripts/make_mask.py"));
    msk_file.open(QIODevice::ReadOnly);
    QString msk_script = msk_file.readAll();
    msk_file.close();

    msk_script.replace("@T1IMG@", checkStringValue(data_obj["T1img"]));
    msk_script.replace("@T2IMG@", checkStringValue(data_obj["T2img"]));
    msk_script.replace("@ATLASES_DIR@", SSAtlases_dir);
    msk_script.replace("@ATLASES_LIST@", SSAtlases_selected);

    msk_script.replace("@ImageMath_PATH@", checkStringValue(m_Executables["ImageMath"]));
    msk_script.replace("@bet_PATH@", checkStringValue(m_Executables["bet"]));
    msk_script.replace("@convertITKformats_PATH@", checkStringValue(m_Executables["convertITKformats"]));
    msk_script.replace("@ANTS_PATH@", checkStringValue(m_Executables["ANTS"]));
    msk_script.replace("@WarpImageMultiTransform_PATH@", checkStringValue(m_Executables["WarpImageMultiTransform"]));

    msk_script.replace("@OUTPUT_DIR@", checkStringValue(data_obj["output_dir"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);

    QString make_mask_script = QDir::cleanPath(scripts_dir + QString("/make_mask.py"));
    QFile msk_outfile(make_mask_script);
    msk_outfile.open(QIODevice::WriteOnly);
    QTextStream msk_outstream(&msk_outfile);
    msk_outstream << msk_script;
    msk_outfile.close();
}

void CSFScripts::write_tissue_seg()
{
    
    QFile seg_file(QString(":/PythonScripts/tissue_seg.py"));
    seg_file.open(QIODevice::ReadOnly);
    QString seg_script = seg_file.readAll();
    seg_file.close();

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    seg_script.replace("@T1IMG@", checkStringValue(data_obj["T1img"]));
    seg_script.replace("@T2IMG@", checkStringValue(data_obj["T2img"]));
    seg_script.replace("@ATLASES_DIR@", checkStringValue(param_obj["tissueSegAtlasDirectory"]));
    seg_script.replace("@OUTPUT_DIR@", checkStringValue(data_obj["output_dir"]));

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

void CSFScripts::write_ABCxmlfile()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QString T1img = checkStringValue(data_obj["T1img"]);
    QString T2img = checkStringValue(data_obj["T2img"]);
    QString output_dir = checkStringValue(data_obj["output_dir"]);

    QFile xmlFile(QDir::cleanPath(output_dir + QString("/ABCparam.xml")));
    xmlFile.open(QIODevice::WriteOnly);
    QTextStream xmlStream(&xmlFile);
    
    //check number of classes in atlas dir
    // check number of files that match *.mha where * is a number in 
    // QString atlasFolder = lineEdit_TissueSegAtlas->text();
    QString atlasFolder = checkStringValue(param_obj["tissueSegAtlasDirectory"]);
    QDir atlasfolderEntries(atlasFolder,"?.mha",QDir::Name, QDir::Files);
    int numExtraClasses = atlasfolderEntries.count() - 4;

    std::cout << "numClasses " << numExtraClasses << std::endl;

    xmlStream << "<?xml version=\"1.0\"?>" << '\n' << "<!DOCTYPE SEGMENTATION-PARAMETERS>" << '\n' << "<SEGMENTATION-PARAMETERS>" << '\n';
    xmlStream << "<SUFFIX>EMS</SUFFIX>" << '\n' << "<ATLAS-DIRECTORY>" << atlasFolder 
        << "</ATLAS-DIRECTORY>" << '\n' << "<ATLAS-ORIENTATION>file</ATLAS-ORIENTATION>" <<'\n';
    xmlStream << "<OUTPUT-DIRECTORY>" << QDir::cleanPath(output_dir+QString("/ABC_Segmentation")) << "</OUTPUT-DIRECTORY>" 
        << '\n' << "<OUTPUT-FORMAT>Nrrd</OUTPUT-FORMAT>" << '\n';
    xmlStream << "<IMAGE>" << '\n' << "\t<FILE>"<<T1img<<"</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' 
        << "</IMAGE>" << '\n';

    if (!T2img.isEmpty())
    {
        xmlStream << "<IMAGE>" << '\n' << "\t<FILE>"<<T2img<<"</FILE>" << '\n' << "\t<ORIENTATION>file</ORIENTATION>" << '\n' 
        << "</IMAGE>" << '\n';
    }

    xmlStream << "<FILTER-ITERATIONS>10</FILTER-ITERATIONS>" << '\n' << "<FILTER-TIME-STEP>0.01</FILTER-TIME-STEP>" << '\n' 
        << "<FILTER-METHOD>Curvature flow</FILTER-METHOD>" << '\n' << "<MAX-BIAS-DEGREE>4</MAX-BIAS-DEGREE>" << '\n';
    
    xmlStream << "<PRIOR>1.3</PRIOR>" << '\n' << "<PRIOR>1</PRIOR>" << '\n' << "<PRIOR>0.7</PRIOR>" << '\n';

    for (int extraclass = 0; extraclass < numExtraClasses; extraclass++ )
    {
        xmlStream << "<PRIOR>1</PRIOR>" << '\n';
    }
    xmlStream << "<PRIOR>0.8</PRIOR>" << '\n' ;

    xmlStream << "<INITIAL-DISTRIBUTION-ESTIMATOR>robust</INITIAL-DISTRIBUTION-ESTIMATOR>" << '\n';
    xmlStream << "<DO-ATLAS-WARP>0</DO-ATLAS-WARP>" << '\n' << "<ATLAS-WARP-FLUID-ITERATIONS>50</ATLAS-WARP-FLUID-ITERATIONS>" << '\n';
    xmlStream << "<ATLAS-WARP-FLUID-MAX-STEP>0.1</ATLAS-WARP-FLUID-MAX-STEP>" << '\n' << "<ATLAS-LINEAR-MAP-TYPE>id</ATLAS-LINEAR-MAP-TYPE>" << '\n';
    xmlStream << "<IMAGE-LINEAR-MAP-TYPE>id</IMAGE-LINEAR-MAP-TYPE>" << '\n' << "</SEGMENTATION-PARAMETERS>" << endl;
    xmlFile.close();
}

void CSFScripts::write_vent_mask()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile v_file(QString(":/PythonScripts/vent_mask.py"));
    v_file.open(QIODevice::ReadOnly);
    QString v_script = v_file.readAll();
    v_file.close();
    

    v_script.replace("@T1IMG@", checkStringValue(data_obj["T1img"]));
    v_script.replace("@SUB_VMASK@" , checkStringValue(data_obj["SubjectVentricleMask"]));

    v_script.replace("@TEMPT1VENT@", checkStringValue(param_obj["templateT1Ventricle"]));
    v_script.replace("@TEMP_INV_VMASK@", checkStringValue(param_obj["templateInvMaskVentricle"]));
    v_script.replace("@TISSUE_SEG@", checkStringValue(param_obj["tissueSegAtlasDirectory"]));
    
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

    v_script.replace("@OUTPUT_DIR@", checkStringValue(data_obj["output_dir"]));
    v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["output_dir"]) + m_PythonScripts);

    QString vent_mask_script = QDir::cleanPath(scripts_dir + QString("/vent_mask_script.py"));
    QFile v_outfile(vent_mask_script);
    v_outfile.open(QIODevice::WriteOnly);
    QTextStream v_outstream(&v_outfile);
    v_outstream << v_script;
    v_outfile.close();
}
#include "csfwindow.h"
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QtCore>

#define BUFFER_SIZE 5000
#define MAX_TOKEN_COUNT 128

using namespace std;
//Inputs
QString T1img;
QString T2img;
QString VentricleMask;
QString CerebMask;
QString TissueSeg;
QString output_dir;

//Executables
QString ABC;
QString ANTS;
QString BRAINSFit;
QString FSLBET;
QString ImageMath;
QString ITK;
QString N4;
QString Python;

//ANTS Registration_Default
QString Registration_Type;
QString Transformation_Step="0.25";
QString Iterations="100x50x25";
QString Sim_Metric="CC";
QString Sim_Parameter="4";
QString Gaussian="3";
QString T1_Weight="1";


CSFWindow::CSFWindow(QWidget *parent):QMainWindow(parent){
    this->setupUi(this);
    this->initializeMenuBar();
}

CSFWindow::~CSFWindow()
{

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

QString CSFWindow::OpenFile(){
    QString filename = QFileDialog::getOpenFileName(
        this,
                tr("Open File"),
                "C://",
                "All files (*.*);;Text File (*.txt);; Json File (*.json)"
    );
    return filename;
}

void CSFWindow::OnLoadDataConfiguration(){
    QString settings;
    QFile file;
    QJsonObject dataFile;
    QString filename= OpenFile();

    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    settings = file.readAll();
    file.close();

    QJsonDocument data = QJsonDocument::fromJson(settings.toUtf8());
    dataFile = data.object();

    lineEdit_T1img->setText(dataFile.value(QString("T1img")).toString());
    lineEdit_T2img->setText(dataFile.value(QString("T2img")).toString());
    lineEdit_BrainMask->setText(dataFile.value(QString("BrainMask")).toString());
    lineEdit_VentriclMask->setText(dataFile.value(QString("VentricleMask")).toString());
    lineEdit_TissueSeg->setText(dataFile.value(QString("TissueSeg")).toString());
    lineEdit_CerebellumMask->setText(dataFile.value(QString("CerebellumMask")).toString());
    lineEdit_OutputDir->setText(dataFile.value(QString("output_dir")).toString());
}

void CSFWindow::OnLoadParameterConfiguration(){
    QString settings;
    QFile file;
    QJsonObject paramFile;
    QString filename= OpenFile();

    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    settings = file.readAll();
    file.close();

    QJsonDocument data = QJsonDocument::fromJson(settings.toUtf8());
    paramFile = data.object();

    //PARAM
    spinBox_Index->setValue(paramFile.value(QString("ACPC_index")).toInt());
    doubleSpinBox_mm->setValue(paramFile.value(QString("ACPC_mm")).toDouble());
    lineEdit_ReferenceAtlasDir->setText(paramFile.value(QString("Reference_Atlas_dir")).toString());
    spinBox_CSFLabel->setValue(paramFile.value(QString("TissueSeg_csf")).toInt());
    lineEdit_TissueSegAtlas->setText(paramFile.value(QString("TissueSeg_Atlas_dir")).toString());

    lineEdit_ROIAtalsT1->setText(paramFile.value(QString("ROI_Atlas_T1")).toString());
    lineEdit_ROIAtalsLabel->setText(paramFile.value(QString("ROI_Atlas_label")).toString());
    spinBox_LeftVentricleLabel->setValue(paramFile.value(QString("left_vLabel")).toInt());
    spinBox_RightVentricleLabel->setValue(paramFile.value(QString("right_vLabel")).toInt());

    comboBox_RegType->setCurrentText(paramFile.value(QString("ANTS_reg_type")).toString());
    doubleSpinBox_TransformationStep->setValue(paramFile.value(QString("ANTS_transformation_step")).toDouble());
    lineEdit_Iterations->setText(paramFile.value(QString("ANTS_iterations_val")).toString());
    comboBox_Metric->setCurrentText(paramFile.value(QString("ANTS_sim_metric")).toString());
    spinBox_SimilarityParameter->setValue(paramFile.value(QString("ANTS_sim_param")).toInt());
    doubleSpinBox_GaussianSigma->setValue(paramFile.value("ANTS_gaussian_sig").toDouble());
    spinBox_T1Weight->setValue(paramFile.value(QString("ANTS_T1_weight")).toInt());
}

void CSFWindow::OnLoadSoftwareConfiguration(){
    QString settings;
    QFile file;
    QJsonObject swFile;
    QString filename= OpenFile();

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
    lineEdit_ITK->setText(swFile.value(QString("ITK")).toString());
    lineEdit_N4->setText(swFile.value(QString("N4")).toString());
    lineEdit_Python->setText(swFile.value(QString("Python")).toString());

    QString ABC=lineEdit_ABC->text();
    QString ANTS=lineEdit_ANTS->text();
    QString BRAINSFit=lineEdit_BRAINSFit->text();
    QString FSLBET=lineEdit_FSLBET->text();
    QString ImageMath=lineEdit_ImageMath->text();
    QString ITK=lineEdit_ITK->text();
    QString N4=lineEdit_N4->text();
    QString Python=lineEdit_Python->text();
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

void CSFWindow::writeDataConfiguration_d(QJsonObject &json)
{
    json["T1img"] = lineEdit_T1img->text();
    json["T2img"] = lineEdit_T2img->text();
    json["BrainMask"] = lineEdit_BrainMask->text();
    json["VentricleMask"] = lineEdit_VentriclMask->text();
    json["TissueSeg"] = lineEdit_TissueSeg->text();
    json["CerebellumMask"] = lineEdit_CerebellumMask->text();
    json["output_dir"] = lineEdit_OutputDir->text();

    cout<<"Save Data Configuration"<<endl;
}

void CSFWindow::writeDataConfiguration_p(QJsonObject &json)
{
    json["ACPC_index"] = spinBox_Index->value();
    json["ACPC_mm"] = doubleSpinBox_mm->value();

    json["Reference_Atlas_dir"] = lineEdit_ReferenceAtlasDir->text();
    json["TissueSeg_csf"] = spinBox_CSFLabel->value();
    json["TissueSeg_Atlas_dir"] = lineEdit_TissueSegAtlas->text();
    json["left_vLabel"] = spinBox_LeftVentricleLabel->value();
    json["right_vLabel"] = spinBox_RightVentricleLabel->value();

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
    json["ITK"] = lineEdit_ITK->text();
    json["N4"] = lineEdit_N4->text();
    json["Python"] = lineEdit_Python->text();
    cout<<"Save Software Configuration"<<endl;
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

void CSFWindow::on_pushButton_VentricleMask_clicked()
{
     lineEdit_VentriclMask->setText(OpenFile());
}

void CSFWindow::on_pushButton_TissueSeg_clicked()
{
    lineEdit_TissueSeg->setText(OpenFile());
}

void CSFWindow::on_CerebellumMask_clicked()
{
    lineEdit_CerebellumMask->setText(OpenFile());
}

void CSFWindow::on_pushButton_OutputDir_clicked()
{
     lineEdit_OutputDir->setText(OpenFile());
}

void CSFWindow::on_spinBox_Index_valueChanged(int arg1)
{
    indexVal=arg1;
}

void CSFWindow::on_doubleSpinBox_mm_valueChanged(double arg1)
{
    mm=arg1;
}

void CSFWindow::on_radioButton_Index_clicked(bool checked)
{
    if(!checked) indexVal=0;
}

void CSFWindow::on_radioButton_mm_clicked(bool checked)
{
    if(!checked) mm=0;
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

void CSFWindow::on_pushButton_ITK_clicked()
{
     lineEdit_ITK->setText(OpenFile());
}

void CSFWindow::on_pushButton_N4_clicked()
{
     lineEdit_N4->setText(OpenFile());
}

void CSFWindow::on_pushButton_Python_clicked()
{
     lineEdit_Python->setText(OpenFile());
}

// 3rd Tab - 1.Reference Alginment, 2.Skull Stripping
void CSFWindow::on_pushButton_ReferenceAtlasDir_clicked()
{
     lineEdit_ReferenceAtlasDir->setText(OpenFile());
}

void CSFWindow::on_radioButton_rigidRegistration_clicked(bool checked)
{
        preAlign=!preAlign;
        notPreAlign=!notPreAlign;
}

void CSFWindow::on_radioButton_preAligned_clicked(bool checked)
{
    preAlign=!preAlign;
    notPreAlign=!notPreAlign;
}

// 4th Tab - 3.Tissue Seg
void CSFWindow::on_pushButton_TissueSegAtlas_clicked()
{
     lineEdit_TissueSegAtlas->setText(OpenFile());
}

// 5th Tab - 4.Ventricle Masking


// 6th Tab - 5.ANTS Registration
void CSFWindow::on_comboBox_RegType_currentTextChanged(const QString &registration_type)
{
    Registration_Type=registration_type;
}

void CSFWindow::on_comboBox_Metric_currentTextChanged(const QString &sim_metric)
{
    Sim_Metric=sim_metric;
}

void CSFWindow::on_lineEdit_Iterations_textChanged(const QString &iterations)
{
    Iterations=iterations;
}

void CSFWindow::on_spinBox_SimilarityParameter_valueChanged(const QString &sim_parameter)
{
    Sim_Parameter=sim_parameter;
}

void CSFWindow::on_doubleSpinBox_GaussianSigma_valueChanged(const QString &gaussian)
{
    Gaussian=gaussian;
}

void CSFWindow::on_spinBox_T1Weight_valueChanged(const QString &t1_weight)
{
    T1_Weight=t1_weight;
}

// 7th Tab - 6.Execution
void CSFWindow::on_checkBox_SkullStripping_clicked(bool checked)
{
}

void CSFWindow::on_checkBox_SkullStripping_stateChanged(int arg1)
{
}

void CSFWindow::readyReadStandardOutput()
{
}

// Execute
void CSFWindow::on_pushButton_execute_clicked()
{
    //0. WRITE MAIN_SCRIPT
    QFile file(QString(":/PythonScripts/main_script.py"));
    file.open(QIODevice::ReadOnly);
    QString script = file.readAll();
    file.close();

    //MAIN_KEY WORDS
    T1img=lineEdit_T1img->text();
    VentricleMask=lineEdit_VentriclMask->text();
    CerebMask=lineEdit_CerebellumMask->text();
    TissueSeg=lineEdit_TissueSeg->text();
    output_dir=lineEdit_OutputDir->text();

    script.replace("@T1IMG@", T1img);
    script.replace("@VENTRICLE_MASK@", VentricleMask);
    script.replace("@CEREB_MASK@", CerebMask);
    script.replace("@TISSUE_SEG@", TissueSeg);
    script.replace("@OUTPUT_DIR@", output_dir);

    QDir().mkdir(output_dir);
    QString main_script = QDir::cleanPath(output_dir + QString("/main_script.py"));
    QFile outfile(main_script);
    outfile.open(QIODevice::WriteOnly);
    QTextStream outstream(&outfile);
    outstream << script;
    outfile.close();

    //1. WRITE RIGID_ALIGN_SCRIPT
    QFile rgd_file(QString(":/PythonScripts/rigid_align.py"));
    rgd_file.open(QIODevice::ReadOnly);
    QString rgd_script = rgd_file.readAll();
    rgd_file.close();

    rgd_script.replace("@T1IMG@", "./noscale_718312_V24_t1w_RAI_Bias.nrrd");
    rgd_script.replace("@T2IMG@", "./noscale_718312_V24_t2w_RAI_Bias.nrrd");
    rgd_script.replace("@ATLAS@", "./atlas_T1_sym_RAI.nrrd");
    rgd_script.replace("@OUTPUT_DIR@", output_dir);

    QString rigid_align_script = QDir::cleanPath(output_dir + QString("/rigid_align_script.py"));
    QFile rgd_outfile(rigid_align_script);
    rgd_outfile.open(QIODevice::WriteOnly);
    QTextStream rgd_outstream(&rgd_outfile);
    rgd_outstream << rgd_script;
    rgd_outfile.close();

    //2. WRITE MAKE_MASK_SCRIPT
    QFile msk_file(QString(":/PythonScripts/make_mask.py"));
    msk_file.open(QIODevice::ReadOnly);
    QString msk_script = msk_file.readAll();
    msk_file.close();

    msk_script.replace("@T1IMG@", "./stx_noscale_718312_V24_t1w_RAI_Bias.nrrd");
    msk_script.replace("@T2IMG@", "./stx_noscale_718312_V24_t2w_RAI_Bias.nrrd");
    msk_script.replace("@ATLAS_PATH@", "./over6m/");
    msk_script.replace("@MNI_ATLAS_PATH@", "./MNIATLAS/");
    msk_script.replace("@BIGCSF_ATLAS_PATH@", "./MulSegAtlas/");

    QString make_mask_script = QDir::cleanPath(output_dir + QString("/make_mask_script.py"));
    QFile msk_outfile(make_mask_script);
    msk_outfile.open(QIODevice::WriteOnly);
    QTextStream msk_outstream(&msk_outfile);
    msk_outstream << msk_script;
    msk_outfile.close();

    //3. WRITE Auto_SEG XML
    QFile xmlFile(QString(":/PythonScripts/ABCparam.xml"));
    xmlFile.open(QIODevice::ReadOnly);
    xml.setDevice(&xmlFile);
    xml.readNext();
    xml.text().toString();

    //4. WRITE VENT_MASK_SCRIPT
    QFile v_file(QString(":/PythonScripts/vent_mask.py"));
    v_file.open(QIODevice::ReadOnly);
    QString v_script = v_file.readAll();
    v_file.close();

    v_script.replace("@T1IMG@", "./stx_noscale_718312_V24_t1w_RAI_Bias.nrrd");
    v_script.replace("@ATLAS@", "./atlas_T1_sym_stripped_RAI-byte.nrrd");
    v_script.replace("@REG_TYPE@", Registration_Type);
    v_script.replace("@TRANS_STEP@", Transformation_Step);
    v_script.replace("@ITERATIONS@", Iterations);
    v_script.replace("@SIM_METRIC@", Sim_Metric);
    v_script.replace("@SIM_PARAMETER@", Sim_Parameter);
    v_script.replace("@GAUSSIAN@", Gaussian);
    v_script.replace("@T1_WEIGHT@", T1_Weight);
    v_script.replace("@TISSUE_SEG@", "./stx_noscale_718312_V24_t1w_RAI_FINAL_Seg.nrrd");
    v_script.replace("@VENTRICLE_MASK@" ,"./Vent_CSF-BIN-RAI-Fusion_INV.nrrd");
    v_script.replace("@OUTPUT_DIR@", output_dir);
    v_script.replace("@OUTPUT_MASK@", "_AtlasToVent.nrrd");

    QString vent_mask_script = QDir::cleanPath(output_dir + QString("/vent_mask_script.py"));
    QFile v_outfile(vent_mask_script);
    v_outfile.open(QIODevice::WriteOnly);
    QTextStream v_outstream(&v_outfile);
    v_outstream << v_script;
    v_outfile.close();

    //Notification
    QMessageBox::information(
        this,
        tr("Auto EACSF"),
        tr("Python scripts are running. It may take up to 24 hours to process.")
    );

    // RUN PYTHON    
    QString  command("python");
    QStringList params = QStringList() << main_script;
    QProcess *prc = new QProcess();    
    connect(prc, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    prc->startDetached(command, params, output_dir);
}


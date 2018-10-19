#ifndef CSFWINDOW_H
#define CSFWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore>

#include "ui_csfwindow.h"



namespace Ui {
class CSFWindow;
}

class CSFWindow : public QMainWindow, public Ui::CSFWindow
{
    Q_OBJECT

public:
    explicit CSFWindow(QWidget *parent = 0);
    ~CSFWindow();

private:
    void initializeMenuBar();
    void find_executables();
    void readDataConfiguration_d(QString filename);
    void readDataConfiguration_p(QString filename);
    void readDataConfiguration_sw(QString filename);
    void writeDataConfiguration_d(QJsonObject &json);
    void writeDataConfiguration_p(QJsonObject &json);
    void writeDataConfiguration_sw(QJsonObject &json);
    QString OpenFile();
    QString OpenDir();

    bool lineEdit_isEmpty(QLineEdit*& le);

    bool checkOptionalMasks();
    void setBestDataAlignmentOption();

    int questionMsgBox(bool checkState, QString maskType, QString action);

    void displayAtlases(QString folder_path);

    void write_main_script();
    void write_rigid_align();
    void write_make_mask();
    void write_ABCxmlfile(bool T2provided);
    void write_vent_mask();

    QProcess *prc;

    //Inputs
    QString T1img;
    QString T2img;
    QString VentricleMask;
    QString CerebMask;
    QString TissueSeg;
    QString output_dir;

    //Executables
    QStringList executables;
    QString ABC;
    QString ANTS;
    QString BRAINSFit;
    QString FSLBET;
    QString ImageMath;
    QString ImageStat;
    QString convertITKformats;
    QString WarpImageMultiTransform;
    QString Python;

    //ANTS Registration_Default
    QString Registration_Type;
    QString Transformation_Step="0.25";
    QString Iterations="100x50x25";
    QString Sim_Metric="CC";
    QString Sim_Parameter="4";
    QString Gaussian="3";
    QString T1_Weight="1";

    //Other
    bool dataSeemsAligned=false;
    bool script_running=false;
    bool outlog_file_created=false;

private slots:
    void disp_output();
    void disp_err();

    //File
    void OnLoadDataConfiguration();
    bool OnSaveDataConfiguration();
    void OnLoadParameterConfiguration();
    bool OnSaveParameterConfiguration();
    void OnLoadSoftwareConfiguration();
    bool OnSaveSoftwareConfiguration();

    //1st Tab
    void on_pushButton_T1img_clicked();
    void on_pushButton_T2img_clicked();
    void on_pushButton_BrainMask_clicked();
    void on_lineEdit_BrainMask_textChanged();
    void on_pushButton_VentricleMask_clicked();
    void on_lineEdit_VentricleMask_textChanged();
    void on_pushButton_TissueSeg_clicked();
    void on_lineEdit_TissueSeg_textChanged();
    void on_CerebellumMask_clicked();
    void on_lineEdit_CerebellumMask_textChanged();

    void on_pushButton_OutputDir_clicked();

    void on_radioButton_Index_clicked(const bool checkState);
    void on_radioButton_mm_clicked(const bool checkState);

    //2nd Tab
    void on_pushButton_ABC_clicked();
    void on_pushButton_ANTS_clicked();
    void on_pushButton_BRAINSFit_clicked();
    void on_pushButton_FSLBET_clicked();
    void on_pushButton_ImageMath_clicked();
    void on_pushButton_ImageStat_clicked();
    void on_pushButton_convertITKformats_clicked();
    void on_pushButton_WarpImageMultiTransform_clicked();
    void on_pushButton_Python_clicked();

    //3rd Tab
    void on_pushButton_ReferenceAtlasFile_clicked();

    void on_checkBox_SkullStripping_clicked(const bool checkState);
    void on_checkBox_SkullStripping_stateChanged(int state);
    void on_pushButton_SSAtlasFolder_clicked();
    void selectAtlases(QListWidgetItem*);

    //4th Tab
    void on_checkBox_TissueSeg_clicked(const bool checkState);
    void on_checkBox_TissueSeg_stateChanged(int state);

    void on_spinBox_CSFLabel_valueChanged(const int val);
    void on_pushButton_TissueSegAtlas_clicked();

    //5th Tab
    void on_checkBox_VentricleRemoval_clicked(const bool checkState);
    void on_checkBox_VentricleRemoval_stateChanged(int state);

    void on_pushButton_ROIAtlasT1_clicked();
    void on_pushButton_ROIAtlasLabel_clicked();
    void on_spinBox_LeftVentricleLabel_valueChanged(const int val);
    void on_spinBox_RightVentricleLabel_valueChanged(const int val);

    //ANTS Registration
    void on_comboBox_RegType_currentTextChanged(const QString &val);
    void on_doubleSpinBox_TransformationStep_valueChanged(const double val);
    void on_comboBox_Metric_currentTextChanged(const QString &val);
    void on_spinBox_SimilarityParameter_valueChanged(const int val);
    void on_doubleSpinBox_GaussianSigma_valueChanged(const double val);
    void on_spinBox_T1Weight_valueChanged(const QString &val);
    void on_lineEdit_Iterations_textChanged(const QString &val);

    //Execution
    void on_pushButton_execute_clicked();
};

#endif // CSFWINDOW_H

#ifndef CSFWINDOW_H
#define CSFWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore>
#include <QMap>
#include <QProcess>
#include <QMessageBox>

#include "ui_csfwindow.h"



namespace Ui {
class CSFWindow;
}

class CSFWindow : public QMainWindow, public Ui::CSFWindow
{
    Q_OBJECT

public:
    explicit CSFWindow(QWidget *m_parent = 0);
    ~CSFWindow();
    void runNoGUI(QString configFileName, QString cli_T1, QString cli_T2, QString cli_BrainMask, QString cli_TissueSeg, QString cli_subjectVMask, QString cli_CerebMask, QString cli_output_dir);
    void setTesttext(QString txt);

private:
    QStringList check_exe_in_folder(QStringList exe_list, QString dir_path, bool use_hint);
    void find_executables();
    void readConfig(QString filename, bool default_config);
    bool writeConfig(QString filename);
    QString OpenFile();
    QString OpenDir();
    QString SaveFile();

    bool lineEdit_isEmpty(QLineEdit*& le);

    bool checkOptionalMasks();
    void setBestDataAlignmentOption();

    int questionMsgBox(bool checkState, QString maskType, QString action);
    void infoMsgBox(QString message, QMessageBox::Icon type);

    void displayAtlases(QString folder_path, bool T2_provided);

    void write_main_script();
    void write_rigid_align();
    void write_make_mask();
    void write_tissue_seg();
    void write_ABCxmlfile(bool T2provided);
    void write_vent_mask();

    void run_AutoEACSF(QString cli_T1 = "", QString cli_T2 = "", QString cli_BrainMask = "", QString cli_TissueSeg = "",
                       QString cli_subjectVMask = "", QString cli_CerebMask = "", QString cli_output_dir = "");

    static const QString m_github_url;
    QProcess *prc;
    bool m_GUI;

    //Inputs
    QString T1img;
    QString T2img;
    QString CerebMask;
    QString BrainMask;
    QString TissueSeg;
    QString subjectVentricleMask;
    QString output_dir;
    QString scripts_dir;

    //Executables
    QMap<QString,QString> executables;
    QMap<QString,QStringList> script_exe;

    //ANTS Registration_Default
    QString Registration_Type;
    QString Transformation_Step;
    QString Iterations;
    QString Sim_Metric;
    QString Sim_Parameter;
    QString Gaussian;
    QString T1_Weight;

    //Other
    bool dataSeemsAligned;
    bool script_running;
    bool outlog_file_created;


private slots:
    void disp_output();
    void disp_err();
    void prc_finished(int exitCode, QProcess::ExitStatus exitStatus);

    //File
    void on_actionLoad_Configuration_File_triggered();
    void on_actionSave_Configuration_triggered();

    //Window
    void on_actionShow_executables_toggled(bool toggled);

    //About
    void on_actionAbout_triggered();

    //1st Tab
    void on_pushButton_T1img_clicked();
    void on_pushButton_T2img_clicked();
    void on_lineEdit_T2img_textChanged();
    void on_pushButton_BrainMask_clicked();
    void on_lineEdit_BrainMask_textChanged();
    void on_pushButton_VentricleMask_clicked();
    void on_pushButton_TissueSeg_clicked();
    void on_lineEdit_TissueSeg_textChanged();
    void on_CerebellumMask_clicked();
    void on_lineEdit_CerebellumMask_textChanged();

    void on_pushButton_OutputDir_clicked();

    void on_radioButton_Index_clicked(const bool checkState);
    void on_radioButton_mm_clicked(const bool checkState);

    //2nd Tab
    void updateExecutables(QString exeName, QString path);

    //3rd Tab
    void on_pushButton_ReferenceAtlasFile_clicked();

    void on_checkBox_SkullStripping_clicked(const bool checkState);
    void on_checkBox_SkullStripping_stateChanged(int state);
    void on_pushButton_SSAtlasFolder_clicked();
    void selectAtlases(QListWidgetItem*);

    //4th Tab
    void on_checkBox_TissueSeg_clicked(const bool checkState);
    void on_checkBox_TissueSeg_stateChanged(int state);

    void on_pushButton_TissueSegAtlas_clicked();

    //5th Tab
    void on_checkBox_VentricleRemoval_stateChanged(int state);

    void on_pushButton_templateT1Ventricle_clicked();
    void on_pushButton_templateInvMaskVentricle_clicked();

    //ANTS Registration
    void on_comboBox_RegType_currentTextChanged(const QString &val);
    void on_doubleSpinBox_TransformationStep_valueChanged();
    void on_comboBox_Metric_currentTextChanged(const QString &val);
    void on_spinBox_SimilarityParameter_valueChanged(const int val);
    void on_doubleSpinBox_GaussianSigma_valueChanged(const double val);
    void on_spinBox_T1Weight_valueChanged(const QString &val);
    void on_lineEdit_Iterations_textChanged(const QString &val);

    //CSF Density
    void on_pushButton_LH_inner_clicked();
    void on_pushButton_RH_inner_clicked();
    void on_checkBox_CSFDensity_stateChanged(int state);

    //Execution
    void on_pushButton_execute_clicked();
};

#endif // CSFWINDOW_H

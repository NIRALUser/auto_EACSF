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
    void writeDataConfiguration_d(QJsonObject &json);
    void writeDataConfiguration_p(QJsonObject &json);
    void writeDataConfiguration_sw(QJsonObject &json);
    QString OpenFile();


    void read_Software(QJsonObject &json);
    void write_Software(QJsonObject &json);

    QString sName;
    QString sPath;

    int indexVal;
    double mm;

    bool notPreAlign;
    bool preAlign;
    bool skullStr;

    //Scripts
    QXmlStreamReader xml;

public slots:
    void readyReadStandardOutput();

private slots:

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
    void on_pushButton_TissueSeg_clicked();
    void on_pushButton_BrainMask_clicked();
    void on_pushButton_VentricleMask_clicked();
    void on_pushButton_OutputDir_clicked();

    //2nd Tab
    void on_pushButton_ABC_clicked();
    void on_pushButton_ANTS_clicked();
    void on_pushButton_BRAINSFit_clicked();
    void on_pushButton_FSLBET_clicked();
    void on_pushButton_ImageMath_clicked();
    void on_pushButton_ITK_clicked();
    void on_pushButton_N4_clicked();
    void on_pushButton_Python_clicked();

    //3rd Tab    
    void on_pushButton_ReferenceAtlasDir_clicked();

    //4th Tab
    void on_pushButton_TissueSegAtlas_clicked();
    void on_spinBox_Index_valueChanged(int arg1);
    void on_doubleSpinBox_mm_valueChanged(double arg1);
    void on_radioButton_Index_clicked(bool checked);
    void on_radioButton_mm_clicked(bool checked);
    void on_radioButton_rigidRegistration_clicked(bool checked);
    void on_radioButton_preAligned_clicked(bool checked);
    void on_checkBox_SkullStripping_clicked(bool checked);
    void on_checkBox_SkullStripping_stateChanged(int arg1);
    void on_CerebellumMask_clicked();
    void on_pushButton_execute_clicked();

    //ANTS Registration
    void on_comboBox_RegType_currentTextChanged(const QString &arg1);
    void on_comboBox_Metric_currentTextChanged(const QString &arg1);
    void on_spinBox_SimilarityParameter_valueChanged(const QString &arg1);
    void on_doubleSpinBox_GaussianSigma_valueChanged(const QString &arg1);
    void on_spinBox_T1Weight_valueChanged(const QString &arg1);
    void on_lineEdit_Iterations_textChanged(const QString &arg1);
};

#endif // CSFWINDOW_H

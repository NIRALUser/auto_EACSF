#ifndef EXTEXECUTABLESWIDGET_H
#define EXTEXECUTABLESWIDGET_H

#include <QWidget>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>

#include <iostream>

using namespace std;

class ExtExecutablesWidget : public QWidget
{
    /*** ExtExecutables class info :
     * Written by Arthur Le Maout
     * Automates the interface building and interaction of a widget that displays and helps the user managing the external executables used by a tool.
     * For each executable, creates a pair {QPushButton, QLineEdit}. The button displays the name of the executable and the lineEdit its path.
     * On button click, a QFileDialog opens for the user to manually select an executable.
     * To use it :
     *      1. create to pointer to an instance of the class
     *      2. call the function buildInterface(QMap<QString,QString>)
     *      3. connect the signal newExePath(QString,QString) to the slot of your application that updates the executables paths.
     * You must provide the building interface function a QMap that represents the exectuables : name (keys) and path (values).
     * The signals sends both the name and the path of the executable that has been changed.
     * You may specify a default directory for the QFileDialog using the setExeDir(QString) function.
     * If no default directory is specified, the QFileDialog will open in the Qt::currentDir() of the application.
     ***/
    Q_OBJECT
public:
    explicit ExtExecutablesWidget(QWidget *m_parent = NULL);
    vector<QString> buildInterface(QMap<QString, QString> exeMap); //Builds the interface
    void setExeDir(QString dir); //Sets the default directory for the QFileDialog
    QJsonArray GetExeArray();
    QMap<QString, QString> GetExeMap();

    QString findExecutable(QString, QString);

private:
    QString m_exeDir; //Default directory for the QFileDialog
    QMap<QString, QLineEdit*> m_ExeMap;

signals:
    void newExePath(QString exeName,QString path); //Signal emitted when an executable's path has been changed by the user (to be connected to the application)

private slots:
    void exeQpbTriggered(); //On trigger of a button, opens a dialog and updates the corresponding lineEdit
    void exeLinedTextChanged(QString new_text); //On change of a line edit, gets the info about the sender (button name = executable name) and emit the signal newExePath(QString,QString)
};

#endif // EXTEXECUTABLESWIDGET_H

#ifndef EXTEXECUTABLESWIDGET_H
#define EXTEXECUTABLESWIDGET_H

#include <QWidget>
#include <QMap>

class ExtExecutablesWidget : public QWidget
{
    /*** ExtExecutables class info :
     * Written by Arthur Le Maout
     * Automates the interface building and interaction of a widget that displays and helps the user managing the external executables used by a tool.
     * For each executable, creates a pair {QPushButton, QLineEdit}. The button displays the name of the executable and the lineEdit its path.
     * On button click, a QFileDialog opens for the user to manually select an executable.
     * To use it, create to pointer to an instance of the class and simply call the function buildInterface(QMap<QString,QString>).
     * You must provide the building interface function a QMap that represents the exectuables : name (keys) and path (values).
     * You may specify a default directory for the QFileDialog using the setExeDir(QString) function.
     ***/
    Q_OBJECT
public:
    explicit ExtExecutablesWidget(QWidget *m_parent = nullptr);
    void buildInterface(QMap<QString, QString> exeMap);
    void setExeDir(QString dir);

private:
    QMap<QString,QString> *m_exeMap;
    QString m_exeDir;

signals:
    void newExePath(QString exeName,QString path);

private slots:
    void exeQpbTriggered();
    void exeLinedTextChanged(QString new_text);
};

#endif // EXTEXECUTABLESWIDGET_H

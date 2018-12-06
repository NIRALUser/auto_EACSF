#ifndef EXTEXECUTABLESWIDGET_H
#define EXTEXECUTABLESWIDGET_H

#include <QWidget>
#include <QMap>

class ExtExecutablesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExtExecutablesWidget(QWidget *m_parent = nullptr);
    void setExeMap(QMap<QString, QString> *map);
    void setExeDir(QString dir);
    void buildInterface();

private:
    QMap<QString,QString> *m_exeMap;
    QString m_exeDir;

signals:

private slots:
    void exe_qpb_triggered();
    void exe_lined_textChanged(QString new_text);
};

#endif // EXTEXECUTABLESWIDGET_H

#include "extexecutableswidget.h"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QCoreApplication>

ExtExecutablesWidget::ExtExecutablesWidget(QWidget *m_parent) :
    QWidget(m_parent),
    m_exeDir("")
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

vector<QString> ExtExecutablesWidget::buildInterface(QMap<QString,QString> exeMap)
{
    QLayout *verticalLayout = new QVBoxLayout();
    vector<QString> notFound;

    foreach (const QString exeName, exeMap.keys()) //create the buttons/lineEdit for each executable
    {
        QWidget *containerWidget = new QWidget;
        QLayout *horizontalLayout = new QHBoxLayout();
        verticalLayout->addWidget(containerWidget);
        QPushButton *qpb =  new QPushButton();
        qpb->setText(exeName);
        qpb->setMinimumWidth(181);
        qpb->setMinimumHeight(31);
        qpb->setMaximumHeight(31);
        qpb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
        QObject::connect(qpb,SIGNAL(clicked()),this,SLOT(exeQpbTriggered()));
        QLineEdit *lined = new QLineEdit();
        lined->setMinimumWidth(521);
        lined->setMinimumHeight(31);
        lined->setMaximumHeight(31);
        lined->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        
        QString exe_name = findExecutable(exeName, exeMap[exeName]);

        if(!exe_name.isEmpty()){
            lined->setText(exe_name);
        }else{
            notFound.push_back(exeMap[exeName]);
        }

        QObject::connect(lined,SIGNAL(textChanged(QString)),this,SLOT(exeLinedTextChanged(QString)));
        m_ExeMap[exeName] = lined;

        horizontalLayout->addWidget(qpb);
        horizontalLayout->addWidget(lined);
        containerWidget->setLayout(horizontalLayout);
    }
    this->setLayout(verticalLayout);

    return notFound;
}

QString ExtExecutablesWidget::findExecutable(QString exe_name, QString exe_path){

     QFileInfo info = QFileInfo(exe_path);

     if(info.exists()){
        return exe_path;
     }else{

        QString env_PATH(qgetenv("PATH"));
        QStringList hints = env_PATH.split(":");

        QString app_path = QDir::cleanPath(QCoreApplication::applicationDirPath());

        cout<<app_path.toStdString()<<endl;

        hints.push_front(app_path + QString("./"));
        hints.push_front(app_path + QString("./bin/"));
        hints.push_front(app_path + QString("/../auto_EACSF-inner-install/bin/"));

        QString found_exe("");

        foreach (const QString hint, hints){

            QFileInfo info = QFileInfo(QDir::cleanPath(hint + exe_name));

            if(info.exists()){
                found_exe = info.absoluteFilePath();
                break;
            }

        }

        return found_exe;

     }

}

void ExtExecutablesWidget::setExeDir(QString dir)
{
    m_exeDir = dir;
}

void ExtExecutablesWidget::setExecutable(QString exe, QString path){
    if(m_ExeMap.find(exe) != m_ExeMap.end()){
        m_ExeMap[exe]->setText(path);    
    }
}

//SLOTS
void ExtExecutablesWidget::exeQpbTriggered()
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QLineEdit *le = NULL;
    foreach (QObject *ch, par->children())
    {
        le = qobject_cast<QLineEdit*>(ch);
        if (le)
        {
            break;
        }
    }

    QString path = QFileDialog::getOpenFileName(this,tr("Select executable"),m_exeDir);

    if (!path.isEmpty())
    {
        le->setText(path);
    }
}

void ExtExecutablesWidget::exeLinedTextChanged(QString new_text)
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QPushButton *bt = NULL;
    foreach (QObject *ch, par->children())
    {
        bt =qobject_cast<QPushButton*>(ch);
        if (bt)
        {
            break;
        }
    }
    emit newExePath(bt->text(),new_text);
}


QJsonArray ExtExecutablesWidget::GetExeArray(){

    QJsonArray exe_array;

    foreach (const QString exe_name, m_ExeMap.keys())
    {
        QJsonObject exe_obj;
        exe_obj["name"] = exe_name;
        exe_obj["path"] = m_ExeMap[exe_name]->text();
        exe_array.append(exe_obj);
    }

    return exe_array;
}

QMap<QString, QString> ExtExecutablesWidget::GetExeMap(){

    QMap<QString, QString> exe_map;

    foreach (const QString exe_name, m_ExeMap.keys())
    {
        exe_map[exe_name] = m_ExeMap[exe_name]->text();
    }

    return exe_map;
}
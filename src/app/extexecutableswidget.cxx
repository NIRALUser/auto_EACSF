#include "extexecutableswidget.h"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

ExtExecutablesWidget::ExtExecutablesWidget(QWidget *m_parent) :
    QWidget(m_parent),
    m_exeMap(nullptr),
    m_exeDir("")
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void ExtExecutablesWidget::setExeMap(QMap<QString, QString> *map)
{
    m_exeMap = map;
}

void ExtExecutablesWidget::setExeDir(QString dir)
{
    m_exeDir = dir;
}

void ExtExecutablesWidget::buildInterface()
{
    QLayout *verticalLayout = new QVBoxLayout();
    for (QString exeName : m_exeMap->keys()) //create the buttons/lineEdit for each executable
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
        QObject::connect(qpb,SIGNAL(clicked()),this,SLOT(exe_qpb_triggered()));
        QLineEdit *lined = new QLineEdit();
        lined->setMinimumWidth(521);
        lined->setMinimumHeight(31);
        lined->setMaximumHeight(31);
        lined->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        lined->setText(m_exeMap->value(exeName));
        QObject::connect(lined,SIGNAL(textChanged(QString)),this,SLOT(exe_lined_textChanged(QString)));
        horizontalLayout->addWidget(qpb);
        horizontalLayout->addWidget(lined);
        containerWidget->setLayout(horizontalLayout);
    }
    this->setLayout(verticalLayout);
}


//SLOTS
void ExtExecutablesWidget::exe_qpb_triggered()
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QLineEdit *le;
    for (QObject *ch : par->children())
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

void ExtExecutablesWidget::exe_lined_textChanged(QString new_text)
{
    QObject *sd = QObject::sender();
    QObject *par = sd->parent();
    QPushButton *bt;
    for (QObject *ch : par->children())
    {
        bt =qobject_cast<QPushButton*>(ch);
        if (bt)
        {
            break;
        }
    }
    m_exeMap->operator [](bt->text()) = new_text;
}

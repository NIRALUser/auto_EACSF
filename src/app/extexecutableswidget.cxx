#include "extexecutableswidget.h"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

ExtExecutablesWidget::ExtExecutablesWidget(QWidget *m_parent) :
    QWidget(m_parent),
    m_exeDir("")
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void ExtExecutablesWidget::buildInterface(QMap<QString,QString> exeMap)
{
    QLayout *verticalLayout = new QVBoxLayout();
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
        lined->setText(exeMap.value(exeName));
        QObject::connect(lined,SIGNAL(textChanged(QString)),this,SLOT(exeLinedTextChanged(QString)));
        horizontalLayout->addWidget(qpb);
        horizontalLayout->addWidget(lined);
        containerWidget->setLayout(horizontalLayout);
    }
    this->setLayout(verticalLayout);
}

void ExtExecutablesWidget::setExeDir(QString dir)
{
    m_exeDir = dir;
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

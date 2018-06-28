#include "loaddata_dialog.h"
#include "ui_loaddata_dialog.h"

loadData_Dialog::loadData_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadData_Dialog)
{
    ui->setupUi(this);
}

loadData_Dialog::~loadData_Dialog()
{
    delete ui;
}

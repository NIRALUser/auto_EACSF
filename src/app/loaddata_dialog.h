#ifndef LOADDATA_DIALOG_H
#define LOADDATA_DIALOG_H

#include <QDialog>

namespace Ui {
class loadData_Dialog;
}

class loadData_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit loadData_Dialog(QWidget *parent = 0);
    ~loadData_Dialog();

private:
    Ui::loadData_Dialog *ui;
};

#endif // LOADDATA_DIALOG_H

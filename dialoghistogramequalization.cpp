#include "dialoghistogramequalization.h"
#include "ui_dialoghistogramequalization.h"

DialogHistogramEqualization::DialogHistogramEqualization(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogHistogramEqualization)
{
    ui->setupUi(this);
}

DialogHistogramEqualization::~DialogHistogramEqualization()
{
    delete ui;
}

int DialogHistogramEqualization::getEqualizationType()
{
    if (ui->radioButton->isChecked())
        return 1;
    else if (ui->radioButton_2->isChecked())
        return 2;
    else if (ui->radioButton_3->isChecked())
        return 3;
    return 0;
}

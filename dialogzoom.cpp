#include "dialogzoom.h"
#include "ui_dialogzoom.h"

DialogZoom::DialogZoom(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogZoom)
{
    ui->setupUi(this);
}

DialogZoom::~DialogZoom()
{
    delete ui;
}

double DialogZoom::getValue()
{
    return ui->doubleSpinBox->value();
}

int DialogZoom::getChoice()
{
    if (ui->radioButton->isChecked())
        return 1;
    else if (ui->radioButton_2->isChecked())
        return 2;
    else if (ui->radioButton_3->isChecked())
        return 3;
    else
        return 4;
}

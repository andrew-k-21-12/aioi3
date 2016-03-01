#include "dialogquantization.h"
#include "ui_dialogquantization.h"

DialogQuantization::DialogQuantization(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQuantization)
{
    ui->setupUi(this);
}

DialogQuantization::~DialogQuantization()
{
    delete ui;
}

int DialogQuantization::quantsCount()
{
    return ui->spinBox->value();
}

void DialogQuantization::setQuantCountMaximum(int maximum)
{
    ui->spinBox->setMaximum(maximum);
    ui->spinBox->setValue(maximum);
}

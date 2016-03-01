#include "dialogotsulocal.h"
#include "ui_dialogotsulocal.h"

DialogOtsuLocal::DialogOtsuLocal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOtsuLocal)
{
    ui->setupUi(this);
}

DialogOtsuLocal::~DialogOtsuLocal()
{
    delete ui;
}

void DialogOtsuLocal::setSpinBoxes(int imageWidth, int imageHeight)
{
    ui->spinBox->setValue(imageWidth);
    ui->spinBox->setMaximum(imageWidth);
    ui->spinBox_2->setValue(imageHeight);
    ui->spinBox_2->setMaximum(imageHeight);
}

int DialogOtsuLocal::gridX()
{
    return ui->spinBox->value();
}

int DialogOtsuLocal::gridY()
{
    return ui->spinBox_2->value();
}

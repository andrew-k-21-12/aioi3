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
    ui->spinBox_2->setValue(imageHeight);
}

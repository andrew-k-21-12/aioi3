#include "dialoggammacorrection.h"
#include "ui_dialoggammacorrection.h"

DialogGammaCorrection::DialogGammaCorrection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGammaCorrection)
{
    ui->setupUi(this);
}

DialogGammaCorrection::~DialogGammaCorrection()
{
    delete ui;
}

double DialogGammaCorrection::getGamma()
{
    return ui->doubleSpinBox->value();
}

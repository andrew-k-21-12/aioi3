#include "dialogbinarization.h"
#include "ui_dialogbinarization.h"

Dialogbinarization::Dialogbinarization(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialogbinarization)
{
    ui->setupUi(this);
}

Dialogbinarization::~Dialogbinarization()
{
    delete ui;
}

int Dialogbinarization::methodChosed()
{
    if (ui->radioButton->isChecked())
        return 1;
    else if (ui->radioButton_2->isChecked())
        return 2;
    else
        return 3;
}

int Dialogbinarization::treshold1()
{
    return ui->spinBox->value();
}

int Dialogbinarization::treshold2()
{
    return ui->spinBox_2->value();
}

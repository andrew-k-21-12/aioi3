#include "dialogbasecolorcorrection.h"
#include "ui_dialogbasecolorcorrection.h"

DialogBaseColorCorrection::DialogBaseColorCorrection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBaseColorCorrection)
{
    ui->setupUi(this);
}

DialogBaseColorCorrection::~DialogBaseColorCorrection()
{
    delete ui;
}

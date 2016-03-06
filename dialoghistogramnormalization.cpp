#include "dialoghistogramnormalization.h"
#include "ui_dialoghistogramnormalization.h"

DialogHistogramNormalization::DialogHistogramNormalization(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogHistogramNormalization)
{
    ui->setupUi(this);
}

DialogHistogramNormalization::~DialogHistogramNormalization()
{
    delete ui;
}

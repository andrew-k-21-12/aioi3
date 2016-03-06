#ifndef DIALOGHISTOGRAMNORMALIZATION_H
#define DIALOGHISTOGRAMNORMALIZATION_H

#include <QDialog>

namespace Ui {
class DialogHistogramNormalization;
}

class DialogHistogramNormalization : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHistogramNormalization(QWidget *parent = 0);
    ~DialogHistogramNormalization();

private:
    Ui::DialogHistogramNormalization *ui;
};

#endif // DIALOGHISTOGRAMNORMALIZATION_H

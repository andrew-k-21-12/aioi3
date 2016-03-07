#ifndef DIALOGHISTOGRAMEQUALIZATION_H
#define DIALOGHISTOGRAMEQUALIZATION_H

#include <QDialog>

namespace Ui {
class DialogHistogramEqualization;
}

class DialogHistogramEqualization : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHistogramEqualization(QWidget *parent = 0);
    ~DialogHistogramEqualization();

    int getEqualizationType();

private:
    Ui::DialogHistogramEqualization *ui;
};

#endif // DIALOGHISTOGRAMEQUALIZATION_H

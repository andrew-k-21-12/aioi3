#ifndef DIALOGGAMMACORRECTION_H
#define DIALOGGAMMACORRECTION_H

#include <QDialog>

namespace Ui {
class DialogGammaCorrection;
}

class DialogGammaCorrection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGammaCorrection(QWidget *parent = 0);
    ~DialogGammaCorrection();

    double getGamma();

private:
    Ui::DialogGammaCorrection *ui;
};

#endif // DIALOGGAMMACORRECTION_H

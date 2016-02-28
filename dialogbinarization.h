#ifndef DIALOGBINARIZATION_H
#define DIALOGBINARIZATION_H

#include <QDialog>

namespace Ui {
class Dialogbinarization;
}

class Dialogbinarization : public QDialog
{
    Q_OBJECT

public:
    explicit Dialogbinarization(QWidget *parent = 0);
    ~Dialogbinarization();

    int methodChosed();
    int treshold1();
    int treshold2();

private:
    Ui::Dialogbinarization *ui;
};

#endif // DIALOGBINARIZATION_H

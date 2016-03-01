#ifndef DIALOGQUANTIZATION_H
#define DIALOGQUANTIZATION_H

#include <QDialog>

namespace Ui {
class DialogQuantization;
}

class DialogQuantization : public QDialog
{
    Q_OBJECT

public:
    explicit DialogQuantization(QWidget *parent = 0);
    ~DialogQuantization();

    int quantsCount();
    void setQuantCountMaximum(int maximum);

private:
    Ui::DialogQuantization *ui;
};

#endif // DIALOGQUANTIZATION_H

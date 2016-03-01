#ifndef DIALOGOTSULOCAL_H
#define DIALOGOTSULOCAL_H

#include <QDialog>

namespace Ui {
class DialogOtsuLocal;
}

class DialogOtsuLocal : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOtsuLocal(QWidget *parent = 0);
    ~DialogOtsuLocal();

    void setSpinBoxes(int imageWidth, int imageHeight);

private:
    Ui::DialogOtsuLocal *ui;
};

#endif // DIALOGOTSULOCAL_H

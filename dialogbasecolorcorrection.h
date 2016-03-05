#ifndef DIALOGBASECOLORCORRECTION_H
#define DIALOGBASECOLORCORRECTION_H

#include <QDialog>

namespace Ui {
class DialogBaseColorCorrection;
}

class DialogBaseColorCorrection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBaseColorCorrection(QWidget *parent = 0);
    ~DialogBaseColorCorrection();

private:
    Ui::DialogBaseColorCorrection *ui;
};

#endif // DIALOGBASECOLORCORRECTION_H

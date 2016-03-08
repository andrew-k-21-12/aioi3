#ifndef DIALOGPIECEWISELINEAR_H
#define DIALOGPIECEWISELINEAR_H

#include <QDialog>

namespace Ui {
class DialogPiecewiseLinear;
}

class DialogPiecewiseLinear : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPiecewiseLinear(QWidget *parent = 0);
    ~DialogPiecewiseLinear();

    const std::vector<double>& getNums();
    QString getString();
    void setText(QString text);

private slots:
    void on_textEdit_textChanged();

private:
    Ui::DialogPiecewiseLinear *ui;
    std::vector<double> nums;

    bool calcRange();
    bool isValidFormat();
};

#endif // DIALOGPIECEWISELINEAR_H

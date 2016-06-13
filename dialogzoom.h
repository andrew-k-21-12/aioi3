#ifndef DIALOGZOOM_H
#define DIALOGZOOM_H

#include <QDialog>

namespace Ui {
class DialogZoom;
}

class DialogZoom : public QDialog
{
    Q_OBJECT

public:
    explicit DialogZoom(QWidget *parent = 0);
    ~DialogZoom();

    double getValue();
    int getChoice();

private:
    Ui::DialogZoom *ui;
};

#endif // DIALOGZOOM_H

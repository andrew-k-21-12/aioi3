#ifndef DIALOGBASECOLORCORRECTION_H
#define DIALOGBASECOLORCORRECTION_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include<QColor>

namespace Ui {
class DialogBaseColorCorrection;
}

class DialogBaseColorCorrection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBaseColorCorrection(QWidget *parent = 0);
    ~DialogBaseColorCorrection();
    bool eventFilter(QObject *object, QEvent *event);

    void setPixmap(QPixmap &pixmap);
    QColor sourceColor();
    QColor destinationColor();

private slots:
    void on_pushButton_clicked();

    void on_toolButton_2_clicked();

private:
    Ui::DialogBaseColorCorrection *ui;
    QGraphicsScene *scene;
    QGraphicsScene *scene_2;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPixmapItem *pixmapItem_2;
};

#endif // DIALOGBASECOLORCORRECTION_H

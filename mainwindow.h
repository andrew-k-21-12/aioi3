#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int otsu(QImage &image, std::vector<int> &grays, int startX, int startY, int endX, int endY);

private slots:
    void on_actionLoad_triggered();

    void on_actionGrayscale_triggered();

    void on_actionBinarization_manual_triggered();

    void on_actionOtsu_global_triggered();

    void on_actionSave_triggered();

    void on_actionBrightness_gradient_triggered();

    void on_actionOtsu_local_triggered();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsScene *scene_2;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPixmapItem *pixmapItem_2;
};

#endif // MAINWINDOW_H

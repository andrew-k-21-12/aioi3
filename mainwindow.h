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
    bool eventFilter(QObject *object, QEvent *event);

    int otsu(QImage &image, std::vector<int> &grays, int startX, int startY, int endX, int endY);

private slots:
    void on_actionLoad_triggered();

    void on_actionGrayscale_triggered();

    void on_actionBinarization_manual_triggered();

    void on_actionOtsu_global_triggered();

    void on_actionSave_triggered();

    void on_actionBrightness_gradient_triggered();

    void on_actionOtsu_local_triggered();

    void on_actionBrightness_quantization_triggered();

    void on_checkBox_toggled(bool checked);

    void on_actionBase_color_correction_triggered();

    void on_actionGray_world_triggered();

    void on_actionLinear_triggered();

    void on_actionGamma_correction_triggered();

    void on_actionBrightness_normalization_triggered();

    void on_actionHistogram_equalization_triggered();

    void on_actionLg_triggered();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsScene *scene_2;
    QGraphicsScene *sceneHist;
    QGraphicsScene *sceneHist_2;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPixmapItem *pixmapItem_2;
    QGraphicsPixmapItem *pixmapItem_3;
    QGraphicsPixmapItem *pixmapItem_4;
    std::vector<int> hist;
    std::vector<int> hist_2;
    int maxLevel;
    int maxLevel_2;
};

#endif // MAINWINDOW_H

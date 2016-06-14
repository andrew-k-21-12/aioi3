#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QList>

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

private slots:

    void on_actionLoad_triggered();

    void on_actionColorHistogram_triggered();

    void on_actionShapeHistogramDistance_triggered();

private:

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsScene *scene_2;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsPixmapItem *pixmapItem_2;
    std::vector<int> hist;
    std::vector<int> hist_2;
    int maxLevel;
    int maxLevel_2;
    QString pieceWiseLinearText;
    std::vector< std::vector<double> > mA;
    QList<QPixmap> picturesR;
    int picturesRind;

    double histDistance(std::vector<int> &hist1, std::vector<int> &hist2);
    std::vector<int> colHist(QPixmap &pixmap);
    void writeBits(int n, QBitArray &arr, int from, int to);
    std::vector<double> hist2(QPixmap &pixmap, int size1, int size2);
    double histDistance2(std::vector<double> &hist1, std::vector<double> &hist2);

};

#endif // MAINWINDOW_H

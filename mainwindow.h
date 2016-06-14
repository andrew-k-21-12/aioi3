#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <vector>
#include <map>
#include <cmath>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QList>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QRgb>
#include <QtMath>
#include <QByteArray>
#include <QBitArray>
#include <QScrollBar>
#include <QPainter>

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

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


    double histDistance(std::vector<int> &hist1, std::vector<int> &hist2)
    {
        int size = hist1.size();
        std::vector<int> dists(size);
        for (int i = 0; i < size; ++i)
            dists[i] = qAbs(hist1[i] - hist2[i]);

        std::vector<double> tmp(dists.size());

        for (int j = 0; j < size; ++j)
        {
            double sum = 0.0;
            for (int i = 0; i < size; ++i)
                sum += dists[j] * mA[i][j];
            tmp[j] = sum;
        }

        double sum = 0.0;
        for (int i = 0; i < size; ++i)
            sum += dists[i] * tmp[i];
        double res = qSqrt(sum);
        return res;
    }

    std::vector<int> colHist(QPixmap &pixmap)
    {
        std::vector<int> res(64, 0);
        QImage img = pixmap.toImage();

        for (int y = 0; y < img.height(); ++y)
        {
            for (int x = 0; x < img.width(); ++x)
            {
                QRgb pixel = img.pixel(x, y);
                QBitArray bArr(3 * 8);
                writeBits(qRed(pixel), bArr, 0, 7);
                writeBits(qGreen(pixel), bArr, 8, 15);
                writeBits(qBlue(pixel), bArr, 16, 23);
                int i = (int) bArr[6] + (int) bArr[7] * 2 + (int) bArr[14] * 4
                      + (int) bArr[15] * 8 + (int) bArr[22] * 16 + (int) bArr[23] * 32;
                ++res[i];
            }
        }

        return res;
    }

    void writeBits(int n, QBitArray &arr, int from, int to)
    {
        for(int k = from; k < to; ++k)
        {
            int mask =  1 << k;
            int masked_n = n & mask;
            int thebit = masked_n >> k;
            arr[k] = thebit;
        }
    }

    std::vector<double> hist2(QPixmap &pixmap, int size1, int size2)
    {
        std::vector<double> res(size1 + size2);
        QImage img = pixmap.toImage();

        int xMin = img.width();
        int xMax = -1;
        int yMin = img.height();
        int yMax = -1;

        for (int y = 0; y < img.height(); ++y)
        {
            for (int x = 0; x < img.width(); ++x)
            {
                if ( qRed( img.pixel(x, y) ) > 50)
                    continue;

                if (x < xMin)
                    xMin = x;
                else if (x > xMax)
                    xMax = x;
            }
            if (y < yMin)
                yMin = y;
            else if (y > yMax)
                yMax = y;
        }

        double xScale = (double)(xMax - xMin) / size1;
        double yScale = (double)(yMax - yMin) / size2;

        int count = 0;
        for (int x = xMin; x < xMax + 1; ++x)
        {
            for (int y = yMin; y < yMax + 1; ++y)
            {
                if ( qRed( img.pixel(x, y) ) > 50)
                    continue;

                int xInd = (int)qMin((x - xMin) / xScale, (double) res.size() - 1);
                int yInd = (int)qMin((y - yMin) / yScale + size1, (double) res.size() - 1);

                ++res[xInd];
                ++res[yInd];
                ++count;
            }
        }

        for (int i = 0; i < (int) res.size(); ++i)
            res[i] /= (double) count;

        return res;
    }

    double histDistance2(std::vector<double> &hist1, std::vector<double> &hist2)
    {
        int size = hist1.size();
        double sum = 0.0;
        for (int i = 0; i < size; ++i)
            sum += qAbs(hist1[i] - hist2[i]);
        return sum;
    }



public:

    explicit MainWindow(QWidget *parent = 0) :
             QMainWindow(parent),
             ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
        maxLevel = 0;
        maxLevel_2 = 0;
        this->setWindowTitle(tr("Поиск по шаблону"));
        scene = new QGraphicsScene(this);
        pixmapItem = scene->addPixmap(QPixmap());
        ui->graphicsView->setScene(scene);
        scene_2 = new QGraphicsScene(this);
        pixmapItem_2 = scene_2->addPixmap(QPixmap());
        ui->graphicsView->installEventFilter(this);

        mA.assign(64, std::vector<double>(64, 0));
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 64; ++j)
            {
                QBitArray b1(8);
                writeBits(i, b1, 0, 7);
                QBitArray b2(8);
                writeBits(j, b2, 0, 7);
                int r = 0;
                for (int k = 0; k < b1.size(); ++k)
                    r += (int) (b1[k] ^ b2[k]);
                mA[i][j] = r / 8.0;
            }

    }

    ~MainWindow()
    {
        delete ui;
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        // ПО-ДРУГОМУ РЕЗУЛЬТАТЫ!

    //    if (object == ui->graphicsView_2 && event->type() == QEvent::MouseButtonPress)
    //    {
    //        ++picturesRind;
    //        int curInd = picturesRind % picturesR.size();
    //        pixmapItem_2->setPixmap(picturesR[curInd]);
    //        scene_2->setSceneRect(QRectF(picturesR[curInd].rect()));
    //    }
        return false;
    }



private slots:

    void on_actionLoad_triggered()
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"));
        QPixmap pixmap(fileName);
        if ( ! pixmap.isNull() )
        {
            pixmapItem->setPixmap(pixmap);
            scene->setSceneRect(QRectF(pixmap.rect()));
            QPixmap pixmapEmpty;
            pixmapItem_2->setPixmap(pixmapEmpty);
            scene_2->setSceneRect(QRectF(pixmapEmpty.rect()));

        }
        else
            ui->statusBar->showMessage(tr("File loading error"), 3000);
    }

    void on_actionColorHistogram_triggered()
    {
        QPixmap pixmap = pixmapItem->pixmap().copy();

        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();

        if (width == 0 || height == 0)
        {
            ui->statusBar->showMessage( tr("Error. Image bad size"), 3000 );
            return;
        }

        int threshold = 180000;
        picturesR.clear();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose directory") );
        QDir dir(dirName);
        QDirIterator it(dir.absolutePath(), QDir::Files);
        std::vector<int> origHist = colHist(pixmap);
        for ( ; it.hasNext(); it.next() )
        {
            QString curFileName = it.filePath();
            QPixmap curPixmap(curFileName);
            if ( curPixmap.isNull() )
                continue;
            std::vector<int> curHist = colHist(curPixmap);
            double dist = histDistance(origHist, curHist);

            if (dist < threshold)
                picturesR.append(curPixmap);
        }

        QString mes = tr("Images: ") + QString::number(picturesR.size());
        ui->statusBar->showMessage(mes, 3000);
        if (picturesR.size() > 0)
        {
            picturesRind = 0;
            pixmapItem_2->setPixmap(picturesR[0]);
            scene_2->setSceneRect(QRectF(picturesR[0].rect()));
        }
    }

    void on_actionShapeHistogramDistance_triggered()
    {
        QPixmap pixmap = pixmapItem->pixmap().copy();

        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();

        if (width == 0 || height == 0)
        {
            ui->statusBar->showMessage( tr("Error. Image bad size"), 3000 );
            return;
        }

        double threshold = 0.8;
        picturesR.clear();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose directory") );
        QDir dir(dirName);
        QDirIterator it(dir.absolutePath(), QDir::Files);
        std::vector<double> origHist = hist2(pixmap, 20, 20);
        for ( ; it.hasNext(); it.next() )
        {
            QString curFileName = it.filePath();
            QPixmap curPixmap(curFileName);
            if ( curPixmap.isNull() )
                continue;
            std::vector<double> curHist = hist2(curPixmap, 20, 20);
            double dist = histDistance2(origHist, curHist);

            qDebug() << dist;
            if (dist < threshold)
                picturesR.append(curPixmap);
        }

        QString mes = tr("Images: ") + QString::number(picturesR.size());
        ui->statusBar->showMessage(mes, 3000);
        if (picturesR.size() > 0)
        {
            picturesRind = 0;
            pixmapItem_2->setPixmap(picturesR[0]);
            scene_2->setSceneRect(QRectF(picturesR[0].rect()));
        }
    }

};

#endif // MAINWINDOW_H

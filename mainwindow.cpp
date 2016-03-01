#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QFileDialog>
#include<QImage>
#include<QRgb>
#include<QtMath>
#include<QByteArray>
#include<vector>

#include "dialogbinarization.h"

#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Kazaryan Paint"));
    scene = new QGraphicsScene(this);
    pixmapItem = scene->addPixmap(QPixmap());
    ui->graphicsView->setScene(scene);
    scene_2 = new QGraphicsScene(this);
    pixmapItem_2 = scene_2->addPixmap(QPixmap());
    ui->graphicsView_2->setScene(scene_2);
}

MainWindow::~MainWindow()
{
    delete scene_2;
    delete scene;
    delete ui;
}

void MainWindow::on_actionLoad_triggered()
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

void MainWindow::on_actionGrayscale_triggered()
{
    QPixmap pixmap = pixmapItem->pixmap().copy();

    QImage image = pixmap.toImage();
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int gray = qPow(
                       0.2126 * qPow(qRed(oldColor), 2.2) +
                       0.7152 * qPow(qGreen(oldColor), 2.2) +
                       0.0722 * qPow(qBlue(oldColor), 2.2),
                       1/2.2
                       );
            QRgb newColor = qRgba(gray, gray, gray, qAlpha(oldColor));
            image.setPixel(x, y, newColor);
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));
}

void MainWindow::on_actionBinarization_manual_triggered()
{
    Dialogbinarization dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        QPixmap pixmap = pixmapItem->pixmap().copy();

        QRgb black = qRgb(0,0,0);
        QRgb white = qRgb(255,255,255);
        QImage image = pixmap.toImage();
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
            {
                QRgb oldColor = image.pixel(x, y);
                int gray = qPow(
                           0.2126 * qPow(qRed(oldColor), 2.2) +
                           0.7152 * qPow(qGreen(oldColor), 2.2) +
                           0.0722 * qPow(qBlue(oldColor), 2.2),
                           1/2.2
                           );
                QRgb newColor;
                if (dialog.methodChosed() == 1)
                {
                    int treshold = dialog.treshold1();
                    if ( gray < treshold )
                        newColor = black;
                    else
                        newColor = white;
                }
                else if (dialog.methodChosed() == 2)
                {
                    int treshold = dialog.treshold1();
                    if ( gray > treshold )
                        newColor = black;
                    else
                        newColor = white;
                }
                else
                {
                    int treshold1 = dialog.treshold1();
                    int treshold2 = dialog.treshold2();
                    if ( treshold1 < gray && gray <= treshold2 )
                        newColor = black;
                    else
                        newColor = white;
                }
                image.setPixel(x, y, newColor);
            }

        pixmap.convertFromImage(image);

        pixmapItem_2->setPixmap(pixmap);
        scene_2->setSceneRect(QRectF(pixmap.rect()));
    }
}

void MainWindow::on_actionOtsu_global_triggered()
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

    int min = 256;
    int max = -1;
    std::vector<int> grays(width * height, 0);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int gray = qPow(
                       0.2126 * qPow(qRed(oldColor), 2.2) +
                       0.7152 * qPow(qGreen(oldColor), 2.2) +
                       0.0722 * qPow(qBlue(oldColor), 2.2),
                       1/2.2
                       );
            grays[x + y * width] = gray;
            if (gray < min)
                min = gray;
            if (gray > max)
                max = gray;
        }

    int histSize = max - min + 1;

    std::vector<int> hist(histSize);
    for (unsigned int i = 0; i < grays.size(); ++i)
        ++hist[grays[i] - min];
    int t = 0;
    int t1 = 0;
    int sz = max - min;
    for (int i = 0; i < sz; ++i)
    {
        t += i * hist[i];
        t1 += hist[i];
    }
    int alpha = 0;
    int beta = 0;
    int threshold = 0;
    double w1;
    double a;
    double sigma;
    double maxSigma = -1;
    for (int i = 0; i < sz; ++i)
    {
        alpha += i * hist[i];
        beta += hist[i];
        w1 = (double) beta / t1;
        a = (double) alpha / beta - (double) (t - alpha) / (t1 - beta);
        sigma = w1 * (1 - w1) * a * a;
        if (sigma > maxSigma)
        {
            maxSigma = sigma;
            threshold = i;
        }
    }
    int finalThreshold = threshold + min;

    QRgb black = qRgb(0,0,0);
    QRgb white = qRgb(255,255,255);
    QRgb newColor;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int gray = grays[x + y * width];
            if ( gray < finalThreshold )
                newColor = black;
            else
                newColor = white;
            image.setPixel(x, y, newColor);
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"));
    if ( pixmapItem_2->pixmap().save(fileName) )
        ui->statusBar->showMessage(tr("File saved successful!"), 3000);
    else
        ui->statusBar->showMessage(tr("File saving error"), 3000);
}

void MainWindow::on_actionBrightness_gradient_triggered()
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

    std::vector< std::vector<int> > grays( width, std::vector<int>(height) );
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int gray = qPow(
                       0.2126 * qPow(qRed(oldColor), 2.2) +
                       0.7152 * qPow(qGreen(oldColor), 2.2) +
                       0.0722 * qPow(qBlue(oldColor), 2.2),
                       1/2.2
                       );
            grays[x][y] = gray;
        }

    int G_x;
    int G_y;
    int G;
    unsigned long int dividend = 0;
    unsigned int divisor = 0;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            if (x == 0)
                G_x = grays[x+1][y];
            else if (x == width - 1)
                G_x = grays[x-1][y];
            else
                G_x = grays[x+1][y] - grays[x-1][y];
            if (y == 0)
                G_y = grays[x][y+1];
            else if (y == height - 1)
                G_y = grays[x][y-1];
            else
                G_y = grays[x][y+1] - grays[x][y-1];
            G = qMax( qAbs(G_x), qAbs(G_y) );
            dividend += grays[x][y] * G;
            divisor += G;
        }

    int threshold = dividend / divisor;

    if (0 <= threshold && threshold <= 255)
    {
        QRgb black = qRgb(0,0,0);
        QRgb white = qRgb(255,255,255);
        QRgb newColor;
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int gray = grays[x][y];
                if ( gray < threshold )
                    newColor = black;
                else
                    newColor = white;
                image.setPixel(x, y, newColor);
            }
    }
    else
        ui->statusBar->showMessage(tr("Error. Invalid threshold"), 3000);

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));
}

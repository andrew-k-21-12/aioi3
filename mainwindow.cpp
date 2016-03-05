#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QFileDialog>
#include<QImage>
#include<QRgb>
#include<QtMath>
#include<QByteArray>
#include<QScrollBar>
#include<vector>
#include<map>

#include "dialogbinarization.h"
#include "dialogotsulocal.h"
#include "dialogquantization.h"

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
    ui->graphicsView->installEventFilter(this);
    ui->graphicsView_2->installEventFilter(this);
    // Connections
    connect(ui->graphicsView->horizontalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->graphicsView_2->horizontalScrollBar(),
            SLOT(setValue(int)));
    connect(ui->graphicsView_2->horizontalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->graphicsView->horizontalScrollBar(),
            SLOT(setValue(int)));
    connect(ui->graphicsView->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->graphicsView_2->verticalScrollBar(),
            SLOT(setValue(int)));
    connect(ui->graphicsView_2->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            ui->graphicsView->verticalScrollBar(),
            SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
    delete scene_2;
    delete scene;
    delete ui;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (ui->checkBox->isChecked() && object == ui->graphicsView && event->type() == QEvent::Paint)
        ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    if (ui->checkBox->isChecked() && object == ui->graphicsView_2 && event->type() == QEvent::Paint)
        ui->graphicsView_2->fitInView(pixmapItem_2, Qt::KeepAspectRatio);
    return false;
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

int MainWindow::otsu(QImage &image, std::vector<int> &grays, int startX, int startY, int endX, int endY)
{
    for (int y = startY; y < endY; ++y)
        for (int x = startX; x < endX; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int gray = qPow(
                       0.2126 * qPow(qRed(oldColor), 2.2) +
                       0.7152 * qPow(qGreen(oldColor), 2.2) +
                       0.0722 * qPow(qBlue(oldColor), 2.2),
                       1/2.2
                       );
            grays[x + y * (endX - startX)] = gray;
        }

    std::vector<int> hist(256, 0);
    int graysStart = startX + startY * (endX - startX);
    int graysEnd = (endX - 1) + (endY - 1) * (endX - startX) + 1;
    for (int i = graysStart; i < graysEnd; ++i)
        ++hist[grays[i]];
    int t = 0;
    int t1 = 0;
    for (int i = 0; i < 256; ++i)
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
    for (int i = 0; i < 256; ++i)
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
    int finalThreshold = threshold;

    return finalThreshold;
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

    std::vector<int> grays(width * height, 0);

    int finalThreshold = otsu(image, grays, 0, 0, width, height);

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

void MainWindow::on_actionOtsu_local_triggered()
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

    DialogOtsuLocal dialog;
    dialog.setSpinBoxes(pixmapItem->pixmap().width(), pixmapItem->pixmap().height());

    if (dialog.exec() == QDialog::Rejected)
        return;

    std::vector<int> grays(width * height, 0);

    int countX = dialog.gridX();
    int countY = dialog.gridY();

    double shiftY = ( (double) height ) / countY;
    double shiftX = ( (double) width ) / countX;

    double curX = 0.0;
    double curY = 0.0;
    double nextX = 0.0;
    double nextY = 0.0;

    int cX;
    int cY;
    int nX;
    int nY;

    QRgb black = qRgb(0,0,0);
    QRgb white = qRgb(255,255,255);
    QRgb newColor;

    for (int i = 0; i < countY; ++i)
    {
        nextY += shiftY;
        cY = qFloor(curY);
        nY = qFloor(nextY);
        curX = 0.0;
        nextX = 0.0;
        for (int j = 0; j < countX; ++j)
        {
            nextX += shiftX;
            cX = qFloor(curX);
            nX = qFloor(nextX);

            int threshold = otsu(image, grays, cX, cY, nX, nY);

            for (int y = cY; y < nY; ++y)
                for (int x = cX; x < nX; ++x)
                {
                    int gray = grays[x + y * (nX - cX)];
                    if ( gray < threshold )
                        newColor = black;
                    else
                        newColor = white;
                    image.setPixel(x, y, newColor);
                }

            curX = nextX;
        }
        curY = nextY;
    }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));
}

void MainWindow::on_actionBrightness_quantization_triggered()
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

    std::vector<int> grays(width * height);

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
        }

    std::vector<unsigned int> Rs(256, 0);
    std::vector<unsigned int> Gs(256, 0);
    std::vector<unsigned int> Bs(256, 0);
    std::vector<int> hist(256, 0);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int num = grays[x + y * width];
            Rs[num] += qRed( image.pixel(x, y) );
            Gs[num] += qGreen( image.pixel(x, y) );
            Bs[num] += qBlue( image.pixel(x, y) );
            ++hist[num];
        }

    int quantsCountMaximum = 0;
    std::map<int, QRgb> colors;
    for (int i = 0; i < 256; ++i)
    {
        if (hist[i] == 0)
            continue;
        Rs[i] /= hist[i];
        Gs[i] /= hist[i];
        Bs[i] /= hist[i];
        colors[i] = qRgb(Rs[i], Gs[i], Bs[i]);
        ++quantsCountMaximum;
    }

    DialogQuantization dialog;
    dialog.setQuantCountMaximum(quantsCountMaximum);

    if (dialog.exec() == QDialog::Rejected)
        return;

    int quantsCount = dialog.quantsCount();

    double shift = 256 / quantsCount;
    double cur = 0.0;
    double next = 0.0;

    for (int i = 0; i < quantsCount; ++i)
    {
        next += shift;
        int c = qFloor(cur);
        int n = qFloor(next);

        int minC = 256;
        for (std::map<int, QRgb>::iterator it = colors.begin(); it != colors.end(); ++it)
            if (c <= (it->first) && (it->first) < n)
                if (it->first < minC)
                    minC = it->first;
        QRgb newColor = colors[minC];

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int num = grays[x + y * width];
                if (c <= num && num < n)
                    image.setPixel(x, y, newColor);
                else
                    continue;
            }
        cur = next;
    }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));
    //ui->graphicsView_2->fitInView(scene_2->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::on_checkBox_toggled(bool checked)
{
    if (checked)
    {
        ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
        ui->graphicsView_2->fitInView(pixmapItem_2, Qt::KeepAspectRatio);
    }
    else
    {
        ui->graphicsView->resetTransform();
        ui->graphicsView_2->resetTransform();
    }
}

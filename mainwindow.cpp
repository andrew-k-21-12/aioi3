#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QFileDialog>
#include<QImage>
#include<QRgb>
#include<QtMath>
#include<QByteArray>
#include<QScrollBar>
#include<QPainter>
#include<vector>
#include<map>
#include<cmath>

#include "dialogbinarization.h"
#include "dialogotsulocal.h"
#include "dialogquantization.h"
#include "dialogbasecolorcorrection.h"
#include "dialoghistogramequalization.h"
#include "dialoggammacorrection.h"
#include "dialogpiecewiselinear.h"

#include<QDebug>

void calcHist(QPixmap &pixmap, std::vector<int> &hist, int &maxLevel)
{
    QImage image = pixmap.toImage();
    const int histSize = 256;
    hist.assign(histSize, 0);
    maxLevel = -1;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
        {
            QColor oldColor(image.pixel(x, y));
            int gray = oldColor.value();
            ++hist[gray];
            if (hist[gray] > maxLevel)
                maxLevel = hist[gray];
        }
}

void drawHist(QGraphicsPixmapItem *pixmapItem, std::vector<int> &hist, int maxLevel)
{
    QGraphicsScene *scene = pixmapItem->scene();
    QGraphicsView *view = scene->views()[0];
    int width = view->contentsRect().width();
    int height = view->contentsRect().height();
    QColor black(0, 0, 0);
    QColor white(255, 255, 255);
    QColor red(255, 0, 0);
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(white);
    QPainter p(&image);
    double shift = (double) image.width() / hist.size();
    double step = (double) image.height() / maxLevel;
    for (unsigned int i = 0; i < hist.size(); ++i)
    {
        if (i == 0 || i == hist.size() - 1)
        {
            p.setPen(red);
            p.setBrush(QBrush(red));
        }
        else if (i == 1)
        {
            p.setPen(black);
            p.setBrush(QBrush(black));
        }
        if (hist[i] == 0)
            continue;
        int curHeight = hist[i] * step;
        p.drawRect(qFloor(i * shift), qFloor(image.height() - curHeight), qFloor(shift), qFloor(curHeight));
    }
    pixmapItem->setPixmap(QPixmap::fromImage(image));
    scene->setSceneRect(QRectF(image.rect()));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    maxLevel = 0;
    maxLevel_2 = 0;
    this->setWindowTitle(tr("Kazaryan Paint"));
    scene = new QGraphicsScene(this);
    pixmapItem = scene->addPixmap(QPixmap());
    ui->graphicsView->setScene(scene);
    scene_2 = new QGraphicsScene(this);
    pixmapItem_2 = scene_2->addPixmap(QPixmap());
    ui->graphicsView_2->setScene(scene_2);
    ui->graphicsView->installEventFilter(this);
    ui->graphicsView_2->installEventFilter(this);
    ui->graphicsView_3->installEventFilter(this);
    ui->graphicsView_4->installEventFilter(this);
    //
    sceneHist = new QGraphicsScene(this);
    pixmapItem_3 = sceneHist->addPixmap(QPixmap());
    ui->graphicsView_3->setScene(sceneHist);
    sceneHist_2 = new QGraphicsScene(this);
    pixmapItem_4 = sceneHist_2->addPixmap(QPixmap());
    ui->graphicsView_4->setScene(sceneHist_2);
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
    delete sceneHist_2;
    delete sceneHist;
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
    if (object == ui->graphicsView_3 && event->type() == QEvent::Paint)
        drawHist(pixmapItem_3, hist, maxLevel);
    if (object == ui->graphicsView_4 && event->type() == QEvent::Paint)
        drawHist(pixmapItem_4, hist_2, maxLevel_2);
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
        calcHist(pixmap, hist, maxLevel);
        drawHist(pixmapItem_3, hist, maxLevel);
        hist_2.assign(256, 0);
        drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

        calcHist(pixmap, hist_2, maxLevel_2);
        drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
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

void MainWindow::on_actionBase_color_correction_triggered()
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

    DialogBaseColorCorrection dialog;
    dialog.setPixmap(pixmap);

    if (dialog.exec() == QDialog::Rejected)
        return;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            double red = qRed(oldColor);
            double green = qGreen(oldColor);
            double blue = qBlue(oldColor);
            QColor dst = dialog.destinationColor();
            QColor src = dialog.sourceColor();
            red *= (double) dst.red() / src.red();
            green *= (double) dst.green() / src.green();
            blue *= (double) dst.blue() / src.blue();
            if (red < 0)
                red = 0;
            if (red > 255)
                red = 255;
            if (green < 0)
                green = 0;
            if (green > 255)
                green = 255;
            if (blue < 0)
                blue = 0;
            if (blue > 255)
                blue = 255;
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionGray_world_triggered()
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

    double avgRed = 0.0;
    double avgGreen = 0.0;
    double avgBlue = 0.0;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            avgRed += qRed(oldColor);
            avgGreen += qGreen(oldColor);
            avgBlue += qBlue(oldColor);
        }

    int length = width * height;
    avgRed /= length;
    avgGreen /= length;
    avgBlue /= length;
    double avg = (avgRed + avgGreen + avgBlue) / 3;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            double red = qRed(oldColor) * avg / avgRed;
            double green = qGreen(oldColor) * avg / avgGreen;
            double blue = qBlue(oldColor) * avg / avgBlue;
            if (red < 0)
                red = 0;
            if (red > 255)
                red = 255;
            if (green < 0)
                green = 0;
            if (green > 255)
                green = 255;
            if (blue < 0)
                blue = 0;
            if (blue > 255)
                blue = 255;
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionLinear_triggered()
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

    int minR = 256;
    int maxR = -1;
    int minG = 256;
    int maxG = -1;
    int minB = 256;
    int maxB = -1;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            if (red < minR)
                minR = red;
            if (red > maxR)
                maxR = red;
            if (green < minG)
                minG = green;
            if (green > maxG)
                maxG = green;
            if (blue < minB)
                minB = blue;
            if (blue > maxB)
                maxB = blue;
        }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = ( qRed(oldColor) - minR) * 255 / (maxR - minR);
            int green = ( qGreen(oldColor) - minG) * 255 / (maxG - minG);
            int blue = ( qBlue(oldColor) - minB) * 255 / (maxB - minB);
            if (red < 0)
                red = 0;
            if (red > 255)
                red = 255;
            if (green < 0)
                green = 0;
            if (green > 255)
                green = 255;
            if (blue < 0)
                blue = 0;
            if (blue > 255)
                blue = 255;
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionGamma_correction_triggered()
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

    DialogGammaCorrection dialog;

    if (dialog.exec() == QDialog::Rejected)
        return;

    int maxR = -1;
    int maxG = -1;
    int maxB = -1;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            if (red > maxR)
                maxR = red;
            if (green > maxG)
                maxG = green;
            if (blue > maxB)
                maxB = blue;
        }

    double gamma = dialog.getGamma();
    double cR = 255 / qPow(maxR, gamma);
    double cG = 255 / qPow(maxG, gamma);
    double cB = 255 / qPow(maxB, gamma);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = cR * qPow(qRed(oldColor), gamma);
            int green = cG * qPow(qGreen(oldColor), gamma);
            int blue = cB * qPow(qBlue(oldColor), gamma);
            if (red < 0)
                red = 0;
            if (red > 255)
                red = 255;
            if (green < 0)
                green = 0;
            if (green > 255)
                green = 255;
            if (blue < 0)
                blue = 0;
            if (blue > 255)
                blue = 255;
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionBrightness_normalization_triggered()
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

    int minV = 256;
    int maxV = -1;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QColor oldColor(image.pixel(x, y));
            int v = oldColor.value();
            if (v < minV)
                minV = v;
            if (v > maxV)
                maxV = v;
        }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QColor oldColor(image.pixel(x, y));
            int v = ( oldColor.value() - minV) * 255 / (maxV - minV);
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            QColor newColor;
            newColor.setHsv(oldColor.hue(), oldColor.saturation(), v);
            image.setPixel(x, y, newColor.rgb());
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void RGB_equalization(QImage &image)
{
    int width = image.width();
    int height = image.height();

    int histSize = 256;
    std::vector<double> histR(histSize, 0.0);
    std::vector<double> histG(histSize, 0.0);
    std::vector<double> histB(histSize, 0.0);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            ++histR[red];
            ++histG[green];
            ++histB[blue];
        }

    for (int i = 1; i < histSize; ++i)
    {
        histR[i] += histR[i - 1];
        histG[i] += histG[i - 1];
        histB[i] += histB[i - 1];
    }

    for (int i = 0; i < histSize; ++i)
    {
        histR[i] /= width * height;
        histG[i] /= width * height;
        histB[i] /= width * height;
    }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            int newRed = qMin(qFloor(histSize * histR[red]), histSize - 1);
            int newGreen = qMin(qFloor(histSize * histG[green]), histSize - 1);
            int newBlue = qMin(qFloor(histSize * histB[blue]), histSize - 1);
            image.setPixel(x, y, qRgb(newRed, newGreen, newBlue));
        }
}

void V_equalization(QImage &image)
{
    int width = image.width();
    int height = image.height();

    int histSize = 256;
    std::vector<double> histV(histSize, 0.0);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QColor oldColor(image.pixel(x, y));
            int v = oldColor.value();
            ++histV[v];
        }

    for (int i = 1; i < histSize; ++i)
        histV[i] += histV[i - 1];

    for (int i = 0; i < histSize; ++i)
        histV[i] /= width * height;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QColor oldColor(image.pixel(x, y));
            int h = oldColor.hue();
            int s = oldColor.saturation();
            int v = oldColor.value();
            int newV = qMin(qFloor(histSize * histV[v]), histSize - 1);
            QColor newColor;
            newColor.setHsv(h, s, newV);
            image.setPixel(x, y, newColor.rgb());
        }
}

void Grayscale_equalization(QImage &image)
{
    int width = image.width();
    int height = image.height();

    int histSize = 256;
    std::vector<double> hist(histSize, 0.0);

    std::vector< std::vector<int> > grays(width, std::vector<int>(height) );

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
            ++hist[gray];
        }

    for (int i = 1; i < histSize; ++i)
        hist[i] += hist[i - 1];

    for (int i = 0; i < histSize; ++i)
        hist[i] /= width * height;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            int gray = grays[x][y];
            int newGray = qMin(qFloor(histSize * hist[gray]), histSize - 1);
            int delta = newGray - gray;
            int newRed = qMax(qMin(red + delta, 255), 0);
            int newGreen = qMax(qMin(green + delta, 255), 0);;
            int newBlue = qMax(qMin(blue + delta, 255), 0);;
            image.setPixel(x, y, qRgb(newRed, newGreen, newBlue));
        }
}

void MainWindow::on_actionHistogram_equalization_triggered()
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

    DialogHistogramEqualization dialog;

    if (dialog.exec() == QDialog::Rejected)
        return;

    int equalizationType = dialog.getEqualizationType();

    if (equalizationType == 1)
        RGB_equalization(image);
    else if (equalizationType == 2)
        V_equalization(image);
    else if (equalizationType == 3)
        Grayscale_equalization(image);

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionLg_triggered()
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

    int maxR = -1;
    int maxG = -1;
    int maxB = -1;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            if (red > maxR)
                maxR = red;
            if (green > maxG)
                maxG = green;
            if (blue > maxB)
                maxB = blue;
        }

    double cR = 255 / log10(1 + maxR);
    double cG = 255 / log10(1 + maxG);
    double cB = 255 / log10(1 + maxB);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = cR * log10(1 + qRed(oldColor));
            int green = cG * log10(1 + qGreen(oldColor));
            int blue = cB * log10(1 + qBlue(oldColor));
            if (red < 0)
                red = 0;
            if (red > 255)
                red = 255;
            if (green < 0)
                green = 0;
            if (green > 255)
                green = 255;
            if (blue < 0)
                blue = 0;
            if (blue > 255)
                blue = 255;
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

void MainWindow::on_actionPiecewise_linear_triggered()
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

    DialogPiecewiseLinear dialog;
    dialog.setText(pieceWiseLinearText);

    if (dialog.exec() == QDialog::Rejected)
        return;

    std::vector<double> nums = dialog.getNums();

    pieceWiseLinearText = dialog.getString();

    bool normalize = dialog.isNormalize();
    std::vector<double> csR(nums.size() / 4);
    std::vector<double> csG(nums.size() / 4);
    std::vector<double> csB(nums.size() / 4);
    if (normalize)
    {
        int maxR = -1;
        int maxG = -1;
        int maxB = -1;

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                QRgb oldColor = image.pixel(x, y);
                int red = qRed(oldColor);
                int green = qGreen(oldColor);
                int blue = qBlue(oldColor);
                if (red > maxR)
                    maxR = red;
                if (green > maxG)
                    maxG = green;
                if (blue > maxB)
                    maxB = blue;
            }

        for (unsigned int i = 0; i < nums.size(); i += 4)
        {
            csR[i/4] = 255 / (nums[i] * maxR + nums[i+1]);
            csG[i/4] = 255 / (nums[i] * maxG + nums[i+1]);
            csB[i/4] = 255 / (nums[i] * maxB + nums[i+1]);
        }
    }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            QRgb oldColor = image.pixel(x, y);
            int red = qRed(oldColor);
            int green = qGreen(oldColor);
            int blue = qBlue(oldColor);
            double cR = 1.0;
            double cG = 1.0;
            double cB = 1.0;
            for (unsigned int i = 0; i < nums.size(); i += 4)
            {
                double k = nums[i];
                double b = nums[i+1];
                int left = qFloor(nums[i+2]);
                int right = qFloor(nums[i+3]);
                if (left <= red && red <= right)
                {
                    if (normalize)
                        cR = csR[i/4];
                    red = cR * (k * red + b);
                    if (red < 0)
                        red = 0;
                    if (red > 255)
                        red = 255;
                }
                if (left <= green && green <= right)
                {
                    if (normalize)
                        cG = csG[i/4];
                    green = cG * (k * green + b);
                    if (green < 0)
                        green = 0;
                    if (green > 255)
                        green = 255;
                }
                if (left <= blue && blue <= right)
                {
                    if (normalize)
                        cB = csB[i/4];
                    blue = cB * (k * blue + b);
                    if (blue < 0)
                        blue = 0;
                    if (blue > 255)
                        blue = 255;
                }
            }
            image.setPixel(x, y, qRgb(red, green, blue));
        }

    pixmap.convertFromImage(image);

    pixmapItem_2->setPixmap(pixmap);
    scene_2->setSceneRect(QRectF(pixmap.rect()));

    calcHist(pixmap, hist_2, maxLevel_2);
    drawHist(pixmapItem_4, hist_2, maxLevel_2);
}

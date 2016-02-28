#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QFileDialog>
#include<QImage>
#include<QRgb>
#include<QtMath>

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
        pixmapItem->setPixmap(pixmap);
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
}

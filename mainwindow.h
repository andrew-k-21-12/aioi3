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

#include "resultdialog.h"

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    Ui::MainWindow *mUi;
    QGraphicsScene *mScene;
    QGraphicsPixmapItem *mPixmapItem;

    std::vector< std::vector<double> > mA;
    QList<QString> mFoundPicturesFiles;



    void resizeEvent(QResizeEvent* event)
    {
       QMainWindow::resizeEvent(event);

       mUi->graphicsView->fitInView(mUi->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
    }

    void demonstrateResults(QList<QString>& foundPicturesFiles)
    {
        if (foundPicturesFiles.size() != 0)
        {
            ResultDialog d(foundPicturesFiles, this);
            d.exec();
        }
    }



    //! В заданный вектор битов в диапазоне от from до to записывает 1, если она также есть у n на той же позиции в битовом формате.
    void writeBits(int n, QBitArray &arr, int from, int to)
    {
        for(int k = from; k <= to; ++k)
        {
            int mask =  1 << k;
            int masked_n = n & mask;
            int thebit = masked_n >> k;
            arr[k] = thebit;
        }
    }



    std::vector<int> histProj(QPixmap &pixmap)
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

    double distHistProj(std::vector<int> &hist1, std::vector<int> &hist2)
    {
        // Гистограммы могут быть разных размеров - берем размер первой.
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



    std::vector<double> histCol(QPixmap &pixmap, int size1, int size2)
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

    double distHistCol(std::vector<double> &hist1, std::vector<double> &hist2)
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
             mUi(new Ui::MainWindow)
    {
        mUi->setupUi(this);
        this->setWindowTitle(tr("Поиск по шаблону"));

        mScene = new QGraphicsScene(this);
        mPixmapItem = mScene->addPixmap(QPixmap());
        mUi->graphicsView->setScene(mScene);


        /* Заменяем n элементов вектора (первое число в параметрах)
         * на указанные во втором параметре (увеличиваем размер, если нужно). */
        mA.assign(64, std::vector<double>(64, 0)); // инициализация с 64 нулями: второе число - чем заполнять
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 64; ++j)
            {
                /* Два байта, у которых все позиции заняты нулями, кроме тех,
                 * которые совпадают с соответствующими позициями единиц в битовом формате i или j. */
                QBitArray b1(8);
                writeBits(i, b1, 0, 7);
                QBitArray b2(8);
                writeBits(j, b2, 0, 7);

                // В r будет число, образованное как сумма всех несовпадений соответствующих битов в их массивах.
                int r = 0;
                for (int k = 0; k < b1.size(); ++k)
                    r += (int) (b1[k] ^ b2[k]);

                mA[i][j] = r / 8.0;
            }

    }

    ~MainWindow()
    {
        delete mUi;
    }



private slots:

    void on_actionLoad_triggered()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Выберите изображение для поиска"),
                                                        QDir::currentPath(),
                                                        tr("Изображения (*.png *.jpg *.bmp)"));
        if (fileName.isEmpty())
        {
            mUi->statusBar->showMessage(tr("Не выбран файл для загрузки"), 3000);
            return;
        }

        QPixmap pixmap(fileName);
        if ( !pixmap.isNull() )
        {
            mPixmapItem->setPixmap(pixmap);
            mScene->setSceneRect(pixmap.rect());
            mUi->graphicsView->fitInView(pixmap.rect(), Qt::KeepAspectRatio);
        } else
            mUi->statusBar->showMessage(tr("Ошибка при загрузке изображения"), 3000);
    }

    void on_actionColorHistogram_triggered()
    {
        // Берем копию загруженного изображения для работы.
        QPixmap pixmap = mPixmapItem->pixmap().copy();
        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();

        if (width == 0 || height == 0)
        {
            mUi->statusBar->showMessage( tr("Ошибка изображения"), 3000 );
            return;
        }


        // !!!
        int threshold = 180000;
        // !!!


        // Чистим прежние результаты и получаем директорию для новых.
        mFoundPicturesFiles.clear();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Выберите директорию для поиска похожих") );
        QDir dir(dirName);
        // Для итераций по всем файлам в директории.
        QDirIterator it(dir.absolutePath(), QDir::Files);

        // Гистограмма открытого изображения.
        std::vector<int> origHist = histProj(pixmap);

        // Проходимся по всем файлам в папке.
        for (; it.hasNext(); it.next())
        {
            QString curFileName = it.filePath();

            // Строим для картинки (только!) гистограмму.
            QPixmap curPixmap(curFileName);
            if ( curPixmap.isNull() )
                continue;
            std::vector<int> curHist = histProj(curPixmap);

            // Смотрим разницу гистограмм.
            double dist = distHistProj(origHist, curHist);

            if (dist < threshold)
                mFoundPicturesFiles.append(curFileName);
        }

        QString mes = tr("Найдено изображений: ") + QString::number(mFoundPicturesFiles.size());
        mUi->statusBar->showMessage(mes, 3000);


        demonstrateResults(mFoundPicturesFiles);
    }

    void on_actionShapeHistogramDistance_triggered()
    {
        // Копируем текущую картинку.
        QPixmap pixmap = mPixmapItem->pixmap().copy();
        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();
        if (width == 0 || height == 0)
        {
            mUi->statusBar->showMessage( tr("Ой! Что-то не так с картинкой!"), 3000 );
            return;
        }


        // !!!
        double threshold = 0.8;
        // !!!


        // Чистим прошлые результаты.
        mFoundPicturesFiles.clear();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Где искать-то будем?") );
        QDir dir(dirName);
        QDirIterator it(dir.absolutePath(), QDir::Files);


        // !!!
        std::vector<double> origHist = histCol(pixmap, 20, 20);
        // !!!


        // Для всех картинок в папке смотрим гистограммы.
        for (; it.hasNext(); it.next())
        {
            QString curFileName = it.filePath();
            QPixmap curPixmap(curFileName);
            if ( curPixmap.isNull() )
                continue;


            // !!!
            std::vector<double> curHist = histCol(curPixmap, 20, 20);
            // !!!

            // !!!
            double dist = distHistCol(origHist, curHist);
            // !!!

            if (dist < threshold)
                mFoundPicturesFiles.append(curFileName);
        }

        QString mes = tr("Найдено изображений: ") + QString::number(mFoundPicturesFiles.size());
        mUi->statusBar->showMessage(mes, 3000);


        demonstrateResults(mFoundPicturesFiles);
    }

};

#endif // MAINWINDOW_H

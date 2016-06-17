#ifndef SKETCHSEARCH
#define SKETCHSEARCH

#include <QList>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QDirIterator>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "skeletonbuilder.h"

class SketchSearch
{

private:

    int CONFIG_GLOBAL_REFINE_THRESHOLD = 200;
    int CONFIG_DIFF_THRESH = 1;



    void refineImage(const cv::Mat & src, cv::Mat & result, const int& threshold)
    {
        cv::Mat matTmp = cv::Mat(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

        int coreX[3][3] = { {-1, 0, 1},
                            {-2, 0, 2},
                            {-1, 0, 1} };
        int coreY[3][3] = { {-1, -2, -1},
                            { 0,  0,  0},
                            { 1,  2,  1} };

        for (int x = 0; x < src.cols; ++x)
            for (int y = 0; y < src.rows; ++y)
            {
                int gX = 0,
                    gY = 0;

                for (int i = -1; i <= 1; ++i)
                    for (int j = -1; j <= 1; ++j)
                    {
                        int targetX = x + i;
                        int targetY = y + j;

                        if (targetX < 0 || targetX >= src.cols || targetY < 0 || targetY >= src.rows)
                            continue;

                        uchar intensity = src.at<uchar>(targetY, targetX);

                        gX += intensity * coreX[i+1][j+1];
                        gY += intensity * coreY[i+1][j+1];
                    }

                int color = qRound( qSqrt(qPow(gX, 2) + qPow(gY, 2)) );
                color = qMin(255, color);

                if (color > threshold)
                    matTmp.at<uchar>(y, x) = color;
            }


        result = matTmp.clone();
    }

public:

    void genSketch(const cv::Mat & src, cv::Mat & result)
    {
        cv::Mat matTemp = src.clone();

        // 1. Масштабируем до 64х64 и удаляем шум медианным фильтром.
        cv::resize(matTemp, matTemp, cv::Size(64, 64));
        cv::medianBlur(matTemp, matTemp, 3);

        // 2. Чистим контурное изображение.
        refineImage(matTemp, matTemp, CONFIG_GLOBAL_REFINE_THRESHOLD);
//        refineImage(matTemp, matTemp, 250);
//        cv::Mat matStrEl = cv::getStructuringElement(cv::MORPH_ELLIPSE,
//                                                     cv::Size(1, 1),
//                                                     cv::Point(0, 0));
//        cv::dilate(matTemp, matTemp, matStrEl);
//        matStrEl = cv::getStructuringElement(cv::MORPH_RECT,
//                                             cv::Size(1, 1),
//                                             cv::Point(0, 0));
//        cv::erode(matTemp, matTemp, matStrEl);
        for (int x = 0; x < matTemp.cols; ++x)
            for (int y = 0; y < matTemp.rows; ++y)
            {
                uchar intensity = matTemp.at<uchar>(y, x);

                if (intensity != 0)
                    matTemp.at<uchar>(y, x) = 255;
            }

        // 3. Строим скелет.
        SkeletonBuilder::normalizeLetter(matTemp, matTemp);


        result = matTemp.clone();
    }

    int evalDifference(cv::Mat & m1, cv::Mat & m2)
    {
        int total = 0;

        for (int xStep = 0; xStep < 16; ++xStep)
            for (int yStep = 0; yStep < 16; ++yStep)
            {
                int xShift = xStep * 4;
                int yShift = yStep * 4;

                int filled1 = 0;
                int filled2 = 0;

                for (int xInner = 0; xInner < 4; ++xInner)
                    for (int yInner = 0; yInner < 4; ++yInner)
                    {
                        if (m1.at<uchar>(yShift + yInner, xShift + yInner) != 0)
                            filled1++;
                        if (m2.at<uchar>(yShift + yInner, xShift + yInner) != 0)
                            filled2++;
                    }

                if (qAbs(filled1 - filled2) > 4)
                    total++;
            }

        return total;
    }



public:

    QList<QString> search(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;

        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        cv::Mat matTemp = cv::Mat(imgSrc.height(), imgSrc.width(), CV_8UC3, imgSrc.bits(), imgSrc.bytesPerLine());
        cvtColor(matTemp, matTemp, CV_BGR2GRAY);

        genSketch(matTemp, matTemp);

        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        for (QDirIterator it(searchDirName, namesFilterList, QDir::Files); it.hasNext(); it.next())
        {
            QString currentFilePath = it.filePath();
            QImage imgCurrent = QImage(currentFilePath);
            if (imgCurrent.isNull())
                continue;

            cv::Mat matCurrent = cv::imread(currentFilePath.toUtf8().data());
            if (matCurrent.empty())
                continue;

            cvtColor(matCurrent, matCurrent, CV_BGR2GRAY);
            genSketch(matCurrent, matCurrent);

            int difference = evalDifference(matTemp, matCurrent);

            if (difference < CONFIG_DIFF_THRESH)
                resultsList.append(currentFilePath);
        }


        return resultsList;
    }

};

#endif // SKETCHSEARCH

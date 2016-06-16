#ifndef TEXTURES
#define TEXTURES

#include <QList>
#include <QGraphicsPixmapItem>
#include <qmath.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class Textures
{

private:

    int CONFIG_INTENSITY_THRESHOLD = 128;
    qreal CONFIG_HIST_DIFF_THRESHOLD = 0.5;
    qreal CONFIG_MAGDIR_DIFF_THRESHOLD = 140 * 100;



    std::map<uint8_t, qreal> evalBinaryHist(const QImage& src)
    {
        QImage srcSmall = src.scaled(750, 750, Qt::KeepAspectRatio);

        std::map<uint8_t, qreal> result;

        int totalCount = 0;

        for (int i = 1; i < srcSmall.width() - 1; ++i)
            for (int j = 1; j < srcSmall.height() - 1; ++j)
            {
                uint8_t key = 0;

                for (int ii = -1; ii <= 1; ++ii)
                    for (int jj = -1; jj <= 1; ++jj)
                    {
                        if (ii == 0 && jj == 0)
                            continue;

                        if (qGray(srcSmall.pixel(i + ii, j + jj)) > CONFIG_INTENSITY_THRESHOLD)
                        {
                            // Считаем побитовый сдвиг:
                            // ii от 0 до 2  * 3   == 0, 3, 6
                            // jj от 0 до 2  * 1   == 0, 1, 2     0 1 2 3 4 5 6 7 8, если >= 4, то - 1
                            int shift = (ii + 1) * 3 + (jj + 1);
                            shift = (shift >= 4) ? shift - 1 : shift;

                            // Изначально маска имеет вид 00000001, потом единица при необходимости сдвигается на нужную позицию.
                            uint8_t mask = 1;
                            mask = mask << shift;

                            // В ключ ставится единица на нужной позиции.
                            key = key | mask;
                        }
                    }

                result[key] += 1;
                ++totalCount;
            }

        // Нормализуем.
        for (const auto& element : result)
            result[uint8_t(element.first)] /= totalCount;


        return result;
    }

    qreal histDiff(std::map<uint8_t, qreal>& hist1, std::map<uint8_t, qreal>& hist2)
    {
        qreal totalDiff = 0;

        for (const auto &val : hist1)
        {
            const uint8_t& key = uint8_t(val.first);
            totalDiff += qAbs(hist1[key] - hist2[key]);
        }


        return totalDiff;
    }



    std::vector<std::vector<int> > evalMagdir(const QImage& src)
    {
        cv::Mat matSrc = cv::Mat(src.height(), src.width(), CV_8UC3, (uchar*) src.bits(), src.bytesPerLine());
        // Deep copy to gray.
        cvtColor(matSrc, matSrc, CV_BGR2GRAY);

        // Temp mats and mats for abs results.
        cv::Mat gradX, gradY;
        cv::Mat absGradX, absGradY;

        int ddepth = CV_16S;
        int scale = 1;
        int delta = 0;

        // X.
        cv::Sobel(matSrc, gradX, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
        cv::convertScaleAbs(gradX, absGradY);

        // Y.
        cv::Sobel(matSrc, gradY, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
        cv::convertScaleAbs(gradY, absGradY);


        int size = absGradX.rows * absGradX.cols;
        std::vector<qreal> mag(size);
        std::vector<qreal> dir(size);

        int index = 0;

        for (auto y = 0; y < absGradX.rows; ++y)
            for (auto x = 0; x < absGradX.cols; ++x)
            {
                // Может, поменять X и Y?
                dir[index] = qSqrt(qPow(absGradX.at<uint8_t>(cv::Point(x, y)), 2) + qPow(absGradY.at<uint8_t>(cv::Point(x, y)), 2));
                mag[index] = qAtan2(absGradY.at<uint8_t>(cv::Point(x, y)), absGradX.at<uint8_t>(cv::Point(x, y)));
                ++index;
            }

        int histSize = 10;
        int maxIndex = histSize - 1;
        std::vector<int> magHist(histSize);
        std::vector<int> dirHist(histSize);

        qreal magMin = findMin(mag);
        qreal magMax = findMax(mag);
        qreal magDelta = magMax - magMin;
        qreal dirMin = findMin(dir);
        qreal dirMax = findMax(dir);
        qreal dirDelta = dirMax - dirMin;

        for (int i = 0; i < size; ++i)
        {
            int j = qMin(qRound((mag[i] - magMin) * histSize / magDelta), maxIndex);
            ++magHist[j];

            int k = qMin(qRound((dir[i] - dirMin) * histSize / dirDelta), maxIndex);
            ++dirHist[k];
        }


        std::vector<std::vector<int> > result;
        result.push_back(magHist);
        result.push_back(dirHist);

        return result;
    }

    qreal findMin(std::vector<qreal>& src)
    {
        qreal min = INT_MAX;

        for (unsigned long i = 0; i < src.size(); ++i)
        {
            min = qMin(min, src[i]);
        }

        return min;
    }

    qreal findMax(std::vector<qreal>& src)
    {
        qreal max = - (INT_MAX - 1);

        for (unsigned long i = 0; i < src.size(); ++i)
        {
            max = qMax(max, src[i]);
        }

        return max;
    }

    qreal magdirDiff(std::vector<std::vector<int> >& magdir1, std::vector<std::vector<int> >& magdir2)
    {
        qreal totalMag = 0;
        qreal totalDir = 0;

        std::vector<int>& mag1 = magdir1.at(0);
        std::vector<int>& mag2 = magdir2.at(0);

        std::vector<int>& dir1 = magdir1.at(1);
        std::vector<int>& dir2 = magdir2.at(1);

        for (unsigned long i = 0; i < mag1.size(); ++i)
            totalMag += qAbs(mag1[i] - mag2[i]);

        for (unsigned long i = 0; i < dir1.size(); ++i)
            totalDir += qAbs(dir1[i] - dir2[i]);


        return totalMag + totalDir;
    }



public:

    QList<QString> binarySearch(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;

        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        std::map<uint8_t, qreal> histSrc = evalBinaryHist(imgSrc);

        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        for (QDirIterator it(searchDirName, namesFilterList, QDir::Files); it.hasNext(); it.next())
        {
            QString currentFilePath = it.filePath();
            QImage imgCurrent = QImage(currentFilePath);
            if (imgCurrent.isNull())
                continue;

            std::map<uint8_t, qreal> histCurr = evalBinaryHist(imgCurrent);
            qreal difference = histDiff(histSrc, histCurr);

            if (difference < CONFIG_HIST_DIFF_THRESHOLD)
                resultsList.append(currentFilePath);
        }


        return resultsList;
    }

    QList<QString> magdirSearch(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;

        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        std::vector<std::vector<int> > magdirSrc = evalMagdir(imgSrc);

        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        for (QDirIterator it(searchDirName, namesFilterList, QDir::Files); it.hasNext(); it.next())
        {
            QString currentFilePath = it.filePath();
            QImage imgCurrent = QImage(currentFilePath);
            if (imgCurrent.isNull())
                continue;

            std::vector<std::vector<int> > magdirCurr = evalMagdir(imgCurrent);
            qreal difference = magdirDiff(magdirSrc, magdirCurr);

            if (difference < CONFIG_MAGDIR_DIFF_THRESHOLD)
                resultsList.append(currentFilePath);
        }


        return resultsList;
    }

};

#endif // TEXTURES

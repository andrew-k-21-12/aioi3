#ifndef TANGENTHISTSEARCH
#define TANGENTHISTSEARCH

#include <map>

#include <QString>
#include <QList>
#include <QDir>
#include <QGraphicsPixmapItem>
#include <QDirIterator>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QDebug>

//! Provides an engine to search images using histograms of contours' tangents.
class TangentHistSearch
{

private:

    qreal CONFIG_THRESHOLD = 0.095;
    qreal CONFIG_MAX_SCALE_SIDE = 750;



public:

    QList<QString> run(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;


        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        // Это прямая копия imgSrc!
        cv::Mat matSrc(imgSrc.height(), imgSrc.width(), CV_8UC3, imgSrc.bits(), imgSrc.bytesPerLine());
        std::map<int, qreal> histSrc = evalHist(matSrc);


        // Фильтры, чтобы просматривать только изображения.
        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        // Проходимся по всем файлам в папке.
        for (QDirIterator it(searchDirName, namesFilterList, QDir::Files); it.hasNext(); it.next())
        {
            QString currentFilePath = it.filePath();

            cv::Mat matCurrent = cv::imread(currentFilePath.toUtf8().data());

            if (matCurrent.empty())
                continue;

            std::map<int, qreal> histCurr = evalHist(matCurrent);

            qreal difference = histDiff(histSrc, histCurr);

            if (difference < CONFIG_THRESHOLD)
                resultsList.append(currentFilePath);
        }


        return resultsList;
    }



private:

    std::map<int, qreal> evalHist(cv::Mat& src)
    {
        // 56 возможных касательных, у каждой свой индекс.
        std::map<int, qreal> result;
        int totalCount = 0;

        std::vector<std::vector<cv::Point> > contours = extractContours(src);
        for (unsigned long i = 0; i < contours.size(); ++i)
        {
            std::vector<cv::Point>& currentContour = contours.at(i);
            addTransitionsForContour(currentContour, totalCount, result);

            std::vector<cv::Point> loopContour;
            loopContour.push_back(currentContour[0]);
            loopContour.push_back(currentContour[currentContour.size() - 1]);
            addTransitionsForContour(loopContour, totalCount, result);
        }

        for (const auto &pair : result)
        {
            result[pair.first] /= totalCount;
        }

//        for (std::map<int, qreal>::iterator it = result.begin(); it != result.end(); ++it)
//        {
//            const int& key(it->first);
//            result[key] /= totalCount;
//        }

        return result;
    }

    qreal histDiff(std::map<int, qreal>& hist1, std::map<int, qreal>& hist2)
    {
        qreal totalDiff = 0;

        for (const auto &pair : hist1)
        {
            totalDiff += qAbs(hist1[pair.first] - hist2[pair.first]);
        }

//        for (std::map<int, qreal>::iterator it = hist1.begin(); it != hist1.end(); ++it)
//        {
//            const int& key(it->first);
//            totalDiff += qAbs(hist1[key] - hist2[key]);
//        }

        return totalDiff;
    }

    std::vector<std::vector<cv::Point> > extractContours(cv::Mat& matForContoursExtraction)
    {
        // Convert image to gray and blur it.
        // Эта операция отсоединяет прежние адреса - получается глубокая копия.
        cvtColor(matForContoursExtraction, matForContoursExtraction, CV_BGR2GRAY);

        // Уменьшаем для ускорения.
        qreal scale = CONFIG_MAX_SCALE_SIDE / (qreal) qMax(matForContoursExtraction.rows, matForContoursExtraction.cols);
        cv::resize(matForContoursExtraction, matForContoursExtraction,
                   cv::Size(matForContoursExtraction.cols * scale, matForContoursExtraction.rows * scale),
                   0, 0, cv::INTER_CUBIC);

        // Размываем.
        cv::blur(matForContoursExtraction, matForContoursExtraction, cv::Size(3, 3));

        // Находим границы (некоторые точки могут не соединяться друг с другом).
        Canny(matForContoursExtraction, matForContoursExtraction, 1, 100, 3);

        // Find contours. Теперь точки упорядочены в связанные контуры.
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        findContours(matForContoursExtraction, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));


        return contours;
    }

    void addTransitionsForContour(std::vector<cv::Point>& contour, int& total, std::map<int, qreal>& transitions)
    {
        fillGapsInContour(contour);

        for (unsigned long k = 1; k < contour.size() - 1; ++k)
        {
            cv::Point& pPrev = contour.at(k - 1);
            cv::Point& pCurr = contour.at(k);
            cv::Point& pNext = contour.at(k + 1);

            // Числа от 0 до 2 включительно.
            int xPrevDiff = 1 + pCurr.x - pPrev.x;
            int yPrevDiff = 1 + pCurr.y - pPrev.y;
            int xNextDiff = 1 + pCurr.x - pNext.x;
            int yNextDiff = 1 + pCurr.y - pNext.y;

            int xMinIndex = qMin(xPrevDiff, xNextDiff);
            int yMinIndex = qMin(yPrevDiff, yNextDiff);
            int xMaxIndex = qMax(xPrevDiff, xNextDiff);
            int yMaxIndex = qMax(yPrevDiff, yNextDiff);

            int index = xMinIndex + yMinIndex * 10 + xMaxIndex * 100 + yMaxIndex * 1000;
            // int index = xPrevDiff + yPrevDiff * 10 + xNextDiff * 100 + yNextDiff * 1000;

            transitions[index] += 1;

            ++total;
        }
    }

    void fillGapsInContour(std::vector<cv::Point>& contourPoints)
    {
        if (contourPoints.size() == 0)
            return;

        for (unsigned long i = 1; i < contourPoints.size(); ++i)
        {
            cv::Point& previousPoint = contourPoints.at(i - 1);
            cv::Point& currentPoint = contourPoints.at(i);
            if (qAbs(currentPoint.x - previousPoint.x) >= 2 || qAbs(currentPoint.y - previousPoint.y) >= 2)
            {
                std::vector<cv::Point> innerPoints;
                pushInnerPointRecursively(currentPoint, previousPoint, innerPoints);

                int targetX = previousPoint.x;
                int targetY = previousPoint.y;
                while (innerPoints.size() != 0)
                {
                    int minDiff = INT_MAX;
                    int indexClosest = -1;

                    for (unsigned long j = 0; j < innerPoints.size(); ++j)
                    {
                        cv::Point& innerPoint = innerPoints.at(j);
                        int diff = qAbs(innerPoint.x - targetX) + qAbs(innerPoint.y - targetY);
                        if (diff < minDiff)
                        {
                            minDiff = diff;
                            indexClosest = j;
                        }
                    }

                    cv::Point& closestPoint = innerPoints.at(indexClosest);
                    targetX = closestPoint.x;
                    targetY = closestPoint.y;

                    contourPoints.insert(contourPoints.begin() + i, closestPoint);

                    innerPoints.erase(innerPoints.begin() + indexClosest);

                    ++i;
                }
            }
        }
    }

    void pushInnerPointRecursively(const cv::Point& p1, const cv::Point& p2, std::vector<cv::Point>& points)
    {
        int x = qRound( (p1.x + p2.x) / (qreal) 2 );
        int y = qRound( (p1.y + p2.y) / (qreal) 2 );

        if ((x == p1.x && y == p1.y) || (x == p2.x && y == p2.y))
            return;

        cv::Point pCenter(x, y);

        points.push_back(pCenter);

        pushInnerPointRecursively(p1, pCenter, points);
        pushInnerPointRecursively(p2, pCenter, points);
    }

};

#endif // TANGENTHISTSEARCH

#ifndef TANGENTHISTSEARCH
#define TANGENTHISTSEARCH

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

public:

    QList<QString> run(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;


        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        // Это прямая копия imgSrc!
        cv::Mat matSrc(imgSrc.height(), imgSrc.width(), CV_8UC3, imgSrc.bits(), imgSrc.bytesPerLine());
        std::vector<qreal> histSrc = evalHist(matSrc);


        // Фильтры, чтобы просматривать только изображения.
        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        // Проходимся по всем файлам в папке.
        for (QDirIterator it(searchDirName, namesFilterList, QDir::Files); it.hasNext(); it.next())
        {
            QString currentFilePath = it.filePath();





            resultsList.append(currentFilePath);
        }


        return resultsList;
    }



private:

    std::vector<std::vector<cv::Point> > extractContours(cv::Mat& matForContoursExtraction)
    {
        // Convert image to gray and blur it.
        // Эта операция отсоединяет прежние адреса - получается глубокая копия.
        cvtColor(matForContoursExtraction, matForContoursExtraction, CV_BGR2GRAY);
        cv::blur(matForContoursExtraction, matForContoursExtraction, cv::Size(3, 3));

        // Находим границы (некоторые точки могут не соединяться друг с другом).
        Canny(matForContoursExtraction, matForContoursExtraction, 1, 100, 3);

        // Find contours. Теперь точки упорядочены в связанные контуры.
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        findContours(matForContoursExtraction, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));


        return contours;
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

    std::vector<qreal> evalHist(cv::Mat& src)
    {
        // !!!
        std::vector<std::vector<cv::Point> > contours = extractContours(src);

        std::vector<cv::Point> contour = contours.at(0);

        fillGapsInContour(contour);

        cv::Mat drawing = cv::Mat::zeros( src.size(), CV_8UC3 );
        for (unsigned long i = 1; i < contour.size(); ++i)
        {
            cv::Point p1 = contour.at(i-1);
            cv::Point p2 = contour.at(i);
            cv::line(drawing, p1, p2, cv::Scalar(255, 255, 255));
        }

        cv::line(drawing, contour.at(0), contour.at(contour.size() - 1), cv::Scalar(255, 255, 255));

        cv::imshow("ke", drawing);

//        std::vector<cv::Point>& kek = ccc.at(0);
//        for (unsigned long i = 0; i < kek.size(); ++i)
//        {
//            cv::Point& p = kek.at(i);
//            qDebug() << "before " << p.x << " " << p.y;
//        }
//        fillGapsInContour(kek);
//        for (unsigned long i = 0; i < kek.size(); ++i)
//        {
//            cv::Point& p = kek.at(i);
//            qDebug() << "after " << p.x << " " << p.y;
//        }

        return std::vector<qreal>();
    }

};

#endif // TANGENTHISTSEARCH

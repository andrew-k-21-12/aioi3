#ifndef TEXTURES
#define TEXTURES

#include <QList>
#include <QGraphicsPixmapItem>

class Textures
{

private:

    int CONFIG_INTENSITY_THRESHOLD = 128;
    qreal CONFIG_HIST_DIFF_THRESHOLD = 0.5;



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



public:

    QList<QString> run(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;

        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
        std::map<uint8_t, qreal> histSrc = evalBinaryHist(imgSrc);

        // Фильтры, чтобы просматривать только изображения.
        QStringList namesFilterList;
        namesFilterList.append("*.png");
        namesFilterList.append("*.jpg");

        // Проходимся по всем файлам в папке.
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

};

#endif // TEXTURES

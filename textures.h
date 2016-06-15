#ifndef TEXTURES
#define TEXTURES

#include <QList>
#include <QGraphicsPixmapItem>

class Textures
{

private:

    int CONFIG_INTENSITY_THRESHOLD = 128;



    std::map<uint8_t, qreal> evalBinaryHist(const QImage& src)
    {

    }



public:

    QList<QString> run(const QString & searchDirName, const QGraphicsPixmapItem * templateToSearch)
    {
        QList<QString> resultsList;

        QImage imgSrc = templateToSearch->pixmap().toImage().convertToFormat(QImage::Format_RGB888);

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

//            qreal difference = histDiff(histSrc, histCurr);

//            if (difference < CONFIG_THRESHOLD)
//                resultsList.append(currentFilePath);
        }


        return resultsList;
    }

};

#endif // TEXTURES

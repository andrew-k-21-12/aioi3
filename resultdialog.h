#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include "ui_resultdialog.h"

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog
{
    Q_OBJECT

private:

    Ui::ResultDialog *ui;



public:

    explicit ResultDialog(QList<QString>& foundPicturesFiles, QWidget *parent = 0) :
        QDialog(parent),
        ui(new Ui::ResultDialog)
    {
        ui->setupUi(this);

        setWindowTitle(tr("Найденные похожие изображения"));


        ui->listWidget->setViewMode(QListWidget::IconMode);
        ui->listWidget->setIconSize(QSize(300, 300));
        ui->listWidget->setResizeMode(QListWidget::Adjust);

        for (int i = 0; i < foundPicturesFiles.size(); ++i)
            ui->listWidget->addItem(new QListWidgetItem(QIcon(foundPicturesFiles[i]), foundPicturesFiles[i]));
    }

    ~ResultDialog()
    {
        delete ui;
    }

};

#endif // RESULTDIALOG_H

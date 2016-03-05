#include "dialogbasecolorcorrection.h"
#include "ui_dialogbasecolorcorrection.h"

#include<QMouseEvent>
#include<QFileDialog>
#include<QColorDialog>

#include<QDebug>

DialogBaseColorCorrection::DialogBaseColorCorrection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBaseColorCorrection)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    pixmapItem = scene->addPixmap(QPixmap());
    ui->graphicsView->setScene(scene);
    scene_2 = new QGraphicsScene(this);
    pixmapItem_2 = scene_2->addPixmap(QPixmap());
    ui->graphicsView_2->setScene(scene_2);
    ui->graphicsView->installEventFilter(this);
    ui->graphicsView_2->installEventFilter(this);
}

DialogBaseColorCorrection::~DialogBaseColorCorrection()
{
    delete scene_2;
    delete scene;
    delete ui;
}

bool DialogBaseColorCorrection::eventFilter(QObject *object, QEvent *event)
{
//    if (object == ui->graphicsView && event->type() == QEvent::Paint)
//        ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
//    if (object == ui->graphicsView_2 && event->type() == QEvent::Paint)
//        ui->graphicsView_2->fitInView(pixmapItem_2, Qt::KeepAspectRatio);
    if (object == ui->graphicsView && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPointF pos = ui->graphicsView->mapToScene(mouseEvent->pos());
        int x = pos.x();
        int y = pos.y();
        if (pixmapItem->pixmap().rect().contains(x, y))
        {
            QColor color(pixmapItem->pixmap().toImage().pixel(x,y));
            ui->toolButton->setStyleSheet("background-color: " + color.name());
        }
    }
    if (object == ui->graphicsView_2 && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPointF pos = ui->graphicsView_2->mapToScene(mouseEvent->pos());
        int x = pos.x();
        int y = pos.y();
        if (pixmapItem_2->pixmap().rect().contains(x, y))
        {
            QColor color(pixmapItem_2->pixmap().toImage().pixel(x,y));
            ui->toolButton_2->setStyleSheet("background-color: " + color.name());
        }
    }
    return false;
}

void DialogBaseColorCorrection::setPixmap(QPixmap &pixmap)
{
    pixmapItem->setPixmap(pixmap);
    scene->setSceneRect(QRectF(pixmap.rect()));
    scene_2->setSceneRect(QRectF(pixmap.rect()));
    QColor color(pixmapItem->pixmap().toImage().pixel(0,0));
    ui->toolButton->setStyleSheet("background-color: " + color.name());
    ui->toolButton_2->setStyleSheet(ui->toolButton->styleSheet());
}

QColor DialogBaseColorCorrection::sourceColor()
{
    QString styleSheet = ui->toolButton->styleSheet();
    QColor color(styleSheet.right(styleSheet.size() - styleSheet.indexOf("#")));
    return color;
}

QColor DialogBaseColorCorrection::destinationColor()
{
    QString styleSheet = ui->toolButton_2->styleSheet();
    QColor color(styleSheet.right(styleSheet.size() - styleSheet.indexOf("#")));
    return color;
}

void DialogBaseColorCorrection::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"));
    QPixmap pixmap(fileName);
    if ( ! pixmap.isNull() )
    {
        pixmapItem_2->setPixmap(pixmap);
        scene_2->setSceneRect(QRectF(pixmap.rect()));
    }
}

void DialogBaseColorCorrection::on_toolButton_2_clicked()
{
    QColor color = QColorDialog::getColor(destinationColor(), this, tr("Choose color"), QColorDialog::ShowAlphaChannel);
    if ( color.isValid() )
        ui->toolButton_2->setStyleSheet("background-color: " + color.name());
}

void DialogBaseColorCorrection::on_toolButton_clicked()
{
    QColor color = QColorDialog::getColor(destinationColor(), this, tr("Choose color"), QColorDialog::ShowAlphaChannel);
    if ( color.isValid() )
        ui->toolButton->setStyleSheet("background-color: " + color.name());
}

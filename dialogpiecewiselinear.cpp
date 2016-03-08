#include "dialogpiecewiselinear.h"
#include "ui_dialogpiecewiselinear.h"

#include <QtMath>
#include <vector>
#include <list>

#include <QDebug>

DialogPiecewiseLinear::DialogPiecewiseLinear(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPiecewiseLinear)
{
    ui->setupUi(this);
}

DialogPiecewiseLinear::~DialogPiecewiseLinear()
{
    delete ui;
}

const std::vector<double>& DialogPiecewiseLinear::getNums()
{
    return nums;
}

QString DialogPiecewiseLinear::getString()
{
    return ui->textEdit->toPlainText();
}

void DialogPiecewiseLinear::setText(QString text)
{
    ui->textEdit->setPlainText(text);
}

bool DialogPiecewiseLinear::calcRange()
{
    std::list< std::pair<int, int> > range;
    for (unsigned int i = 2; i < nums.size(); i += 4)
    {
        int left = qFloor(nums[i]);
        int right = qFloor(nums[i+1]);
        if (left > right || left < 0 || right < 0 || left > 255 || right > 255)
            return false;
        for (std::list< std::pair<int, int> >::iterator it = range.begin(); it != range.end(); ++it)
        {
            int curLeft = it->first;
            int curRight = it->second;
            if ( (left <= curLeft && right >= curLeft) ||
                 (left <= curRight && right >= curRight) ||
                 (left >= curLeft && right <= curRight) ||
                 (left <= curLeft && right >= curRight) )
                return false;
        }
        range.push_back( std::make_pair(left, right) );
    }
    return true;
}

bool DialogPiecewiseLinear::isValidFormat()
{
    nums.clear();
    QString plainText = ui->textEdit->toPlainText();
    QStringList lines = plainText.split("\n");
    for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
    {
        QString curLine = *it;
        QStringList elems = curLine.split(";");
        if (elems.size() != 4)
            return false;
        for (QStringList::Iterator it = elems.begin(); it != elems.end(); ++it)
        {
            QString curElem = *it;
            bool ok;
            double num = curElem.toDouble(&ok);
            if ( ! ok )
                return false;
            nums.push_back(num);
        }
    }
    if ( ! calcRange() )
        return false;
    return true;
}

void DialogPiecewiseLinear::on_textEdit_textChanged()
{
    if (isValidFormat())
    {
        ui->buttonBox->setEnabled(true);
        ui->label->setVisible(false);
    }
    else
    {
        ui->buttonBox->setEnabled(false);
        ui->label->setVisible(true);
    }
}

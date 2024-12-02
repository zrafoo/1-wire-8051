#include "chartdata.h"

ChartData::ChartData(QObject *parent) : QObject(parent)
{
}

QVariantList ChartData::data() const
{
    return m_data;
}

void ChartData::appendData(double value)
{
    m_data.append(value);
    emit dataChanged();
}

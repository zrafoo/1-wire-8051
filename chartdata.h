#ifndef CHARTDATA_H
#define CHARTDATA_H

#include <QObject>
#include <QVariantList>

class ChartData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList data READ data NOTIFY dataChanged)
public:
    explicit ChartData(QObject *parent = nullptr);

    QVariantList data() const;

    void appendData(double value);

signals:
    void dataChanged();

private:
    QVariantList m_data;
};

#endif // CHARTDATA_H

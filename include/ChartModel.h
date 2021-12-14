//
// Created by Jacopo Gasparetto on 07/12/21.
//

#ifndef FORTRESS_CHARTMODEL_H
#define FORTRESS_CHARTMODEL_H

#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QtQuick/QQuickView>
#include <QtCore/QObject>
#include <QtCore/QtMath>

#include "SharedParams.h"

class ChartModel : public QObject {
Q_OBJECT
    Q_PROPERTY(int plotWindowSize READ plotWindowSize() CONSTANT)
    Q_PROPERTY(int showADCValues READ showADCValues() WRITE showADCValues())

private:
    // The current index on the X axis
    int m_dataXIndex{ 0 };
    // The total time ticks (number of readings received);
    int m_t{ 0 };
    // Store the entire data as (n_channels x plotWindowSize)
    QList<QList<QPointF>> m_chartData;
    // Store the entire data as (n_channels x plotWindowSize)
    QList<QList<QPointF>> m_chartCurrentData;

    // Last n_channels points received to display as gauge
    ADCReadings_t m_chLastValues{};
    CurrentReadings_t m_chLastCurrentValues{};
    // The current n_channels min/max values to auto rescale Y axis
    ADCReadings_t m_chMinValues{};
    CurrentReadings_t m_chMinCurrentValues{};
    ADCReadings_t m_chMaxValues{};
    CurrentReadings_t m_chMaxCurrentValues{};
    // The n_channels total cumulative sum to display as gauge
    ADCReadings_t m_chTotalSums{};
    CurrentReadings_t m_chTotalCurrentSums{};

    bool m_showADCValues = false;

private:
    static inline auto compareFunction = [](const QPointF &p1, const QPointF &p2) { return p1.y() < p2.y(); };

public:
    explicit ChartModel(QObject *parent = nullptr);

    void insertReadings(const ADCReadings_t &rawReadings, const CurrentReadings_t &currentReadings);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void
    updatePlotSeries(QAbstractSeries *qtQuickLeftSeries, QAbstractSeries *qtQuickRightSeries, int channel);

    // Accessors
    [[nodiscard]] QList<QPointF> getSeries() const;

    [[nodiscard]] static int plotWindowSize();

    [[nodiscard]] bool showADCValues() const;

    void showADCValues(bool show);


    // Listen for events
public slots:

    [[nodiscard]] double getLastChannelValue(int channel) const;

    [[nodiscard]] double getMinChannelValue(int channel) const;

    [[nodiscard]] double getMaxChannelValue(int channel) const;

    [[nodiscard]] double getChannelTotalSum(uint8_t channel) const;

private:

    void generatePlotSeries(int n_channels, int length);

};

#endif //FORTRESS_CHARTMODEL_H

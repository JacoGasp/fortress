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

private:
    // The current index on the X axis
    int m_dataXIndex{ 0 };
    // The total time ticks (number of readings received);
    int m_t{ 0 };
    // Store the entire data as (n_channels x plotWindowSize)
    QList<QList<QPointF>> m_chartData;

    // Last n_channels points received to display as gauge
    std::array<uint16_t, SharedParams::n_channels> m_chLastValues{};
    // The current n_channels max values to auto rescale Y axis
    std::array<uint16_t, SharedParams::n_channels> m_chMaxValues{};
    // The n_channels total cumulative sum to display as gauge
    std::array<int, SharedParams::n_channels> m_chTotalSums{};

public:
    explicit ChartModel(QObject *parent = nullptr);

    void insertReadings(const std::array<uint16_t, SharedParams::n_channels> &readings);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void updatePlotSeries(QAbstractSeries *qtQuickLeftSeries, QAbstractSeries *qtQuickRightSeries, int channel);

    // Accessors
    [[nodiscard]] QList<QPointF> getSeries() const;

    [[nodiscard]] static int plotWindowSize();

    // Listen for events
public slots:

    [[nodiscard]] double getLastChannelValue(int channel) const;

    [[nodiscard]] double getMaxChannelValue(int channel) const;

    [[nodiscard]] double getChannelTotalSum(uint8_t channel) const;

private:

    void generatePlotSeries(int n_channels, int length);

};

#endif //FORTRESS_CHARTMODEL_H

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
    Q_PROPERTY(int showDifferentialValues READ showDifferentialValues() WRITE showDifferentialValues())

private:
    // The current index on the X axis
    int m_dataXIndex{ 0 };
    // The total time ticks (number of readings received);
    int m_t{ 0 };
    // Store the entire data as (n_channels x plotWindowSize)
    QList<QList<QPointF>> m_chartData;
    // Store the entire data as (n_channels x plotWindowSize)
    QList<QList<QPointF>> m_chartDifferentialData;

    // Last n_channels points received to display as gauge
    std::array<int, SharedParams::n_channels> m_chLastValues{};
    std::array<int, SharedParams::n_channels> m_chLastDifferentialValues{};
    // The current n_channels min/max values to auto rescale Y axis
    const std::array<int, SharedParams::n_channels> m_chMinValues{};
    std::array<int, SharedParams::n_channels> m_chMinDifferentialValues{};
    std::array<int, SharedParams::n_channels> m_chMaxValues{};
    std::array<int, SharedParams::n_channels> m_chMaxDifferentialValues{};
    // The n_channels total cumulative sum to display as gauge
    std::array<int, SharedParams::n_channels> m_chTotalSums{};
    std::array<int, SharedParams::n_channels> m_chTotalDifferentialSums{};

    bool m_showDifferentialValues = true;

private:
    static inline auto compareFunction = [](const QPointF &p1, const QPointF &p2) { return p1.y() < p2.y(); };

public:
    explicit ChartModel(QObject *parent = nullptr);

    void insertReadings(const std::array<uint16_t, SharedParams::n_channels> &readings);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void updatePlotSeries(QAbstractSeries *qtQuickLeftSeries, QAbstractSeries *qtQuickRightSeries, int channel);

    // Accessors
    [[nodiscard]] QList<QPointF> getSeries() const;

    [[nodiscard]] static int plotWindowSize();

    [[nodiscard]] bool showDifferentialValues() const;

    void showDifferentialValues(bool show);

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

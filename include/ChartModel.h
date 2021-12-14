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
    std::array<int, SharedParams::n_channels> m_chLastValues{};
    std::array<double, SharedParams::n_channels> m_chLastCurrentValues{};
    // The current n_channels min/max values to auto rescale Y axis
    const std::array<int, SharedParams::n_channels> m_chMinValues{};
    std::array<double, SharedParams::n_channels> m_chMinCurrentValues{};
    std::array<int, SharedParams::n_channels> m_chMaxValues{};
    std::array<double, SharedParams::n_channels> m_chMaxCurrentValues{};
    // The n_channels total cumulative sum to display as gauge
    std::array<int, SharedParams::n_channels> m_chTotalSums{};
    std::array<double, SharedParams::n_channels> m_chTotalCurrentSums{};

    bool m_showADCValues = false;

private:
    static inline auto compareFunction = [](const QPointF &p1, const QPointF &p2) { return p1.y() < p2.y(); };

    static inline auto computeCurrentFromADC = [](int current, int prev, uint32_t delta_t) {
        return -static_cast<double>(current - prev) / static_cast<double>(SharedParams::kADCMaxVal) *
               SharedParams::kAmplifierFeedback *
               SharedParams::kIntegratorCapacitance / static_cast<double>(delta_t * 1e6);
    };


public:
    explicit ChartModel(QObject *parent = nullptr);

    void insertReadings(const std::array<uint16_t, SharedParams::n_channels> &readings, uint32_t deltaTime);

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

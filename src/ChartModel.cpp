//
// Created by Jacopo Gasparetto on 07/12/21.
//

#include <ChartModel.h>
#include <iostream>

ChartModel::ChartModel(QObject *parent) : QObject(parent) {
    generatePlotSeries(SharedParams::n_channels, SharedParams::plotWindowSizeInPoint);
}

void ChartModel::clearData() {
    for (auto &ch: m_chartData)
        ch.clear();

    for (int ch = 0; ch < SharedParams::n_channels; ++ch) {
        m_chartData[ch].clear();
        m_chartDifferentialData[ch].clear();
    }

    m_chartData.clear();
    m_chartDifferentialData.clear();
    m_t = m_dataXIndex = 0;
    m_chTotalSums = {};
    m_chLastValues = {};
    generatePlotSeries(SharedParams::n_channels, SharedParams::plotWindowSizeInPoint);
}

QList<QPointF> ChartModel::getSeries() const {
    return m_chartData.at(0);
}

int ChartModel::plotWindowSize() {
    return SharedParams::plotWindowSizeInPoint;
}

bool ChartModel::showDifferentialValues() const {
    return m_showDifferentialValues;
}

void ChartModel::showDifferentialValues(bool show) {
    m_showDifferentialValues = show;
    clearData();
}


void ChartModel::insertReadings(const std::array<uint16_t, SharedParams::n_channels> &readings) {
    m_dataXIndex = m_t % SharedParams::plotWindowSizeInPoint;

    for (int ch = 0; ch < SharedParams::n_channels; ++ch) {
        auto chSeries = &m_chartData[ch];
        auto chSeriesDiff = &m_chartDifferentialData[ch];

        int lastReading = m_chLastValues[ch];
        int newReading = readings[ch];

        // The integrator has been reset.
//        if (newReading > SharedParams::integratorThreshold) {
//            newReading -= SharedParams::integratorThreshold;
//            lastReading -= SharedParams::integratorThreshold;
//        }

        auto newDifferentialReading = newReading - lastReading;

        // Set the current readings as last the last value
        m_chLastValues[ch] = newReading;
        m_chLastDifferentialValues[ch] = newDifferentialReading;

        // Swipe from left to right and refresh the circular buffer at current position
        double x{ static_cast<double>(m_dataXIndex) };
        chSeries->replace(m_dataXIndex, QPointF{ x, static_cast<double>(newReading) });
        chSeriesDiff->replace(m_dataXIndex, QPointF{ x, static_cast<double>(newDifferentialReading) });

        // Adjust the min/max values to auto-scale the plot
        auto maxValue = *std::max_element(chSeries->begin(), chSeries->end(), compareFunction);
        auto minMaxDiffValues = std::minmax_element(chSeriesDiff->begin(), chSeriesDiff->end(), compareFunction);

        m_chMaxValues[ch] = static_cast<int>(maxValue.y());
        m_chMinDifferentialValues[ch] = static_cast<int>(minMaxDiffValues.first->y());
        m_chMaxDifferentialValues[ch] = static_cast<int>(minMaxDiffValues.second->y());
        m_chTotalSums[ch] += newReading;
        m_chTotalDifferentialSums[ch] += newDifferentialReading;
    }
    ++m_t;
}

// Slots
void ChartModel::generatePlotSeries(int n_channels, int length) {
    for (int ch = 0; ch < n_channels; ++ch) {
        QList<QPointF> points{};
        points.reserve(length);

        for (int i = 0; i < length; ++i)
            points.push_front(QPointF{});

        m_chartData.append(points);
        m_chartDifferentialData.append(points);
    }
}

double ChartModel::getLastChannelValue(int channel) const {
    return m_showDifferentialValues ? m_chLastDifferentialValues[channel] : m_chLastValues[channel];
}

double ChartModel::getMinChannelValue(int channel) const {
    return m_showDifferentialValues ? m_chMinDifferentialValues[channel] : m_chMinValues[channel];
}

double ChartModel::getMaxChannelValue(int channel) const {
    return m_showDifferentialValues ? m_chMaxDifferentialValues[channel] : m_chMaxValues[channel];
}

double ChartModel::getChannelTotalSum(uint8_t channel) const {
    return m_showDifferentialValues ? m_chTotalDifferentialSums[channel] :  m_chTotalSums[channel];
}


void ChartModel::updatePlotSeries(
        QAbstractSeries *qtQuickLeftSeries,
        QAbstractSeries *qtQuickRightSeries,
        int channel
) {
    if (qtQuickLeftSeries && qtQuickRightSeries) {
        auto *xyQtQuickLeftSeries = dynamic_cast<QXYSeries *>(qtQuickLeftSeries);
        auto *xyQtQuickRightSeries = dynamic_cast<QXYSeries *>(qtQuickRightSeries);
        auto channelData = m_showDifferentialValues ? &m_chartDifferentialData[channel] : &m_chartData[channel];

        assert(channelData->length() == SharedParams::plotWindowSizeInPoint);

        auto leftSeries = QList(channelData->begin(), channelData->begin() + m_dataXIndex);
        xyQtQuickLeftSeries->replace(leftSeries);

        // FIXME: to prevent glitches the first time the we span from left to right (right series is empty) we use
        // as workaround m_data_idx + 1, thus we need a sanity check to prevent out of bound. This check is heavy and
        // should be removed
        if (m_dataXIndex < SharedParams::plotWindowSizeInPoint - 2) {
            auto rightSeries = QList(channelData->begin() + m_dataXIndex + 1, channelData->end());
            xyQtQuickRightSeries->replace(rightSeries);
        }
    }
}

//
// Created by Jacopo Gasparetto on 07/12/21.
//

#include <ChartModel.h>

ChartModel::ChartModel(QObject *parent) : QObject(parent) {
    generatePlotSeries(SharedParams::n_channels, SharedParams::plotWindowSizeInPoint);
}

void ChartModel::clearData() {
    for (auto &ch: m_chartData)
        ch.clear();

    m_chartData.clear();
    m_t = m_dataXIndex = 0;
    m_chTotalSums = { 0, 0, 0, 0 };
    m_chLastValues = { 0, 0, 0, 0 };
    generatePlotSeries(SharedParams::n_channels, SharedParams::plotWindowSizeInPoint);
}

QList<QPointF> ChartModel::getSeries() const {
    return m_chartData.at(0);
}

int ChartModel::plotWindowSize() {
    return SharedParams::plotWindowSizeInPoint;
}


void ChartModel::insertReadings(const std::array<uint16_t, SharedParams::n_channels> &readings) {
    m_dataXIndex = m_t % SharedParams::plotWindowSizeInPoint;


    for (int ch = 0; ch < SharedParams::n_channels; ++ch) {
        auto chSeries = &m_chartData[ch];

        // Compute the new max
        if (readings[ch] > m_chMaxValues[ch]) m_chMaxValues[ch] = readings[ch];

        // Set the current readings as last the last value
        m_chLastValues[ch] = readings[ch];

        // Swipe from left to right and refresh the circular buffer
        double x{ static_cast<double>(m_dataXIndex % (SharedParams::plotWindowSizeInPoint)) };
        double y{ static_cast<double>(readings[ch]) };
        chSeries->replace(m_dataXIndex, QPointF{ x, y });


        auto max = *std::max_element(chSeries->begin(), chSeries->end(),
                                     [](const QPointF &p1, const QPointF &p2) {
                                         return p1.y() < p2.y();
                                     });

        m_chMaxValues[ch] = static_cast<uint16_t>(max.y());
        m_chTotalSums[ch] += readings[ch];
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
    }
}

double ChartModel::getLastChannelValue(int channel) const {
    return m_chLastValues[channel];
}

double ChartModel::getMaxChannelValue(int channel) const {
    return m_chMaxValues[channel];
}

double ChartModel::getChannelTotalSum(uint8_t channel) const {
    return m_chTotalSums[channel];
}


void ChartModel::updatePlotSeries(
        QAbstractSeries *qtQuickLeftSeries,
        QAbstractSeries *qtQuickRightSeries,
        int channel
) {
    if (qtQuickLeftSeries && qtQuickRightSeries) {
        auto *xyQtQuickLeftSeries = dynamic_cast<QXYSeries *>(qtQuickLeftSeries);
        auto *xyQtQuickRightSeries = dynamic_cast<QXYSeries *>(qtQuickRightSeries);
        auto channelData = &m_chartData[channel];

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

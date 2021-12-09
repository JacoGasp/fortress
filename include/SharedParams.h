//
// Created by Jacopo Gasparetto on 25/11/21.
//

#ifndef FORTRESS_SHAREDPARAMS_H
#define FORTRESS_SHAREDPARAMS_H

#ifndef ESP32
#include <QObject>
#include <QtQml>

class SharedParams : public QObject {
Q_OBJECT
    QML_ELEMENT

#else
class SharedParams {
#endif

public:
    // General
    static constexpr int n_channels = 8;
    static constexpr int defaultMinScreenWidth = 1280;
    static constexpr int defaultMinScreenHeight = 840;
    static constexpr int plotWindowSizeInPoint = 512;
    static constexpr uint16_t integratorThreshold = 65500;

#ifndef ESP32
private:
    // Toolbar
    const QString m_ipPlaceholder = "192.168.1.7";
    QString m_ip = "192.168.178.20";
    static constexpr int m_defaultPort = 60000;
    int m_samplingFreq = 100;
    static constexpr int m_maxAllowedFreq = 1024;

public:
    Q_PROPERTY(int N_CHANNELS READ nChannels CONSTANT)
    Q_PROPERTY(int MIN_SCREEN_WIDTH READ minScreenWidth CONSTANT)
    Q_PROPERTY(int MAX_SCREEN_HEIGHT READ minScreenHeight CONSTANT)
    Q_PROPERTY(QString IP_PLACEHOLDER READ ipPlaceholder CONSTANT)
    Q_PROPERTY(QString ip READ ip WRITE ip NOTIFY ipChanged)
    Q_PROPERTY(int defaultPort READ defaultPort CONSTANT)
    Q_PROPERTY(int samplingFreq READ samplingFreq WRITE setSamplingFreq NOTIFY samplingFreqChanged)
    Q_PROPERTY(int MAX_ALLOWED_FREQ READ maxAllowedFreq CONSTANT)

    [[nodiscard]] static int nChannels() { return n_channels; }

    [[nodiscard]] static int minScreenWidth() { return defaultMinScreenWidth; }

    [[nodiscard]] static int minScreenHeight() { return defaultMinScreenHeight; }

    [[nodiscard]] QString ipPlaceholder() const { return m_ipPlaceholder; }

    [[nodiscard]] QString ip() const { return m_ip; }

    void ip(const QString &ip) { m_ip = ip; }

    [[nodiscard]] static int defaultPort() { return m_defaultPort; }

    [[nodiscard]] int samplingFreq() const { return m_samplingFreq; }

    void setSamplingFreq(int freq) { m_samplingFreq = freq; }

    [[nodiscard]] static int maxAllowedFreq() { return m_maxAllowedFreq; }

signals:

    void ipChanged();

    void samplingFreqChanged();

#endif

};


#endif //FORTRESS_SHAREDPARAMS_H

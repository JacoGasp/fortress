//
// Created by Jacopo Gasparetto on 25/11/21.
//

#ifndef FORTRESS_SHAREDPARAMS_H
#define FORTRESS_SHAREDPARAMS_H


#include <QObject>

class SharedParams : public QObject {
Q_OBJECT
    QML_ELEMENT

private:
    // General
    static constexpr int m_n_channels = 4;
    static constexpr int m_minScreenWidth = 1280;
    static constexpr int m_minScreenHeight = 840;

    // Toolbar
    const QString m_ipPlaceholder = "192.168.1.7";
    QString m_ip = "192.168.1.47";
    static constexpr int m_defaultPort = 60000;
    int m_samplingFreq = 100;
    static constexpr int m_maxAllowedFreq = 1024;

public:
    Q_PROPERTY(int N_CHANNELS READ n_channel CONSTANT)
    Q_PROPERTY(int MIN_SCREEN_WIDTH READ minScreenWidth CONSTANT)
    Q_PROPERTY(int MAX_SCREEN_HEIGHT READ minScreenHeight CONSTANT)
    Q_PROPERTY(QString IP_PLACEHOLDER READ ipPlaceholder CONSTANT)
    Q_PROPERTY(QString ip READ ip WRITE ip NOTIFY ipChanged)
    Q_PROPERTY(int defaultPort READ defaultPort CONSTANT)
    Q_PROPERTY(int samplingFreq READ samplingFreq WRITE setSamplingFreq NOTIFY samplingFreqChanged)
    Q_PROPERTY(int MAX_ALLOWED_FREQ READ maxAllowedFreq CONSTANT)

    [[nodiscard]] static int n_channel() { return m_n_channels; }

    [[nodiscard]] static int minScreenWidth() { return m_minScreenWidth; }

    [[nodiscard]] static int minScreenHeight() { return m_minScreenHeight; }

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

};


#endif //FORTRESS_SHAREDPARAMS_H

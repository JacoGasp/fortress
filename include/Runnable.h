//
// Created by Jacopo Gasparetto on 18/05/21.
//

#ifndef FORTRESS_RUNNABLE_H
#define FORTRESS_RUNNABLE_H

#include <QObject>
#include <QRunnable>
#include <QThread>
#include <iostream>

class Runnable: public QRunnable {
private:
    int m_number{0};
    bool m_isRunning{false};
    QObject *m_receiver;

public:
    explicit Runnable(QObject *receiver);

    void run() override;

    [[nodiscard]] bool isRunning() const;

    void stop();

    friend std::ostream& operator<<(std::ostream &out, const Runnable &runnable);
};


#endif //FORTRESS_RUNNABLE_H

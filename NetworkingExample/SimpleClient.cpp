//
// Created by Jacopo Gasparetto on 20/05/21.
//

#include <iostream>
#include "Networking/client.h"
#include "Constants.h"

class SimpleClient : public fortress::net::ClientInterface<fortress::net::MsgTypes> {

public:
    void pingServer() {
        static int count = 0;
        std::cout << "Sending ping\n";
        fortress::net::message<fortress::net::MsgTypes> msg;
        msg.header.id = fortress::net::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        msg << timeNow;
        send(msg);
    }
};


void quitHandler(bool &shouldRun) {
    while (true) {
        char c{};
        std::cin >> c;
        if (c) {
            shouldRun = false;
            return;
        }
    }
}

[[noreturn]] void pingThread(SimpleClient &client) {
    while (true) {
        client.pingServer();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2s);
    }
}

int main() {
    SimpleClient c;
    c.connect("127.0.0.1", 60000);

    bool bQuit = false;
    std::thread t{ quitHandler, std::ref(bQuit) };
    std::thread ping { pingThread, std::ref(c)};

    while (!bQuit) {

        if (c.isConnected()) {
            if (!c.incoming().empty()) {
                auto msg = c.incoming().pop_front().message;

                switch (msg.header.id) {
                    case fortress::net::MsgTypes::ServerPing: {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                        std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
                                  << " ms.\n";
                    }
                        break;

                    default:
                        std::cout << "???\n";
                }
            }
        }
    }

    t.join();
    ping.join();

    return 0;
}
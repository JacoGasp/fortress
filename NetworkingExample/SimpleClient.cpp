//
// Created by Jacopo Gasparetto on 20/05/21.
//

#include <iostream>
#include "Networking/client.h"
#include "Constants.h"
#include "argparse.h"

using namespace fortress::net;

class SimpleClient : public ClientInterface<MsgTypes> {

public:
    void pingServer() {
        static int count = 0;
        std::cout << "Sending ping\n";
        message<MsgTypes> msg;
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

int main(int argc, char *argv[]) {

    auto parser = ArgumentParser(argc, argv);
    parser.addArgument("ip", "127.0.0.1");
    parser.addArgument("port", 60000);

    parser.parseArguments();

    auto ip = parser.getValue<std::string>("ip");
    auto port = parser.getValue<int>("port");

    SimpleClient c;
    c.connect(ip, port);


    bool bQuit = false;
    std::thread t;
    std::thread ping;

    while (!bQuit) {

        if (c.isConnected()) {
            if (!c.incoming().empty()) {
                auto msg = c.incoming().pop_front().message;

                switch (msg.header.id) {
                    case MsgTypes::ServerAccept: {
                        std::cout << "Server accepted\n";
                        t = std::thread{ quitHandler, std::ref(bQuit) };
                        ping = std::thread{ pingThread, std::ref(c)};
                        break;
                    }
                    case MsgTypes::ServerPing: {
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
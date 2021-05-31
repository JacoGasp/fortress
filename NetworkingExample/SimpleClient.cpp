//
// Created by Jacopo Gasparetto on 20/05/21.
//

#include <iostream>
#include "Networking/client.h"
#include "Constants.h"
#include "argparse.h"
#include <thread>
#include <atomic>

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

    void greeting() {
        message<MsgTypes> msg;
        msg.header.id = ClientMessage;
        msg << "John";
        send(msg);
    }

    void onMessage(message<MsgTypes> &msg) override {
        switch (msg.header.id) {
            case MsgTypes::ServerAccept: {
                std::cout << "Server accepted\n";
                break;
            }

            case MsgTypes::ServerMessage: {
                std::cout << "Message from server: " << msg << ":\n";
                std::ostringstream out;
                for (auto ch: msg.body) {
                    out << ch;
                }
                std::cout << out.str() << std::endl;
            }
                break;

            case MsgTypes::ServerPing: {
                std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                std::chrono::system_clock::time_point timeThen;
                msg >> timeThen;
                std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
                          << " ms.\n";
            }
                break;

            case MsgTypes::ClientPing:
                send(msg);
                break;

            default:
                std::cout << "???\n";
        }
    }
};

char getCommand() {
    std::cout << "Choose a command\n"
    << "\t [p] ping\n"
    << "\t [g] greeting\n"
    << "\t [q] quit\n";
    char c;
    std::cin >> c;
    return c;
}

int main(int argc, char *argv[]) {

    auto parser = ArgumentParser(argc, argv);
    parser.addArgument("ip", std::string {"127.0.0.1"});
    parser.addArgument("port", 60000);

    parser.parseArguments();

    auto ip = parser.getValue<std::string>("ip");
    auto port = parser.getValue<int>("port");

    SimpleClient c;
    c.connect(ip, port);

    c.startUpdating();

    char ch;
    while ((ch = getCommand()) != 'q') {
        switch(ch) {
            case 'p':
                c.pingServer();
                break;
            case 'g':
                c.greeting();
                break;

            default:
                std::cout << "Unknown command\n";
        }
    }
    c.stopUpdating();

    return 0;
}
//
// Created by Jacopo Gasparetto on 15/06/21.
//

#include <iostream>
#include <array>
#include "FRServer.h"
#include "argparse.h"
#include "ValueNoise1D.h"


using namespace fortress::net;

constexpr auto PING_FREQUENCY = std::chrono::seconds(10);

std::atomic_bool shouldRun;
std::atomic_bool bRunPingThread;

ValueNoise1D valueNoise1D;

void quitHandler() {
    while (shouldRun) {
        char c;
        std::cin >> c;
        if (c > 0) {
            shouldRun = false;
            bRunPingThread = false;
            break;
        }
    }
}

void pingHelper(FRServer &server) {
    while (bRunPingThread) {
        server.pingAll();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(PING_FREQUENCY);
    }
}

void update(FRServer *server) {
    static int t;
    message<MsgTypes> msg;
    msg.header.id = ServerReadings;

    std::array<double, 4> values{
            valueNoise1D.eval(static_cast<float>(t)),
            valueNoise1D.eval(static_cast<float>(t + 42)),
            valueNoise1D.eval(static_cast<float>(t + 666)) + 0.3,
            valueNoise1D.eval(static_cast<float>(t - 123) + 0.1)
    };

    msg << values;
    server->sendMessageToAllClients(msg);
    t++;
}

int main(int argc, char *argv[]) {
    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    FRServer server(parser.getValue<int>("port"), &update);
    server.start();

    shouldRun = true;
    bRunPingThread = true;
    std::thread quitThread{ quitHandler };
    std::thread pingThread{ pingHelper, std::ref(server) };

    pingThread.join();
    quitThread.join();

    server.stop();

    return 0;
};
//
// Created by Jacopo Gasparetto on 15/06/21.
//

#include <iostream>
#include "FRServer.h"
#include "argparse.h"

using namespace fortress::net;

constexpr auto PING_FREQUENCY = std::chrono::seconds(10);

std::atomic_bool shouldRun;
std::atomic_bool bRunPingThread;

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

int main(int argc, char *argv[]) {
    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    FRServer server(parser.getValue<int>("port"));
    server.start();

    shouldRun = true;
    bRunPingThread = true;
    std::thread quitThread{ quitHandler};
    std::thread pingThread{ pingHelper, std::ref(server)};

    pingThread.join();
    quitThread.join();

    server.stop();

    return 0;
};
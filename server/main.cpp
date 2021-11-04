//
// Created by Jacopo Gasparetto on 15/06/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
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


void update(FRServer *server) {
    static int t;
    message<MsgTypes> msg;
    msg.header.id = ServerReadings;
    msg << valueNoise1D.eval(static_cast<double>(t))
    << valueNoise1D.eval(static_cast<double>(t + 42))
    << valueNoise1D.eval(static_cast<double>(t + 666) + 0.3)
    << valueNoise1D.eval(static_cast<double>(t - 123) + 0.1);

    server->sendMessageToAllClients(msg);
    ++t;
}

int main(int argc, char *argv[]) {
    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    // ---- ASIO Context ----
    asio::io_context ioContext;

    FRServer server(ioContext, parser.getValue<int>("port"), &update);

    server.start();

    std::thread t([&ioContext]() { ioContext.run(); });

    char ch{};

    while (ch != 'q') {
        std::cout << "Press q to quit.\n";
        std::cin >> ch;
    }

    ioContext.stop();
    t.join();

    return 0;
};
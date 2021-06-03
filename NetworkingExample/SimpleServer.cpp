//
// Created by Jacopo Gasparetto on 19/05/21.
//

#include <iostream>
#include "Networking/server.h"
#include "Constants.h"
#include "argparse.h"

using namespace fortress::net;

constexpr auto PING_FREQUENCY = std::chrono::seconds(10);

class SimpleServer : public ServerInterface<MsgTypes> {
public:
    explicit SimpleServer(uint16_t port)
            : ServerInterface<MsgTypes>(port) {}

protected:
    bool onClientConnect(std::shared_ptr<Connection<MsgTypes>> client) override {
        std::cout << "[SERVER] New Client Connected\n";

        return true;
    }

    void onClientDisconnect(std::shared_ptr<Connection<MsgTypes>> client) override {
        std::cout << '[' << client->getID() << "] Client Disconnected\n";
    }

    void onMessage(const std::shared_ptr<Connection<MsgTypes>> client, message<MsgTypes> &msg) override {

        switch (msg.header.id) {
            case MsgTypes::ServerPing: {
                std::cout << '[' << client->getID() << "]: Server Ping\n";
                //Simply bounce message back to client
                client->send(msg);
            }
                break;

            case MsgTypes::ClientPing: {
                std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                std::chrono::system_clock::time_point timeThen;
                msg >> timeThen;
                std::cout <<  '[' << client->getID() << "] Ping: "
                          << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
                          << " ms.\n";
            }
                break;

            case MsgTypes::MessageAll: {
                std::cout << '[' << client->getID() << "]: Message All\n";
                message<MsgTypes> newMessage;
                newMessage.header.id = MsgTypes::ServerMessage;
                msg << client->getID();
                sendMessageToAllClients(msg, client);
            }

            case MsgTypes::ClientMessage: {
                message<MsgTypes> newMsg;
                newMsg.header.id = ServerMessage;

                std::ostringstream out;
                out << "Hello, ";

                for (int i = 0; i < msg.size() - 1; ++i)
                        out << msg.body[i];

                out << '!';

                for (auto ch : out.str())
                    newMsg << ch;

                sendMessage(client, newMsg);
            } break;

            case MsgTypes::ClientDisconnect: {
                std::cout << '[' << client->getID() << "] Client Disconnects\n";
            } break;

            default:
                std::cout << "???\n";
        }
    }

    void onClientValidated(std::shared_ptr<Connection<MsgTypes>> client) override {
        message<MsgTypes> newMessage;
        newMessage.header.id = ServerAccept;
        newMessage << client->getID();
        sendMessage(client, newMessage);
        std::cout << '[' << client->getID() << "] Client Validated" << std::endl;
    }

public:
    void pingAll() {
        std::cout << "[SERVER]: Ping All\n";
        message<MsgTypes> msg;
        msg.header.id = ClientPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        msg << timeNow;
        sendMessageToAllClients(msg);
    }
};

std::atomic_bool shouldRun;
std::atomic_bool bRunPingThread = false;

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

void pingHelper(SimpleServer &server) {
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

    SimpleServer server(parser.getValue<int>("port"));
    server.start();

    shouldRun = true;
    bRunPingThread = true;
    std::thread quitThread{ quitHandler};
    std::thread pingThread{ pingHelper, std::ref(server)};

    pingThread.join();
    quitThread.join();

    server.stop();

    return 0;
}
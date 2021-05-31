//
// Created by Jacopo Gasparetto on 19/05/21.
//

#include <iostream>
#include "Networking/server.h"
#include "Constants.h"
#include "argparse.h"

using namespace fortress::net;

class SimpleServer : public ServerInterface<MsgTypes> {
public:
    explicit SimpleServer(uint16_t port)
            : ServerInterface<MsgTypes>(port) {}

protected:
    bool onClientConnect(std::shared_ptr<Connection<MsgTypes>> client) override {
        std::cout << "Client " << client->getID() << " connected\n";

        message<MsgTypes> newMessage;
        newMessage.header.id = MsgTypes::ServerAccept;

        return true;
    }

    void onClientDisconnect(std::shared_ptr<Connection<MsgTypes>> client) override {
        std::cout << "Client disconnected\n";
    }

    void onMessage(const std::shared_ptr<Connection<MsgTypes>> client, message<MsgTypes> &msg) override {

        switch (msg.header.id) {
            case MsgTypes::ServerPing: {
                std::cout << '[' << client->getID() << "]: Server ping\n";
                //Simply bounce message back to client
                client->send(msg);
            }
                break;

            case MsgTypes::ClientPing: {
                std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                std::chrono::system_clock::time_point timeThen;
                msg >> timeThen;
                std::cout << "Ping client [" << client->getID() << "]: "
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
                out << "Hello";

                for (auto ch : msg.body)
                        out << ch;

                out << '!';

                for (auto ch : out.str())
                    newMsg << ch;

                sendMessage(client, newMsg);
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


void quitHandler(bool &shouldRun) {
    while (true) {
        char c;
        std::cin >> c;
        if (c > 0) {
            shouldRun = false;
            break;
        }
    }
}

void pingHelper(SimpleServer &server) {
    while (true) {
        server.pingAll();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(30s);
    }
}

int main(int argc, char *argv[]) {

    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    SimpleServer server(parser.getValue<int>("port"));
    server.start();

//    auto pingThread{std::thread{pingHelper, std::ref(server)}};

    bool shouldRun = true;
    std::thread t{quitHandler, std::ref(shouldRun)};

    while (shouldRun)
        server.update(-1, true);

//    pingThread.join();
    t.join();

    server.stop();

    return 0;
}
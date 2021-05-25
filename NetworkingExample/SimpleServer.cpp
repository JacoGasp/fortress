//
// Created by Jacopo Gasparetto on 19/05/21.
//

#include <iostream>
#include "Networking/server.h"
#include "Constants.h"
#include "argparse.h"

using namespace fortress::net;

class SimpleServer: public ServerInterface<MsgTypes> {
public:
    explicit SimpleServer(uint16_t port)
    : ServerInterface<MsgTypes>(port){}

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
                 std::cout << '[' << client->getID()  << "]: Server ping\n";
                 //Simply bounce message back to client
                 client->send(msg);
             }
             break;

             case MsgTypes::MessageAll: {
                 std::cout << '[' << client->getID() << "]: Message All\n";
                 message<MsgTypes> newMessage;
                 newMessage.header.id = MsgTypes::ServerMessage;
                 msg << client->getID();
                 sendMessageToAllClients(msg, client);
             }

             default:
                 std::cout << "???\n";
         }
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

int main(int argc, char *argv[]) {

    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    SimpleServer server(parser.getValue<int>("port"));
    server.start();

    bool shouldRun = true;
    std::thread t{ quitHandler, std::ref(shouldRun)};

    while(shouldRun)
        server.update(-1, true);

    t.join();

    server.stop();

    return 0;
}
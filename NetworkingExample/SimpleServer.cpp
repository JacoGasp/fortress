//
// Created by Jacopo Gasparetto on 19/05/21.
//

#include <iostream>
#include "Networking/server.h"
#include "Constants.h"

class SimpleServer: public fortress::net::ServerInterface<fortress::net::MsgTypes> {
public:
    explicit SimpleServer(uint16_t port)
    : fortress::net::ServerInterface<fortress::net::MsgTypes>(port){}

protected:
    bool onClientConnect(std::shared_ptr<fortress::net::Connection<fortress::net::MsgTypes>> client) override {
        std::cout << "Client " << client->getID() << " connected\n";
        return true;
    }

    void onClientDisconnect(std::shared_ptr<fortress::net::Connection<fortress::net::MsgTypes>> client) override {
        std::cout << "Client disconnected\n";
    }

    void onMessage(const std::shared_ptr<fortress::net::Connection<fortress::net::MsgTypes>> client, fortress::net::message<fortress::net::MsgTypes> &msg) override {

         switch (msg.header.id) {
             case fortress::net::MsgTypes::ServerPing: {
                 std::cout << '[' << client->getID()  << "]: Server ping\n";
                 //Simply bounce message back to client
                 client->send(msg);
             }
             break;

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

int main() {

    SimpleServer server(60000);
    server.start();

    bool shouldRun = true;
    std::thread t{ quitHandler, std::ref(shouldRun)};

    while(shouldRun)
        server.update();

    t.join();


    return 0;
}
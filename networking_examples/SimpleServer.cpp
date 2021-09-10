//
// Created by Jacopo Gasparetto on 01/09/21.
//

#include "networking/server_interface.h"
#include "argparse.h"
#include <string>
#include <fstream>
#include <cstring>
#include <iomanip>

using namespace fortress::net;

class SimpleServer : public server_interface {
private:
    std::fstream m_file;
public:
    SimpleServer(asio::io_context &context, uint16_t port) : server_interface(context, port) {
        m_file.open("/tmp/server.log");
    }

    ~SimpleServer() override {
        m_file.close();
    }

protected:
    bool onClientConnect(std::shared_ptr<tcp_connection> client) override {
        return true;
    }

    void onClientDisconnect(std::shared_ptr<tcp_connection> client) override {

    }

    void onMessage(std::shared_ptr<tcp_connection> client, message<MsgTypes> &msg) override {
        std::cout << "[" << client->getID() << "] new message: " << msg << '\n';

        size_t i = sizeof(msg.header) + msg.header.size;

        std::vector<uint8_t> b;
        b.resize(i);

        std::memcpy(b.data(), &msg.header, sizeof(msg.header));
        std::memcpy(b.data() + sizeof(msg.header), msg.body.data(), msg.size());

        for (auto &el: b)
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)el << std::dec << ' ';

        std::cout << std::endl;

        if (msg.header.id == fortress::net::ServerPing) {
            sendMessage(client, msg);
        }
    }
};

int main(int argc, char *argv[]) {
    ArgumentParser parser(argc, argv);
    parser.addArgument<int>("port", 60000);
    parser.parseArguments();

    // ---- ASIO Context ----
    asio::io_context ioContext;

    SimpleServer server(ioContext, parser.getValue<int>("port"));
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

}
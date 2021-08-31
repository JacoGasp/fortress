//
// Created by Jacopo Gasparetto on 10/08/21.
//
#include "client_v2.h"

using namespace fortress::net;

class Client : public client_v2 {
public:
    explicit Client(asio::io_context &io_context) : fortress::net::client_v2(io_context){};
};


int main() {
    asio::io_context io_context;
    auto client = Client(io_context);

    client.connect("127.0.0.1", 63419);

    std::thread t([&io_context](){ io_context.run(); });

    char line[512];

    while (std::cin.getline(line, 512)) {
        if (line[0] == 'q')
            break;

        auto msg = message<MsgTypes>();
        msg.header.id = ::ClientMessage;
        msg << line;
        client.sendMessage(msg);
        std::cout << "message sent\n";
    }

    client.disconnect();
    t.join();

    return 0;
}


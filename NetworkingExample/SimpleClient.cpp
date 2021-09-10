//
// Created by Jacopo Gasparetto on 10/08/21.
//
#include "networking/client_interface.h"
#include "argparse.h"

using namespace fortress::net;


class Client : public client_interface {
private:
    bool m_bIsPinging{ false };
    std::unique_ptr<asio::steady_timer> m_pPingTimer;
    static constexpr asio::chrono::milliseconds PING_DELAY{ 1000 };

public:
    explicit Client(asio::io_context &io_context) :
            fortress::net::client_interface(io_context),
            m_pPingTimer{ std::make_unique<asio::steady_timer>(io_context, PING_DELAY) } {};

    void sendPing() {
        message<MsgTypes> msg;
        msg.header.id = ServerPing;
        auto now = std::chrono::system_clock::now();
        msg << now;
        sendMessage(msg);
    }

    void togglePing() {
        if (!m_bIsPinging) {
            m_bIsPinging = true;
            pingHandler();
        } else {
            m_bIsPinging = false;
        }
    }

    void pingHandler() {
        if (m_bIsPinging) {
            sendPing();
            m_pPingTimer->expires_from_now(PING_DELAY);
            m_pPingTimer->async_wait([this](asio::error_code ec) {
                pingHandler();
            });
        }
    }

    void onMessage(message<MsgTypes> &msg) override {
        std::cout << "New message: " << msg << '\n';
        switch (msg.header.id) {
            case ::ServerPing: {
                std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                std::chrono::system_clock::time_point timeThen;

                msg >> timeThen;
                auto ping = std::chrono::duration<double>(timeNow - timeThen).count() * 1000;
                std::cout << "Ping: " << ping << '\n';
            }
                break;

            default:
                std::cout << "Unknown message.\n";
        }
    }

};

char getCommand() {
    std::cout << "Choose a command\n"
              << "\t [p] ping\n"
              << "\t [f] send pi\n"
              << "\t [d] disconnect\n"
              << "\t [u] update\n"
              << "\t [s] stop updating\n"
              << "\t [q] quit\n";
    char c;
    std::cin >> c;
    return c;
}

void sendCommand(char ch, Client &c) {
    switch (ch) {
        case 'f':
            c.sendPing();
            break;
        case 'p':
            c.togglePing();
            break;
//        case 'g':
//            c.greeting();
//            break;

//        case 'd':
//            c.disconnect();
//            break;
//
//        case 'u':
//            c.startUpdate();
//            break;
//
//        case 's':
//            c.stopUpdate();
//            break;

        default:
            std::cout << "Unknown command\n";
    }
}

int main(int argc, char *argv[]) {

    // --- Parse Arguments ---
    auto parser = ArgumentParser(argc, argv);
    parser.addArgument("ip", std::string{ "127.0.0.1" });
    parser.addArgument("port", 60000);

    parser.parseArguments();

    auto ip = parser.getValue<std::string>("ip");
    auto port = parser.getValue<int>("port");

    // --- Start Context ---
    asio::io_context io_context;
    auto client = Client(io_context);

    // Connect to server and run context
    client.connect(ip, port);
    std::thread t([&io_context]() { io_context.run(); });

    char ch;
    while ((ch = getCommand()) != 'q') {
        sendCommand(ch, client);
    }

    client.disconnect();
    t.join();

    return 0;
}


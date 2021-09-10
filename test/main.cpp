#include "networking/message.h"
#include "constants.h"
#include <iostream>
#include <cstdint>
#include <cstring>
#include <bitset>
#include <iomanip>

using namespace fortress::net;

void printByteArray(char *begin, const char *end) {
    for (auto *ptr { begin }; ptr != end; ++ptr) {
        std::bitset<8> x(*ptr);
        std::cout << x << ' ';
    }
}


int main() {
    float pi = 3.14;

    // Define message
    message<MsgTypes> msg;
    msg.header.id = ClientMessage;
    msg << pi;

    std::cout << msg << '\n';

    // Encode header and message
    char dataHeader[sizeof(message_header<MsgTypes>)];
    char dataBody[msg.size()];

    // Encode header
    std::memcpy(dataHeader, &msg.header, sizeof(message_header<MsgTypes>));
    std::memcpy(dataBody, msg.body.data(), sizeof(dataBody));

    std::cout << "Encoded header: ";
    printByteArray(&dataHeader[0], &dataHeader[0] + sizeof(message_header<MsgTypes>));
    std::cout << '\n';

    std::cout << "Encoded body: ";
    printByteArray(&dataBody[0], &dataBody[0] + msg.size());
    std::cout << '\n';


    // Read encoded header
    message_header<MsgTypes> decodedHeader;
    decodedHeader << dataHeader;
    std::cout << "Decoded header id: " << decodedHeader.id << ", message size: " << decodedHeader.size << '\n';

    // Decode message
    float decodePi;
    msg >> decodePi;

    std::cout << "Decoded pi: " << decodePi << '\n';

    std::cout << "Hex header: ";
    for (auto& el : dataHeader)
        std::cout << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)el) << ' ';
    std::cout << '\n';

    std::cout << "Hex message: ";
        for (auto& el : dataBody)
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)el) << ' ';


    return 0;
}
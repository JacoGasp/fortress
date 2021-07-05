//
// Created by Jacopo Gasparetto on 20/05/21.
//

#ifndef FORTRESS_CONSTANTS_H
#define FORTRESS_CONSTANTS_H

namespace fortress::consts {
    constexpr uint16_t WINDOW_SIZE_IN_POINT = 512;
    constexpr uint8_t N_CHANNELS = 4;
}

namespace fortress::net {
    enum MsgTypes : uint32_t {
        ServerAccept,
        ServerDeny,
        ServerMessage,
        ServerPing,
        ServerReadings,

        ClientPing,
        ClientMessage,
        ClientDisconnect,
        ClientStartUpdating,
        ClientStopUpdating,
        ClientSetSampleFrequency,

        MessageAll
    };
}

#endif //FORTRESS_CONSTANTS_H

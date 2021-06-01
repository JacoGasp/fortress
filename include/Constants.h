//
// Created by Jacopo Gasparetto on 20/05/21.
//

#ifndef FORTRESS_CONSTANTS_H
#define FORTRESS_CONSTANTS_H

namespace fortress::consts {

}

namespace fortress::net {
    enum MsgTypes : uint32_t {
        ServerAccept,
        ServerDeny,
        ServerMessage,
        ServerPing,

        ClientPing,
        ClientMessage,
        ClientDisconnect,

        MessageAll
    };
}

#endif //FORTRESS_CONSTANTS_H

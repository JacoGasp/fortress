//
// Created by Jacopo Gasparetto on 19/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_MESSAGE_H
#define FORTRESS_MESSAGE_H

#include "commons.h"

namespace fortress::net {
    template<typename T>
    struct MessageHeader {
        T id{};
        uint32_t size = 0;
    };

    template<typename T>
    struct message {
        MessageHeader<T> header{};
        std::vector<uint8_t> body;

        // The size of entire message packet in bytes
        [[nodiscard]] size_t size() const {
            return body.size();
        }

        friend std::ostream &operator<<(std::ostream &out, const message<T> &msg) {
            out << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
            return out;
        }

        // Push any DataType data into the message buffer
        template<typename DataType>
        friend message<T> &operator<<(message<T> &msg, const DataType &data) {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed in a vector");

            // Cache current vector size
            size_t i = msg.body.size();

            // Resize the vector by the size of the data being pushed
            msg.body.resize(msg.body.size() + sizeof(DataType));

            // Physically copy the data into the newly allocated vector space
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

            // Update message size
            msg.header.size = msg.size();

            return msg;
        }

        template<typename DataType>
        friend message<T> &operator>>(message<T> &msg, DataType &data) {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed in a vector");

            // Cache the location towards the end of the vector where the pulled data starts
            size_t i = msg.body.size() - sizeof(DataType);

            // Physically copy data from the vector into user variable
            std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

            // Shrink the vector to remove read bytes and reset end position
            msg.body.resize(i);

            // Recalculate message size
            msg.header.size = msg.size();

            return msg;
        }
    };

    // Forward declare the connection
    template<typename T>
    class Connection;

    template<typename T>
    struct owned_message {

        std::shared_ptr<Connection<T>> remote = nullptr;
        fortress::net::message<T> message;

        friend std::ostream &operator<<(std::ostream &out, const owned_message<T> &msg) {
            out << msg.message;
            return out;
        }
    };

}

#endif //FORTRESS_MESSAGE_H

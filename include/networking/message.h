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

namespace fortress {
    namespace net {
        template<typename T>
        struct message_header {
            T id{};
            uint32_t size = 0;

            template<typename DataType>
            friend message_header<T> &operator<<(message_header<T> &header, const DataType &data) {
                std::memcpy(&header, &data, sizeof(DataType));
                return header;
            }

            friend std::ostream &operator<<(std::ostream &out, const message_header<T> &header) {
                out << "ID: " << int(header.id) << " Size: " << header.size;
                return out;
            }
        };

        template<typename T>
        struct message {
        private:
        public:
            message_header<T> header{};
            std::vector<uint8_t> body;


            // The size of entire message packet in bytes
            size_t size() const {
                return body.size();
            }

            friend std::ostream &operator<<(std::ostream &out, const message<T> &msg) {
                std::array<uint8_t, sizeof(message_header<T>)> _header;
                std::vector<uint8_t> _body;
                _body.resize(msg.size());

                std::memcpy(_header.data(), &msg.header, _header.size());
                std::memcpy(_body.data(), msg.body.data(), msg.size());

                for (auto el : _header)
                    out << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(el) << ' ';

                for (auto el: msg.body)
                    out << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(el) << ' ';

                out << std::dec;

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
        class tcp_connection;

        template<typename T>
        struct owned_message {

            std::shared_ptr<tcp_connection> remote = nullptr;
            fortress::net::message<T> message;

            friend std::ostream &operator<<(std::ostream &out, const owned_message<T> &msg) {
                out << msg.message;
                return out;
            }
        };
    }
}

#endif //FORTRESS_MESSAGE_H

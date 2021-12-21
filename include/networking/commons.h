//
// Created by Jacopo Gasparetto on 19/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_COMMONS_H
#define FORTRESS_COMMONS_H

#include <cinttypes>
#include <vector>
#include <iostream>
#include <memory>
#include <deque>
#include <mutex>
#include <utility>
#include <atomic>
#include <cstring>
#include <iomanip>

#if !defined(ESP32 )
#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ts/buffer.hpp"
#include "asio/ts/internet.hpp"
#endif

#endif //FORTRESS_COMMONS_H


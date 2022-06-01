
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef STREAMS_HPP
#define STREAMS_HPP

#include "GArray.hpp"
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"

#include <string> // std::string

bool reader_for_tx_words(GArray<uint16_t>& array, const std::string& filename);

bool reader_for_tx_words(GArray<uint16_t>& array, GUdpServer& server);

bool writer_for_rx_words(GArray<uint16_t>& array, const std::string& filename);

bool writer_for_rx_words(GArray<uint16_t>& array, GUdpClient& client);

#endif // STREAMS_HPP

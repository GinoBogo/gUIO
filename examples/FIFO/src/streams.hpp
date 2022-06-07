
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

bool stream_reader_for_tx_words(GArray<uint16_t>* array, GUdpClient* client, GUdpServer* server);

bool stream_writer_for_rx_words(GArray<uint16_t>* array, GUdpClient* client, GUdpServer* server);

#endif // STREAMS_HPP

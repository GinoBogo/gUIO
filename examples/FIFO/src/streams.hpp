
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef STREAMS_HPP
#define STREAMS_HPP

#include "GArrayRoller.hpp"
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"

typedef GArray<uint16_t>       g_array_t;
typedef GArrayRoller<uint16_t> g_array_roller_t;

bool stream_reader_for_tx_words(g_array_t* array, GUdpClient* client, GUdpServer* server);

bool stream_writer_for_rx_words(g_array_t* array, GUdpClient* client, GUdpServer* server);

void evaluate_stream_reader_start(g_array_roller_t* roller, GUdpClient* client);

void evaluate_stream_reader_stop(g_array_roller_t* roller, GUdpClient* client);

#endif // STREAMS_HPP

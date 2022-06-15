
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef STREAMS_HPP
#define STREAMS_HPP

#include "globals.hpp"

bool stream_reader_for_tx_words(g_array_t* array, g_udp_client_t* client, g_udp_server_t* server);

bool stream_writer_for_rx_words(g_array_t* array, g_udp_client_t* client, g_udp_server_t* server);

void evaluate_stream_reader_start(g_array_roller_t* roller, g_udp_client_t* client);

void evaluate_stream_reader_stop(g_array_roller_t* roller, g_udp_client_t* client);

#endif // STREAMS_HPP

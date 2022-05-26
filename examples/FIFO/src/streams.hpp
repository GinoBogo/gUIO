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

#include <string> // std::string

void reader_for_tx_words(GArray<uint16_t>& array, const std::string& filename);

void writer_for_rx_words(GArray<uint16_t>& array, const std::string& filename);

#endif // STREAMS_HPP
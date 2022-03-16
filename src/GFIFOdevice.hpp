////////////////////////////////////////////////////////////////////////////////
/// \file      GFIFOdevice.hpp
/// \version   0.1
/// \date      March 15, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GFIFODEVICE_HPP
#define GFIFODEVICE_HPP

#include "GMAPdevice.hpp"
#include "GUIOdevice.hpp"

class GFIFOdevice {
    public:
    GFIFOdevice(size_t dev_addr, size_t dev_size, int uio_num, int uio_map);
    ~GFIFOdevice();

    bool     Open();
    void     Close();
    bool     Reset();
    bool     SetPacketSize(uint32_t words);
    uint32_t GetPacketSize(bool *error = nullptr);
    uint32_t GetFreeWords(bool *error = nullptr);

    private:
    size_t   m_dev_addr;
    size_t   m_dev_size;
    int      m_uio_num;
    int      m_uio_map;
    uint32_t m_words;

    GMAPdevice *m_dev;
    GUIOdevice *m_uio;
    bool        m_is_ready;
};

#endif // GFIFODEVICE_HPP

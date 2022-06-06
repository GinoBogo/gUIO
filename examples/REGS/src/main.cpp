
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"
#include "GMAPdevice.hpp"
#include "GRegisters.hpp"
#include "GString.hpp"

#include <fstream>

#define PS2PL_REGS_ADDR 0xA0050000
#define PS2PL_REGS_SIZE 4096
#define PS2PL_REGS_NUMB 256

#define PL2PS_REGS_ADDR 0xA0060000
#define PL2PS_REGS_SIZE 4096
#define PL2PS_REGS_NUMB 256

static auto load_registers_values = [](const std::string& filename, GMAPdevice::reg_list_t& reg_list) {
    reg_list.clear();

    if (filename.length() > 0) {
        std::ifstream fs;
        fs.open(filename);

        if (fs.is_open()) {
            std::string line;

            while (!fs.eof()) {
                std::getline(fs, line);

                auto items{GString::split(line, "[ \\t]")};
                if (items.size() == 2) {
                    GMAPdevice::reg_pair_t reg_pair;

                    reg_pair.offset = GString::strtoui(items[0]);
                    reg_pair.value  = GString::strtoui(items[1]);
                    reg_list.push_back(reg_pair);
                }
            }

            fs.close();
            return true;
        }
    }
    return false;
};

int main(int argc, char* argv[]) {
    auto exec     = std::filesystem::path(argv[0]);
    auto exec_log = exec.stem().concat(".log");

    GLogger::Initialize(exec_log.c_str());
    LOG_FORMAT(trace, "Process STARTED (%s)", exec.stem().c_str());

    auto _exit_code{-1};

    if (argc > 1) {
        std::string filename{argv[1]};

        auto ps2pl_regs{GMAPdevice(PS2PL_REGS_ADDR, PS2PL_REGS_SIZE)};
        auto pl2ps_regs{GMAPdevice(PL2PS_REGS_ADDR, PL2PS_REGS_SIZE)};

        GMAPdevice::reg_list_t reg_list;

        load_registers_values(filename, reg_list);

        if (ps2pl_regs.Open()) {
            if (ps2pl_regs.MapToMemory()) {
                ps2pl_regs.Write(reg_list);
                ps2pl_regs.Read(reg_list);

                LOG_FORMAT(info, "Write PL registers with \"%s\" file values:", filename.c_str());
                for (const auto& it : reg_list) {
                    auto offset = it.offset;
                    auto value  = it.value;
                    auto bits   = to_bits(value);
                    LOG_FORMAT(debug, "  %3u, %10u, %s", offset, value, bits.c_str());
                }
                _exit_code = 0;
            }

            ps2pl_regs.Close();
        }
    }
    else {
        LOG_WRITE(error, "Command line argument is empty");
    }

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return _exit_code;
}

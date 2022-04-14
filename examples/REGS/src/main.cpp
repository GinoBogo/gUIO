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

#include <cstring> // string, memset
#include <fstream>
#include <regex>

#define PS2PL_REGS_ADDR 0xA0050000
#define PS2PL_REGS_SIZE 4096
#define PS2PL_REGS_NUMB 256

#define PL2PS_REGS_ADDR 0xA0060000
#define PL2PS_REGS_SIZE 4096
#define PL2PS_REGS_NUMB 256

static auto split = [](const std::string& data, const std::string& regex) {
    std::vector<std::string> tokens;

    std::regex                 re{regex};
    std::sregex_token_iterator next{data.begin(), data.end(), re, -1};
    std::sregex_token_iterator last;

    while (next != last) {
        tokens.push_back(next->str());
        ++next;
    }

    auto filter = [](const std::string s) {
        return (s.size() == 0);
    };
    auto junks{std::remove_if(tokens.begin(), tokens.end(), filter)};
    tokens.erase(junks, tokens.end());
    return tokens;
};

static auto strtoui = [](std::string& value) {
    auto _base{10};
    auto _last{value.back()};

    if (_last == 'h' || _last == 'H') {
        value.pop_back();
        _base = 16;
    }
    else if (_last == 'b' || _last == 'B') {
        value.pop_back();
        _base = 2;
    }

    return static_cast<uint32_t>(std::strtoul(value.c_str(), 0, _base));
};

static auto load_registers_values = [](const std::string& filename, GMAPdevice::reg_list_t& reg_list) {
    reg_list.clear();

    if (filename.length() > 0) {
        std::ifstream fs;
        fs.open(filename);

        if (fs.is_open()) {
            std::string line;

            while (!fs.eof()) {
                std::getline(fs, line);

                auto items{split(line, "[ \\t]")};
                if (items.size() == 2) {
                    GMAPdevice::reg_pair_t reg_pair;

                    reg_pair.offset = strtoui(items[0]);
                    reg_pair.value  = strtoui(items[1]);
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
    GLogger::Initialize("MUST_REGS.log");
    LOG_WRITE(trace, "Process STARTED");

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
        LOG_WRITE(warning, "Command line argument is empty");
    }

    LOG_WRITE(trace, "Process STOPPED");
    return _exit_code;
}

//
// Created by drake on 27/8/22.
//

#include "Console.hpp"

Console::Console() : cpu(this), memory(array<byte, memory_map_size>{0}), ram_enabled(false), rom_bank_number(0),
                     ram_bank_number(0), mode_flag(0), number_of_rom_banks(0), number_of_ram_banks(0) {}

void Console::boot(vector<byte> &data) {
    cartridge.init(data);
    number_of_rom_banks = cartridge.number_of_rom_banks;
    number_of_ram_banks = cartridge.number_of_ram_banks;
}

void Console::loop() {
    [[maybe_unused]] auto cycles = cpu.loop();
    std::cout << static_cast<int>(cpu.get(Reg::a));
}

void Console::write(word &address, byte value) {

    if (((0xE000 <= address) && (address <= 0xFDFF)) || ((0xFEA0 <= address) && (address <= 0xFEFF)))
        return;

    memory[address] = value;

    if (address < 0x2000) {
        if (number_of_ram_banks > 0)
            ram_enabled = ((value & 0xF) == 0xA);
        return;
    }

    if (address < 0x4000) {
        byte bitmask = (number_of_rom_banks - 1) & 0x11111;
        if (value == 0) {
            rom_bank_number = 0;
        } else
            rom_bank_number = value & bitmask;
        return;
    }

    if (address < 0x6000) {
        ram_bank_number = value & 0x11;
        return;
    }

    if (address < 0x8000) {
        mode_flag = (address & 1);
        return;
    }

    if (address < 0xC000) {
        if (ram_enabled) {
            word offset = address - 0xA000;
            if (mode_flag == 0)
                cartridge.set_ram_bank(0, offset, value);
            if (mode_flag == 1)
                cartridge.set_ram_bank(ram_bank_number, offset, value);
        }
        return;
    }
}

byte Console::read(word &address) {
    if (0xE000 <= address && address < 0xFE00) //E000 - FDFF (Echo RAM)
        return 0xFF;

    if (0xFEA0 <= address && address < 0xFF00) //FEA0 - FEFF (Not Usable)
        return 0xFF;

    if ((0xC000 <= address) || ((0x8000 <= address) && (address < 0xA000)))
        return memory[address];

    if (address < 0x4000) {
        if (mode_flag == 0)
            return cartridge.get_rom_bank(0, address);
        else {
            byte zero_bank_number = ((ram_bank_number) << 5) & (number_of_rom_banks - 1);
            //TODO Multi ROM carts behavior different
            return cartridge.get_rom_bank(zero_bank_number, address);
        }
    }

    if (address < 0x8000) {
        word offset = address - 0x4000;
        byte bitmask = (number_of_rom_banks - 1) & 0x11111;
        byte high_bank_number = ((ram_bank_number >> 5) & (number_of_rom_banks - 1)) + (rom_bank_number & bitmask);
        return cartridge.get_rom_bank(high_bank_number, offset);
    }

    if (address < 0xC000) {
        if (!ram_enabled)
            return 0xFF;

        word offset = address - 0xA000;

        if (mode_flag == 0)
            return cartridge.get_ram_bank(0, offset);
        else
            return cartridge.get_ram_bank(ram_bank_number, offset);
    }

    return 0xFF;
}
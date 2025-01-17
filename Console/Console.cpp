//
// Created by drake on 27/8/22.
//

#include "Console.hpp"

Console::Console() : cpu(this), memory{},
                     ram_enabled(false), renderer(this), rom_bank_number(0), ram_bank_number(0),
                     mode_flag(0), number_of_rom_banks(0), number_of_ram_banks(0) {
}

void Console::boot_rom() {

    constexpr array<byte, 48> nintendo_logo{
            0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
            0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
            0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
            0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
            0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
            0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };

    for (int i = 0; i < 48; i++) {
        write(0x8000 + i, nintendo_logo[i]);
    }

    write(bgp_palette, 0xFC);
    write(0xFF40, 0xFF);
}

void Console::init(vector<byte> &data) {
    cartridge.init(data);
    number_of_rom_banks = cartridge.number_of_rom_banks;
    number_of_ram_banks = cartridge.number_of_ram_banks;
    boot_rom();
}

void Console::loop() {
    SDL_Event e;
    bool open = true;
    while (open) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    open = false;
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        renderer.resize();
                    }
                }
            }
        }
        auto cycles = 3;
        cycles = cpu.run_instruction_cycle();
        renderer.update(cycles);

    }
}

void Console::run(vector<byte> &data) {
    init(data);
    loop();
}

void Console::write(word address, byte value) {
    memory.at(address) = value;
    // Read Only Segments

    static bool done[256] = {false};
    if (address == 0xFF02 && value == 0x81) {
        if (!done[memory[0xFF01]])
            std::cout << (int) memory[0xFF01] << "\n";
        done[memory[0xFF01]] = true;
    }

    if (address < 0x2000) { // RAM_enable: 0x0000 - 0x1FFF
        if (number_of_ram_banks > 0)
            ram_enabled = ((value & 0xF) == 0xA);
        return;
    }

    if (address < 0x4000) { // ROM_Bank_Number: 0x2000 - 0x3FFF
        byte bitmask = (number_of_rom_banks - 1) & 0x11111;
        if (value == 0) {
            rom_bank_number = 0;
        } else
            rom_bank_number = value & bitmask;
        return;
    }

    if (address < 0x6000) { // RAM_Bank_Number: 0x4000 - 0x5FFF
        ram_bank_number = value & 0x11;
        return;
    }

    if (address < 0x8000) { // Mode_Select: 0x6000 - 0x7FFF
        mode_flag = (address & 1);
        return;
    }

    // Read Write Segments

    if (address < 0xA000) { // VRAM: 0x8000 - 0x9FFF
        return;
    }

    if (address < 0xC000) { // External_RAM: 0xA000 - 0xBFFF
        if (ram_enabled) {
            word offset = address - 0xA000;
            if (mode_flag == 0)
                cartridge.set_ram_bank(0, offset, value);
            if (mode_flag == 1)
                cartridge.set_ram_bank(ram_bank_number, offset, value);
        }
        return;
    }

    if (address < 0xD000) { // Fixed_WRAM: 0xC000 - 0xCFFF
        return;
    }

    if (address < 0xE000) { // Swappable_WRAM(in CGB): 0xD000 - 0xDFFF
        return;
    }

    if (address < 0xFE00) { // Mirror_RAM: 0xE000 - 0xFDFF
        return;
    }

    if (address < 0xFEA0) { // Sprite_OAM: 0xFE00 - 0xFE9F
        return;
    }

    if (address < 0xFF00) { // Unused: 0xFEA0 - 0xFEFF
        return;
    }

    if (address == 0xFF01) {
    }

    if (address < 0xFF80) { // IO-Registers: 0xFF00 - 0xFF7F
    }

    if (address < 0xFFFF) { // High_RAM: 0xFF80 - 0xFFFE
        return;
    }

    if (address == 0xFFFF) { //Interrupt_Enable: 0xFFFF - 0xFFFF
        return;
    }
}

byte Console::read(word address) {

    if (address < 0x4000) { // ROM_BANK_00: 0x0000 - 0x3FFF

        if (mode_flag == 0)
            return cartridge.get_rom_bank(0, address);

        else {
            byte zero_bank_number = ((ram_bank_number) << 5) & (number_of_rom_banks - 1);
            //TODO Multi ROM carts behavior different
            return cartridge.get_rom_bank(zero_bank_number, address);
        }

    }

    if (address < 0x8000) { // ROM_BANK_NN: 0x4000 - 0x7FFF
        word offset = address - 0x4000;
        byte bitmask = (number_of_rom_banks - 1) & 0x11111;
        byte high_bank_number = ((ram_bank_number >> 5) & (number_of_rom_banks - 1)) + (rom_bank_number & bitmask);
        return cartridge.get_rom_bank(high_bank_number, offset);
    }

    if (address < 0xA000) { // VRAM: 0x8000 - 0x9FFF
        // std::cout << (int) memory[address] << "\n";
        return memory[address];
    }

    if (address < 0xC000) { // External_RAM: 0xA000 - 0xBFFF
        if (!ram_enabled)
            return 0xFF;

        word offset = address - 0xA000;

        if (mode_flag == 0)
            return cartridge.get_ram_bank(0, offset);
        else
            return cartridge.get_ram_bank(ram_bank_number, offset);
    }

    if (address < 0xD000) { // Fixed_WRAM: 0xC000 - 0xCFFF
        return memory[address];
    }

    if (address < 0xE000) { // Swappable_WRAM(in CGB): 0xD000 - 0xDFFF
        return memory[address];
    }

    if (address < 0xFE00) { // Mirror_RAM: 0xE000 - 0xFDFF
        return 0xFF;
    }

    if (address < 0xFEA0) { // Sprite_OAM: 0xFE00 - 0xFE9F
        return memory[address];
    }

    if (address < 0xFF00) { // Unused: 0xFEA0 - 0xFEFF
        return 0xFF;
    }

    if (address < 0xFF80) { // IO-Registers: 0xFF00 - 0xFF7F
        return memory[address];
    }

    if (address < 0xFFFF) { // High_RAM: 0xFF80 - 0xFFFE
        return memory[address];
    }

    if (address == 0xFFFF) { //Interrupt_Enable: 0xFFFF - 0xFFFF
        return memory[address];
    }
    return 0xAB;
}
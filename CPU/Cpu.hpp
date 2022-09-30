//
// Created by drake on 27/8/22.
//

#ifndef DMGB_CPU_HPP
#define DMGB_CPU_HPP

#include <stack>
#include "Arithmetic.hpp"
#include "Unary.hpp"
#include "Bit_Operations.hpp"
#include "Load.hpp"
#include "Store.hpp"
#include "Jump_and_Stack.hpp"
#include "Misc.hpp"
#include "Instructions.hpp"

class Console;

class Instructions;

class Cpu {
    word SP, PC;
    array<byte, 9> reg_mapper{};
    std::stack<byte> stack;
    vector<Flag_Status> flags;
    Console *game;
public:

    explicit Cpu(Console *game);

    //void halt(bool status);

    void push(byte to_push);

    byte pop();

    byte read(word address);

    void write(word address, byte value);

    byte get(Reg reg_index);

    word get(DReg reg_index);

    void set(Reg reg_index, byte value);

    void set(DReg reg_index, word value);

    void set_flags(vector<Flag_Status> &flag_array);

    vector<byte> fetch(Instructions &instruction_data);

    void decode_and_execute(vector<byte> fetched, Instructions &instruction_data);

    void loop();
};


#endif //DMGB_CPU_HPP
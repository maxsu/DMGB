//
// Created by drake on 14/9/22.
//

#ifndef DMGB_CPU_UTILITY_HPP
#define DMGB_CPU_UTILITY_HPP

#include "../Base/Constants.hpp"
#include <functional>
#include <variant>

using std::function;

enum struct Reg {
    b = 0,
    c = 1,
    d [[maybe_unused]] = 2,
    e [[maybe_unused]] = 3,
    h [[maybe_unused]] = 4,
    l [[maybe_unused]] = 5,
    a = 7,
    f = 8
};

enum struct DReg {
    bc [[maybe_unused]] = 0,
    de [[maybe_unused]] = 2,
    hl = 4,
    af = 7,
    sp = 6,
    pc
};

enum Flag {
    z = 7,
    n = 6,
    h = 5,
    c = 4
};

inline constexpr word IME = 0xFFFF;

struct Flag_Status {
    Flag bit;
    bool status;

    Flag_Status(Flag x, bool stat) : bit(x), status(stat) {};
};

Flag_Status set(Flag bit, bool status);


enum Type {
    ARITHMETIC = 0,
    UNARY = 1,
    BIT_OP = 2,
    LOAD = 3,
    STORE = 4,
    JUMP = 5,
    MISC = 6
};

namespace arithmetic {

    enum op {
        ADD = 0,
        ADC = 1,
        AND = 2,
        CP = 3,
        OR = 4,
        SBC = 5,
        SUB = 6,
        XOR = 7
    };

    enum addr_modes {
        SP = -2,
        ADD_16 = -1,
        IMM = 0,
        REG = 1,
        MEM = 2
    };
}

namespace unary {

    enum op {
        INC = 0,
        DEC = 1,
        SWAP = 2,
        RL = 3,
        RLA = 4,
        RLC = 5,
        RLCA = 6,
        RR = 7,
        RRA = 8,
        RRC = 9,
        RRCA = 10,
        SLA = 11,
        SRL = 12,
        SRA = 13
    };

    enum addr_modes {
        REG_16 = -1,
        REG = 0,
        MEM = 1
    };
}

namespace bit_op {
    enum op {
        BIT = 0,
        RES = 1,
        SET = 2
    };

    enum addr_modes {
        REG = 0,
        MEM = 1
    };

}

namespace load {
    enum op {
        NOTHING = 0,
        INCREMENT = 1,
        HIGH_C = 1,
        HIGH_IMM = 2,
        DECREMENT = 2
    };

    enum addr_modes {
        SP = -3,
        REL_16 = -2,
        IMM_16 = -1,
        IMM = 0,
        REG = 1,
        MEM_HL = 2,
        MEM_r16 = 3,
        MEM_DI = 4,
        MEM_IMM = 5
    };

}

namespace store {
    enum op {
        NOTHING = 0,
        INCREMENT = 1,
        HIGH_C = 1,
        HIGH_IMM = 2,
        DECREMENT = 2
    };

    enum addr_modes {
        SP = -1,
        IMM = 0,
        REG = 1,
        MEM_r16 = 2,
        MEM_DI = 3,
        MEM_IMM = 4
    };


}

namespace jump_stack {
    enum op {
        JP = 0,
        JPC = 1,
        CALL = 2,
        CALLC = 3,
        RET = 4,
        RETC = 5,
        RETI = 6,
        PUSH = 7,
        POP = 8
    };

    enum addr_modes {
        PUSH_POP = -1,
        MEM = 0,
        IMM = 1,
        REL = 2,
        RST = 3
    };
}

namespace misc {
    enum op {
        CCF = 0,
        CPL = 1,
        DI = 2,
        EI = 3,
        NOP = 4,
        SCF = 5,
        HALT = 6,
        STOP = 7,
        DAA = 8
    };

    enum addr_modes {
        NOTHING = 0
    };
}


#endif //DMGB_CPU_UTILITY_HPP

//
// Created by drake on 24/9/22.
//

#include "Jump.hpp"
#include "Cpu.hpp"

//TODO: RST
Jump::op_args::op_args() {
    jump_address = 0;
    condition = -1;
}

void Jump::dispatch(vector<Flag_Status> &flags, Cpu *cpu, int op_id, vector<byte> &bytes_fetched, int addr_mode) {
    auto args = Jump::get_args(cpu, bytes_fetched, addr_mode);
    Jump::op_codes[op_id](cpu, args);
}


bool checkCondition(byte F, int cond_id) {
    bool C = F & (1 << Flag::c);
    bool Z = F & (1 << Flag::z);
    switch (cond_id) {
        case 0:
            return !Z;
        case 1:
            return Z;
        case 2:
            return !C;
        case 3:
            return C;
        default:
            return false;
    }
}

void Jump::JP(Cpu *cpu, Jump::op_args &args) { //RST
    cpu->set(DReg::pc, args.jump_address);
}

void Jump::JPC(Cpu *cpu, Jump::op_args &args) {
    if (checkCondition(cpu->get(Reg::f), args.condition))
        Jump::JP(cpu, args);
}

void Jump::CALL(Cpu *cpu, Jump::op_args &args) {
    word next = cpu->get(DReg::pc);
    byte hi = next >> 8, lo = next & 0xFF;
    cpu->push(hi);
    cpu->push(lo);
    Jump::JP(cpu, args);
}

void Jump::CALLC(Cpu *cpu, Jump::op_args &args) {
    if (checkCondition(cpu->get(Reg::f), args.condition))
        Jump::CALL(cpu, args);
}

void Jump::RET(Cpu *cpu, Jump::op_args &args) {
    word lo, hi;
    lo = cpu->pop();
    hi = cpu->pop();
    cpu->set(DReg::pc, lo + (hi << 8));
}

void Jump::RETC(Cpu *cpu, Jump::op_args &args) {
    if (checkCondition(cpu->get(Reg::f), args.condition))
        Jump::RET(cpu, args);
}

void Jump::RETI(Cpu *cpu, Jump::op_args &args) {
    Jump::RET(cpu, args);
    cpu->write(IME, 1);
}

Jump::op_args Jump::get_args(Cpu *cpu, vector<byte> &bytes_fetched, int addressing_mode) {
    Jump::op_args result;
    switch (addressing_mode) {
        case 0: //JP HL
        {
            result.jump_address = cpu->get(DReg::hl);
            break;
        }
        case 1://JP u16
        {
            word address = bytes_fetched[1], hi = bytes_fetched[2];
            address += (hi << 8);
            byte opc = bytes_fetched[0];
            result.condition = 2 * ((opc & 0x10) > 0) + ((opc & 0x08) > 0);
            result.jump_address = address;
            break;
        }
        case 2://JR e8
        {
            auto offset = static_cast<s_byte>(bytes_fetched[1]);
            byte opc = bytes_fetched[0];
            result.condition = 2 * ((opc & 0x10) > 0) + ((opc & 0x08) > 0);
            result.jump_address = cpu->get(DReg::pc) + offset;
            break;
        }
        case 3://RSTs
        {
            byte opc = bytes_fetched[0];
            result.jump_address = opc - 0xC7;
            result.condition = 2 * ((opc & 0x10) > 0) + ((opc & 0x08) > 0);
            break;
        }
        default:
            break;
    }
}
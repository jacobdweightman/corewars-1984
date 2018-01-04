/* Copyright 2018 Jacob Weightman
 *
 * This file is part of corewars-1984.
 *
 * corewars-1984 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * corewars-1984 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Jacob Weightman <jacobdweightman@gmail.com>
 */

#include <stdio.h>
#include <strings.h>

#include "mars.h"

// CORE
int blocks[MEMORY_BLOCKS]; // for memory allocation
opcode core[CORE_SIZE];

program* programs[MEMORY_BLOCKS];
int program_count = 0;

void print_block(int index) {
    int base_index = MAX_PROGRAM_SIZE * index;

    for(int i=0; i<MAX_PROGRAM_SIZE / 8; i++) {
        for(int j=0; j<8; j++) {
            printf("%08x ", core[base_index + 8*i + j]);
        }

        printf("\n");
    }
}

/* Returns a random unsigned integer from /dev/urandom. */
unsigned int randuint() {
    unsigned int val;

    FILE* f = fopen("/dev/urandom", "r");
    fread(&val, sizeof(val), 1, f);
    fclose(f);

    return val;
}

/* Creates an instruction struct and populates its fields with information
 * encoded in the given opcode. */
instruction get_instruction(opcode op) {
    instruction instr;

    instr.type = (op & TYPE_MASK) >> TYPE_OFFSET;
    instr.a_mode = (op & A_MODE_MASK) >> A_MODE_OFFSET;
    instr.b_mode = (op & B_MODE_MASK) >> B_MODE_OFFSET;
    instr.a = (op & A_MASK) >> A_OFFSET;
    instr.b = (op & B_MASK) >> B_OFFSET;

    return instr;
}

/* Loads the given program into core memory. Memory is divided into blocks of
 * MAX_PROGRAM_SIZE, and programs are loaded into a random memory block at a
 * random offset between 0 and MAX_PROGRAM_SIZE - prog->size. */
void load_program(program* prog) {
    int block = 0;
    while(blocks[block] != 0) {
        block = randuint() % MEMORY_BLOCKS;
    }

    blocks[block] = 1;

    int offset = 0; //TODO: randuint() % (MAX_PROGRAM_SIZE - prog->size);

    int base_index = block * MAX_PROGRAM_SIZE + offset;

    for(int i=0; i<prog->size; i++) {
        core[base_index+i] = prog->code[i];
    }

    prog->PC = base_index;

    programs[program_count] = prog;
    program_count++;
}

/* Reads a program from the given file handle into a program struct. */
program read_program(FILE* f) {
    program prog;
    prog.player_id = 0;
    prog.PC = 0;
    prog.size = 0;

    if(f) {
        const int size = sizeof(opcode) / sizeof(uint8_t);
        uint8_t data[sizeof(opcode) / sizeof(uint8_t)];

        // read every 4 bytes into code in proper byte order
        while(fread(data, sizeof(uint8_t), 4, f)) {
            opcode op = 0;

            for(int i=0; i<size; i++) {
                op |= data[i] << (8 * (size - i - 1));
            }

            prog.code[prog.size] = op;
            prog.size++;
        }
    } else {
        printf("You must provide an open file from which to read the program.\n");
        prog.player_id = -1;
    }

    return prog;
}

/* Returns as a signed int the value of an operand from an instruction at the
 * given address, with the given addressing mode, and with the given value.
 * This function assumes the given value occupies only its rightmost 12 bits. */
int get_operand_value(int index, unsigned int mode, unsigned int value) {
    printf("value: %d\n", value);
    // convert value to oridinary full-width signed int
    if(value & (1 << (OPERAND_WIDTH - 1))) { // value is negative (MSB is set)
        value = -0x800 + value;
        printf("negative: %d\n", value);
    }

    switch (mode) {
        case IMMEDIATE_MODE:
            return value;
        case RELATIVE_MODE:
            return core[CORE_WRAP(index+value)];
        case INDIRECT_MODE:
            return core[core[CORE_WRAP(index+value)]];
        default:
            printf("died: invalid addressing mode\n");
            return 0xFFFF;
    }
}

int main() {
    memset(core, 0, sizeof(core));
    memset(blocks, 0, sizeof(blocks));

    /*FILE* f = fopen("test/dwarf.hex", "rb");

    if(f == NULL) {
        fprintf(stderr, "Failed to open file `test/dwarf.hex`\n");
        return 1;
    }*/

    program p = read_program(stdin);
    load_program(&p);

    // fclose(f);

    // run 10 cycles
    for(int i=0; i<3; i++) {
        for(int j=0; j<program_count; j++) {
            int addr = programs[j]->PC;
            instruction instr = get_instruction(core[addr]);

            printf("addr: %d\n", addr+1);
            //printf("value: %x\n", core[addr]);
            printf("A: %d\n", get_operand(addr, instr.a_mode, instr.a));
            printf("B: %d\n", get_operand(addr, instr.b_mode, instr.b));

            switch (instr.type) {
                case MOV_TYPE:
                    printf("MOV\n");
                    break;
                case ADD_TYPE:
                    printf("ADD\n");
                    break;
                case SUB_TYPE:
                    printf("SUB\n");
                    break;
                case JMP_TYPE:
                    printf("JMP\n");
                    break;
                case JMZ_TYPE:
                    printf("JMZ\n");
                    break;
                case DJZ_TYPE:
                    printf("DJZ\n");
                    break;
                case CMP_TYPE:
                    printf("CMP\n");
                    break;
                default:
                    printf("died\n");
            }

            programs[j]->PC += 1;
        }
    }

    //print_block(0);

    return 0;
}
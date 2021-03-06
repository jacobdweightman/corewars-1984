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

#define TEST_BUILD

#include <string.h>

#include "../lib/unity/unity.h"
#include "../src/mars.h"

#define TEST_ASSERT_EQUAL_OPCODE_ARRAY TEST_ASSERT_EQUAL_UINT32_ARRAY

void test_create_mars_1(void) {
    mars m = create_mars(256, 64, 100);
    TEST_ASSERT_EQUAL(256, m.core_size);
    TEST_ASSERT_EQUAL(64, m.block_size);
    TEST_ASSERT_EQUAL(100, m.duration);
    TEST_ASSERT_EQUAL(0, m.elapsed);
    TEST_ASSERT_EQUAL(0, m.alive_count);
    TEST_ASSERT_EQUAL(NULL, m.next_warrior);

    for(unsigned int i=0; i<256; i++) {
        TEST_ASSERT_EQUAL_UINT32(m.core[i], 0);
    }

    for(unsigned int i=0; i<4; i++) {
        TEST_ASSERT_FALSE(m.blocks[i]);
    }

    destroy_mars(&m);
}

void test_create_mars_2(void) {
    mars m = create_mars(21, 5, 50);
    TEST_ASSERT_EQUAL(21, m.core_size);
    TEST_ASSERT_EQUAL(5, m.block_size);
    TEST_ASSERT_EQUAL(50, m.duration);
    TEST_ASSERT_EQUAL(0, m.elapsed);
    TEST_ASSERT_EQUAL(0, m.alive_count);
    TEST_ASSERT_EQUAL(NULL, m.next_warrior);

    for(unsigned int i=0; i<21; i++) {
        TEST_ASSERT_EQUAL_UINT32(m.core[i], 0);
    }

    for(unsigned int i=0; i<4; i++) { // last, partial block doesn't count!
        TEST_ASSERT_FALSE(m.blocks[i]);
    }

    destroy_mars(&m);
}

void test_insert_warrior_empty(void) {
    mars m;
    warrior a;

    insert_warrior(&m, &a);

    // verify traversal order is a, a, ...
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
}

void test_insert_warrior(void) {
    mars m;
    warrior a, b, c, d;

    // insert, verify that newly inserted is always next to run
    insert_warrior(&m, &a);
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
    insert_warrior(&m, &b);
    TEST_ASSERT_EQUAL(&b, m.next_warrior);
    insert_warrior(&m, &c);
    TEST_ASSERT_EQUAL(&c, m.next_warrior);
    insert_warrior(&m, &d);
    TEST_ASSERT_EQUAL(&d, m.next_warrior);

    // move from last inserted back to first
    m.next_warrior = m.next_warrior->next;

    // verify traversal order is a, b, c, d, a, ...
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&b, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&c, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&d, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
}

void test_remove_warrior_middle(void) {
    mars m;
    warrior a, b, c, d;

    insert_warrior(&m, &a);
    insert_warrior(&m, &b);
    insert_warrior(&m, &c);
    insert_warrior(&m, &d);

    // remove warrior that is not next to run
    remove_warrior(&m, &b);

    // verify sequence changes from d, a, b, c --> d, a, c
    TEST_ASSERT_EQUAL(&d, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&c, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&d, m.next_warrior);
}

void test_remove_warrior_next(void) {
    mars m;
    warrior a, b, c, d;

    insert_warrior(&m, &a);
    insert_warrior(&m, &b);
    insert_warrior(&m, &c);
    insert_warrior(&m, &d);

    remove_warrior(&m, &d);

    // verify d, a, b, c --> a, b, c
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&b, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&c, m.next_warrior);
    m.next_warrior = m.next_warrior->next;
    TEST_ASSERT_EQUAL(&a, m.next_warrior);
}

void test_remove_warrior_only(void) {
    mars m;
    warrior a;

    insert_warrior(&m, &a);

    remove_warrior(&m, &a);

    TEST_ASSERT_EQUAL(NULL, m.next_warrior);
}

void test_load_program(void) {
    mars m = create_mars(10, 5, 100);
    opcode* buf = (opcode*) "\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04";
    program prog = prog_from_buffer(5, buf, 3);

    for(unsigned int i=0; i<m.core_size; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, m.core[i]);
    }

    load_program(&m, &prog, 0, 0);
    load_program(&m, &prog, 1, 2);

    warrior* w = m.next_warrior;
    TEST_ASSERT_EQUAL(5, w->id);
    TEST_ASSERT_EQUAL(7, w->PC);
    w = w->next;
    TEST_ASSERT_EQUAL(5, w->id);
    TEST_ASSERT_EQUAL(0, w->PC);


    TEST_ASSERT_EQUAL(0x0c0d0e0f, m.core[0]);
    TEST_ASSERT_EQUAL(0x08090a0b, m.core[1]);
    TEST_ASSERT_EQUAL(0x04050607, m.core[2]);
    TEST_ASSERT_EQUAL(0x00000000, m.core[3]);
    TEST_ASSERT_EQUAL(0x00000000, m.core[4]);
    TEST_ASSERT_EQUAL(0x00000000, m.core[5]);
    TEST_ASSERT_EQUAL(0x00000000, m.core[6]);
    TEST_ASSERT_EQUAL(0x0c0d0e0f, m.core[7]);
    TEST_ASSERT_EQUAL(0x08090a0b, m.core[8]);
    TEST_ASSERT_EQUAL(0x04050607, m.core[9]);

    destroy_program(&prog);
    destroy_mars(&m);
}

void test_get_operand_value(void) {
    mars m = create_mars(10, 5, 100);
    m.core[0] = (opcode) -5;
    m.core[1] = (opcode) -4;
    m.core[2] = (opcode) -3;
    m.core[3] = (opcode) -2;
    m.core[4] = (opcode) -1;
    m.core[5] = 0;
    m.core[6] = 1;
    m.core[7] = 2;
    m.core[8] = 3;
    m.core[9] = 4;

    TEST_ASSERT_EQUAL(5, get_operand_value(&m, 5, IMMEDIATE_MODE, 0x0005));
    TEST_ASSERT_EQUAL(260, get_operand_value(&m, 2, IMMEDIATE_MODE, 0x0104));
    TEST_ASSERT_EQUAL(-1, get_operand_value(&m, 9, IMMEDIATE_MODE, 0x0FFF));
    TEST_ASSERT_EQUAL(-256, get_operand_value(&m, 6, IMMEDIATE_MODE, 0x0F00));

    TEST_ASSERT_EQUAL(0, get_operand_value(&m, 3, RELATIVE_MODE, 0x0002));
    TEST_ASSERT_EQUAL(-1, get_operand_value(&m, 7, RELATIVE_MODE, 0x0FFD));
    TEST_ASSERT_EQUAL(-4, get_operand_value(&m, 8, RELATIVE_MODE, 0x0003)); // roll over top
    TEST_ASSERT_EQUAL(3, get_operand_value(&m, 2, RELATIVE_MODE, 0x0FFC)); // roll under bottom

    TEST_ASSERT_EQUAL(0, get_operand_value(&m, 4, INDIRECT_MODE, 0x0002));
    TEST_ASSERT_EQUAL(-5, get_operand_value(&m, 6, INDIRECT_MODE, 0x03));
    TEST_ASSERT_EQUAL(-4, get_operand_value(&m, 9, INDIRECT_MODE, 0x0FFE)); // roll over top
    TEST_ASSERT_EQUAL(1, get_operand_value(&m, 1, INDIRECT_MODE, 0x0FFF)); // roll under bottom

    destroy_mars(&m);
}

void test_get_operand_address(void) {
    mars m = create_mars(10, 5, 100);
    m.core[0] = (opcode) -5;
    m.core[1] = (opcode) -4;
    m.core[2] = (opcode) -3;
    m.core[3] = (opcode) -2;
    m.core[4] = (opcode) -1;
    m.core[5] = 0;
    m.core[6] = 1;
    m.core[7] = 2;
    m.core[8] = 3;
    m.core[9] = 4;

    TEST_ASSERT_EQUAL(8, get_operand_address(&m, 6, RELATIVE_MODE, 0x0002));
    TEST_ASSERT_EQUAL(1, get_operand_address(&m, 4, RELATIVE_MODE, 0x0FFD));
    TEST_ASSERT_EQUAL(0, get_operand_address(&m, 9, RELATIVE_MODE, 0x0001));
    TEST_ASSERT_EQUAL(9, get_operand_address(&m, 8, RELATIVE_MODE, 0x0001));
    TEST_ASSERT_EQUAL(8, get_operand_address(&m, 2, RELATIVE_MODE, 0x0FFC));

    TEST_ASSERT_EQUAL(9, get_operand_address(&m, 6, INDIRECT_MODE, 0x0002));
    TEST_ASSERT_EQUAL(0, get_operand_address(&m, 4, INDIRECT_MODE, 0x0FFD));
    TEST_ASSERT_EQUAL(4, get_operand_address(&m, 9, INDIRECT_MODE, 0x0001));
    TEST_ASSERT_EQUAL(2, get_operand_address(&m, 8, INDIRECT_MODE, 0x0001));
    TEST_ASSERT_EQUAL(7, get_operand_address(&m, 3, INDIRECT_MODE, 0x0FFC));

    destroy_mars(&m);
}

// TESTS FOR MOV
void test_mov_immediate_relative(void) {
    TEST_IGNORE();
}

void test_mov_immediate_indirect(void) {
    TEST_IGNORE();
}

void test_mov_relative_relative(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    // no address wrapping
    m.core[0] = 0x15004002; // MOV 4 2
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x00000002; // DAT 2
    m.core[3] = 0x00000003; // DAT 3
    m.core[4] = 0x12345678; // DAT 4

    w.PC = 0;
    tick(&m);

    TEST_ASSERT_EQUAL(0x15004002, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[1]);
    TEST_ASSERT_EQUAL(0x12345678, m.core[2]);
    TEST_ASSERT_EQUAL(0x00000003, m.core[3]);
    TEST_ASSERT_EQUAL(0x12345678, m.core[4]);

    // with address wrapping
    m.core[0] = 0x00000000; // DAT 0
    m.core[1] = 0x1500AFFF; // MOV 10 -1
    m.core[2] = 0x00000002; // DAT 2
    m.core[3] = 0x00000003; // DAT 3
    m.core[4] = 0x00000004; // DAT 4

    w.PC = 1;
    tick(&m);

    TEST_ASSERT_EQUAL(0x1500AFFF, m.core[0]);
    TEST_ASSERT_EQUAL(0x1500AFFF, m.core[1]);
    TEST_ASSERT_EQUAL(2, m.core[2]);
    TEST_ASSERT_EQUAL(3, m.core[3]);
    TEST_ASSERT_EQUAL(4, m.core[4]);

    destroy_mars(&m);
}

void test_mov_indirect_relative(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    // no address wrapping
    m.core[0] = 0x00000000; // DAT 0
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x19FFF002; // MOV @-1 2
    m.core[3] = 0x00000003; // DAT 3
    m.core[4] = 0x00000004; // DAT 4

    w.PC = 2;
    tick(&m);

    TEST_ASSERT_EQUAL(0, m.core[0]);
    TEST_ASSERT_EQUAL(1, m.core[1]);
    TEST_ASSERT_EQUAL(0x19FFF002, m.core[2]);
    TEST_ASSERT_EQUAL(3, m.core[3]);
    TEST_ASSERT_EQUAL(3, m.core[4]);

    // with address wrapping
    m.core[0] = 0x00000000; // DAT 0
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x1900C006; // MOV @12 6
    m.core[3] = 0x00000003; // DAT 3
    m.core[4] = 0x00000004; // DAT 4

    w.PC = 2;
    tick(&m);

    TEST_ASSERT_EQUAL(0, m.core[0]);
    TEST_ASSERT_EQUAL(1, m.core[1]);
    TEST_ASSERT_EQUAL(0x1900C006, m.core[2]);
    TEST_ASSERT_EQUAL(1, m.core[3]);
    TEST_ASSERT_EQUAL(4, m.core[4]);

    destroy_mars(&m);
}

void test_mov_relative_indirect(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    // no address wrapping
    m.core[0] = 0x00000002; // DAT 2
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x16001FFE; // MOV 1 @-2 (3, #-1 --> @-2, 2, #4)
    m.core[3] = 0xFFFFFFFF; // DAT -1
    m.core[4] = 0x00000004; // DAT 4

    w.PC = 2;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[1]);
    TEST_ASSERT_EQUAL(0x16001FFE, m.core[2]);
    TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[3]);
    TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[4]); // should be -1

    // with address wrapping
    m.core[0] = 0;          // DAT 0
    m.core[1] = 1;          // DAT 1
    m.core[2] = 3;          // DAT -1
    m.core[3] = 0x16008FFC; // MOV 8 @-4
    m.core[4] = 4;          // DAT 4

    w.PC = 3;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000000, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[1]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[2]);
    TEST_ASSERT_EQUAL(0x16008FFC, m.core[3]);
    TEST_ASSERT_EQUAL(0x00000004, m.core[4]);

    destroy_mars(&m);
}

void test_mov_indirect_indirect(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    // no address wrapping
    m.core[0] = 0x00000002; // DAT 2
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x1A001FFE; // MOV @1 @-2
    m.core[3] = 0xFFFFFFFF; // DAT -1
    m.core[4] = 0x00000004; // DAT 4

    w.PC = 2;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[1]);
    TEST_ASSERT_EQUAL(0x1A001FFE, m.core[2]);
    TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[3]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[4]);

    // with address wrapping
    m.core[0] = 0x00000002; // DAT 2
    m.core[1] = 0xFFFFFFFF; // DAT -1
    m.core[2] = 0x1AFFFFFE; // MOV @1 @-2
    m.core[3] = 0x1A003FF7; // DAT @3 @-9
    m.core[4] = 0x00000003; // DAT 3

    w.PC = 3;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0x1AFFFFFE, m.core[1]);
    TEST_ASSERT_EQUAL(0x1AFFFFFE, m.core[2]);
    TEST_ASSERT_EQUAL(0x1A003FF7, m.core[3]);
    TEST_ASSERT_EQUAL(0x00000003, m.core[4]);

    destroy_mars(&m);
}

// TESTS FOR ADD
void test_add_immediate_indirect(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    // no address wrapping
    m.core[0] = 0xFEDCBA98; // DAT -19088744
    m.core[1] = 0x00000104; // DAT 260
    m.core[2] = 0x220FF001; // ADD #255 @1
    m.core[3] = 0xFFFFFFFF; // DAT -1
    m.core[4] = 0x00000104; // DAT 260

    w.PC = 2;
    tick(&m);

    TEST_ASSERT_EQUAL(0xFEDCBA98, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000203, m.core[1]);
    TEST_ASSERT_EQUAL(0x220FF001, m.core[2]);
    TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[3]);
    TEST_ASSERT_EQUAL(0x00000104, m.core[4]);

    // with address wrapping
    m.core[0] = 0xFEDCBA98; // DAT -19088744
    m.core[1] = 0x00000001; // DAT 1
    m.core[2] = 0x220FF001; // ADD #255 @1
    m.core[3] = 0x22004FFC; // ADD #4 @-4
    m.core[4] = 0x0000000A; // DAT 10

    w.PC = 3;
    tick(&m);

    TEST_ASSERT_EQUAL(0xFEDCBA98, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000001, m.core[1]);
    TEST_ASSERT_EQUAL(0x220FF001, m.core[2]);
    TEST_ASSERT_EQUAL(0x22005000, m.core[3]);
    TEST_ASSERT_EQUAL(0x0000000A, m.core[4]);

    destroy_mars(&m);
}

void test_add_immediate_relative(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  // no address wrapping
  m.core[0] = 0x10001000; // DAT 268435456
  m.core[1] = 0x00000104; // DAT 260
  m.core[2] = 0x220FF001; // ADD #255 @1
  m.core[3] = 0xFFFFFFFD; // DAT -3
  m.core[4] = 0x21FF0FFC; // ADD #-16 -4

  w.PC = 4;
  tick(&m);

  TEST_ASSERT_EQUAL(0x10000FF0, m.core[0]);
  TEST_ASSERT_EQUAL(0x00000104, m.core[1]);
  TEST_ASSERT_EQUAL(0x220FF001, m.core[2]);
  TEST_ASSERT_EQUAL(0xFFFFFFFD, m.core[3]);
  TEST_ASSERT_EQUAL(0x21FF0FFC, m.core[4]);

  // with address wrapping
  m.core[0] = 0x2100AFEF; // ADD #10 -17
  m.core[1] = 0x00000104; // DAT 260
  m.core[2] = 0x220FF001; // ADD #255 @1
  m.core[3] = 0xFFFFFFFD; // DAT -3
  m.core[4] = 0x21FF0FFC; // ADD #-16 -4

  w.PC = 0;
  tick(&m);

  TEST_ASSERT_EQUAL(0x2100AFEF, m.core[0]);
  TEST_ASSERT_EQUAL(0x00000104, m.core[1]);
  TEST_ASSERT_EQUAL(0x220FF001, m.core[2]);
  TEST_ASSERT_EQUAL(0x00000007, m.core[3]);
  TEST_ASSERT_EQUAL(0x21FF0FFC, m.core[4]);

  destroy_mars(&m);
}

void test_add_relative_relative(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  // no address wrapping
  m.core[0] = 0x10001100; // DAT 268435456
  m.core[1] = 0x00000104; // DAT 260
  m.core[2] = 0x25FFEFFF; // ADD -2 -1
  m.core[3] = 0xFFFFFFFD; // DAT -3
  m.core[4] = 0x21FF0FFC; // ADD #-16 -4

  w.PC = 2;
  tick(&m);

  TEST_ASSERT_EQUAL(0x10001100, m.core[0]);
  TEST_ASSERT_EQUAL(0x10001204, m.core[1]);
  TEST_ASSERT_EQUAL(0x25FFEFFF, m.core[2]);
  TEST_ASSERT_EQUAL(0xFFFFFFFD, m.core[3]);
  TEST_ASSERT_EQUAL(0x21FF0FFC, m.core[4]);

  // with address wrapping
  m.core[0] = 0x25015010; // ADD 21 16
  m.core[1] = 0x000A0104; // DAT 655620
  m.core[2] = 0x220FF001; // ADD #255 @1
  m.core[3] = 0xFFFFFFFD; // DAT -3
  m.core[4] = 0x21FF0FFC; // ADD #-16 -4

  w.PC = 0;
  tick(&m);

  TEST_ASSERT_EQUAL(0x25015010, m.core[0]);
  TEST_ASSERT_EQUAL(0x00140208, m.core[1]);
  TEST_ASSERT_EQUAL(0x220FF001, m.core[2]);
  TEST_ASSERT_EQUAL(0xFFFFFFFD, m.core[3]);
  TEST_ASSERT_EQUAL(0x21FF0FFC, m.core[4]);

  destroy_mars(&m);
}

void test_add_indirect_relative(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  // no address wrapping
  m.core[0] = 0x00000002; // DAT 1
  m.core[1] = 0x29FFF001; // ADD @-1 1
  m.core[2] = 0xFFFFFFFE; // DAT -2
  m.core[3] = 0x00000002; // DAT
  m.core[4] = 0x00000000; // DAT 0

  w.PC = 1;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
  TEST_ASSERT_EQUAL(0x29FFF001, m.core[1]);
  TEST_ASSERT_EQUAL(0x00000000, m.core[2]);
  TEST_ASSERT_EQUAL(0x00000002, m.core[3]);
  TEST_ASSERT_EQUAL(0x00000000, m.core[4]);

  // with address wrapping
  m.core[0] = 0x00000001; // DAT 1
  m.core[1] = 0xFFFFFFFE; // DAT -2
  m.core[2] = 0x00000002; // DAT 2
  m.core[3] = 0x29FF0005; // ADD @-16 5
  m.core[4] = 0x00000000; // DAT 0

  w.PC = 3;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000001, m.core[0]);
  TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[1]);
  TEST_ASSERT_EQUAL(0x00000002, m.core[2]);
  TEST_ASSERT_EQUAL(0x29FF0006, m.core[3]);
  TEST_ASSERT_EQUAL(0x00000000, m.core[4]);

  destroy_mars(&m);
}

void test_add_relative_indirect(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  m.core[0] = 0x00000002; // DAT 1
  m.core[1] = 0x26FFF006; // ADD -1 @6
  m.core[2] = 0xFFFFFFFE; // DAT -2
  m.core[3] = 0x00000002; // DAT 2
  m.core[4] = 0x00000001; // DAT 1

  w.PC = 1;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
  TEST_ASSERT_EQUAL(0x26FFF006, m.core[1]);
  TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[2]);
  TEST_ASSERT_EQUAL(0x00000002, m.core[3]);
  TEST_ASSERT_EQUAL(0x00000003, m.core[4]);

  destroy_mars(&m);
}

void test_add_indirect_indirect(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  m.core[0] = 0x00000002; // DAT 1
  m.core[1] = 0x2AFFF008; // ADD @-1 @8
  m.core[2] = 0xFFFFFFFE; // DAT -2
  m.core[3] = 0x00000004; // DAT 4
  m.core[4] = 0x00000001; // DAT 1

  w.PC = 1;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
  TEST_ASSERT_EQUAL(0x2AFFF008, m.core[1]);
  TEST_ASSERT_EQUAL(0x00000002, m.core[2]);
  TEST_ASSERT_EQUAL(0x00000004, m.core[3]);
  TEST_ASSERT_EQUAL(0x00000001, m.core[4]);

  destroy_mars(&m);
}

// TESTS FOR SUB
void test_sub_immediate_indirect(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    m.core[0] = 0x00000002; // DAT 1
    m.core[1] = 0x32003001; // SUB #3 @1
    m.core[2] = 0xFFFFFFFE; // DAT -2
    m.core[3] = 0x00000004; // DAT 4
    m.core[4] = 0x10101018; // DAT 269488152

    w.PC = 1;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0x32003001, m.core[1]);
    TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[2]);
    TEST_ASSERT_EQUAL(0x00000004, m.core[3]);
    TEST_ASSERT_EQUAL(0x10101015, m.core[4]);

    destroy_mars(&m);
}

void test_sub_immediate_relative(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    m.core[0] = 0x00000002; // DAT 1
    m.core[1] = 0x00000014; // DAT 20
    m.core[2] = 0xFFFFFFFE; // DAT -2
    m.core[3] = 0x31015003; // SUB #3 3
    m.core[4] = 0x10101018; // DAT 269488152

    w.PC = 3;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[1]);
    TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[2]);
    TEST_ASSERT_EQUAL(0x31015003, m.core[3]);
    TEST_ASSERT_EQUAL(0x10101018, m.core[4]);

    destroy_mars(&m);
}

void test_sub_relative_relative(void) {
    mars m = create_mars(5, 5, 100);
    warrior w;
    insert_warrior(&m, &w);

    m.core[0] = 0x00000002; // DAT 1
    m.core[1] = 0x00000014; // DAT 20
    m.core[2] = 0xFFFFFFFE; // DAT -2
    m.core[3] = 0x10101018; // DAT 269488152
    m.core[4] = 0x35FF8009; // SUB -8 9

    w.PC = 4;
    tick(&m);

    TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
    TEST_ASSERT_EQUAL(0x00000014, m.core[1]);
    TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[2]);
    TEST_ASSERT_EQUAL(0x10101004, m.core[3]);
    TEST_ASSERT_EQUAL(0x35FF8009, m.core[4]);

    destroy_mars(&m);
}

void test_sub_indirect_relative(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  m.core[0] = 0x39006FFE; // SUB @6 -2
  m.core[1] = 0x00000004; // DAT 4
  m.core[2] = 0xFFFFFFFE; // DAT -2
  m.core[3] = 0x10101018; // DAT 269488152
  m.core[4] = 0xFFFFFFFC; // DAT -4

  w.PC = 0;
  tick(&m);

  TEST_ASSERT_EQUAL(0x39006FFE, m.core[0]);
  TEST_ASSERT_EQUAL(0x00000004, m.core[1]);
  TEST_ASSERT_EQUAL(0xFFFFFFFE, m.core[2]);
  TEST_ASSERT_EQUAL(0x1010101C, m.core[3]);
  TEST_ASSERT_EQUAL(0xFFFFFFFC, m.core[4]);

  destroy_mars(&m);
}

void test_sub_relative_indirect(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  m.core[0] = 0x00000004; // DAT 4
  m.core[1] = 0x36006FFE; // SUB 6 @-2
  m.core[2] = 0x12345678; // DAT 0x12345678
  m.core[3] = 0x10101018; // DAT 269488152
  m.core[4] = 0xFFFFFFFC; // DAT -4

  w.PC = 1;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000004, m.core[0]);
  TEST_ASSERT_EQUAL(0x36006FFE, m.core[1]);
  TEST_ASSERT_EQUAL(0x00000000, m.core[2]);
  TEST_ASSERT_EQUAL(0x10101018, m.core[3]);
  TEST_ASSERT_EQUAL(0xFFFFFFFC, m.core[4]);

  destroy_mars(&m);
}

void test_sub_indirect_indirect(void) {
  mars m = create_mars(5, 5, 100);
  warrior w;
  insert_warrior(&m, &w);

  m.core[0] = 0x00000002; // DAT 2
  m.core[1] = 0x36006FFE; // SUB 6 @-2
  m.core[2] = 0x3AFFE001; // SUB @-2 @1
  m.core[3] = 0xFFFFFFFF; // DAT 269488152
  m.core[4] = 0x00000001; // DAT 1

  w.PC = 2;
  tick(&m);

  TEST_ASSERT_EQUAL(0x00000002, m.core[0]);
  TEST_ASSERT_EQUAL(0x36006FFD, m.core[1]);
  TEST_ASSERT_EQUAL(0x3AFFE001, m.core[2]);
  TEST_ASSERT_EQUAL(0xFFFFFFFF, m.core[3]);
  TEST_ASSERT_EQUAL(0x00000001, m.core[4]);

  destroy_mars(&m);
}

// TESTS FOR JMP
void test_jmp_relative(void) {
    TEST_IGNORE();
}

void test_jmp_indirect(void) {
    TEST_IGNORE();
}

// TESTS FOR JMZ
void test_jmz_immediate_indirect(void) {
    TEST_IGNORE();
}

void test_jmz_immediate_relative(void) {
    TEST_IGNORE();
}

void test_jmz_relative_relative(void) {
    TEST_IGNORE();
}

void test_jmz_indirect_relative(void) {
    TEST_IGNORE();
}

void test_jmz_relative_indirect(void) {
    TEST_IGNORE();
}

void test_jmz_indirect_indirect(void) {
    TEST_IGNORE();
}

// TESTS FOR DJZ
void test_djz_relative_relative(void) {
    TEST_IGNORE();
}

void test_djz_indirect_relative(void) {
    TEST_IGNORE();
}

void test_djz_relative_indirect(void) {
    TEST_IGNORE();
}

void test_djz_indirect_indirect(void) {
    TEST_IGNORE();
}

// TESTS FOR CMP
void test_cmp_immediate_immediate(void) {
  TEST_IGNORE();
}

void test_cmp_immediate_indirect(void) {
    TEST_IGNORE();
}

void test_cmp_immediate_relative(void) {
    TEST_IGNORE();
}

void test_cmp_relative_immediate(void) {
  TEST_IGNORE();
}

void test_cmp_relative_relative(void) {
    TEST_IGNORE();
}

void test_cmp_relative_indirect(void) {
    TEST_IGNORE();
}

void test_cmp_indirect_immediate(void) {
    TEST_IGNORE();
}

void test_cmp_indirect_relative(void) {
    TEST_IGNORE();
}

void test_cmp_indirect_indirect(void) {
    TEST_IGNORE();
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_create_mars_1);
    RUN_TEST(test_create_mars_2);
    RUN_TEST(test_insert_warrior_empty);
    RUN_TEST(test_insert_warrior);
    RUN_TEST(test_remove_warrior_middle);
    RUN_TEST(test_remove_warrior_next);
    RUN_TEST(test_remove_warrior_only);
    RUN_TEST(test_load_program);
    RUN_TEST(test_get_operand_value);
    RUN_TEST(test_get_operand_address);
    RUN_TEST(test_mov_immediate_relative);
    RUN_TEST(test_mov_immediate_indirect);
    RUN_TEST(test_mov_relative_relative);
    RUN_TEST(test_mov_indirect_relative);
    RUN_TEST(test_mov_relative_indirect);
    RUN_TEST(test_mov_indirect_indirect);
    RUN_TEST(test_add_immediate_relative);
    RUN_TEST(test_add_immediate_indirect);
    RUN_TEST(test_add_relative_relative);
    RUN_TEST(test_add_indirect_relative);
    RUN_TEST(test_add_relative_indirect);
    RUN_TEST(test_add_indirect_indirect);
    RUN_TEST(test_sub_immediate_relative);
    RUN_TEST(test_sub_immediate_indirect);
    RUN_TEST(test_sub_relative_relative);
    RUN_TEST(test_sub_indirect_relative);
    RUN_TEST(test_sub_relative_indirect);
    RUN_TEST(test_sub_indirect_indirect);
    RUN_TEST(test_jmp_relative);
    RUN_TEST(test_jmp_indirect);
    RUN_TEST(test_jmz_immediate_relative);
    RUN_TEST(test_jmz_immediate_indirect);
    RUN_TEST(test_jmz_relative_relative);
    RUN_TEST(test_jmz_indirect_relative);
    RUN_TEST(test_jmz_relative_indirect);
    RUN_TEST(test_jmz_indirect_indirect);
    RUN_TEST(test_djz_relative_relative);
    RUN_TEST(test_djz_indirect_relative);
    RUN_TEST(test_djz_relative_indirect);
    RUN_TEST(test_djz_indirect_indirect);
    RUN_TEST(test_cmp_immediate_immediate);
    RUN_TEST(test_cmp_immediate_relative);
    RUN_TEST(test_cmp_immediate_indirect);
    RUN_TEST(test_cmp_relative_immediate);
    RUN_TEST(test_cmp_relative_relative);
    RUN_TEST(test_cmp_relative_indirect);
    RUN_TEST(test_cmp_indirect_immediate);
    RUN_TEST(test_cmp_indirect_relative);
    RUN_TEST(test_cmp_indirect_indirect);
    UNITY_END();

    return 0;
}

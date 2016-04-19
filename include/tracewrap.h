#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

#include "frame.piqi.pb-c.h"


void qemu_trace_init(const char *file, int argc, char **argv);
void qemu_trace_newframe(target_ulong addr, int tread_id);
void qemu_trace_add_operand(OperandInfo *oi, int inout);
void qemu_trace_endframe(CPUArchState *env, target_ulong pc, target_ulong size);
void qemu_trace_finish(uint32_t exit_code);

OperandInfo * load_store_reg(target_ulong reg, target_ulong val, int ls);
OperandInfo * load_store_mem(target_ulong addr, target_ulong val, int ls, int len);

#define REG_EFLAGS 66
#define REG_LO 33
#define REG_HI 34

#define REG_CPSR 64
#define REG_APSR 65
#define REG_SP 13
#define REG_LR 14
#define REG_PC 15

#define REG_NF 94
#define REG_ZF 95
#define REG_CF 96
#define REG_VF 97
#define REG_QF 98
#define REG_GE 99

#define SEG_BIT 8

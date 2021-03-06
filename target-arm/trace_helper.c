#include <stdint.h>

#include "cpu.h"
//#include "exec/def-helper.h"
#include "helper.h"
#include "tracewrap.h"
#include "qemu/log.h"
#include "tcg.h"

uint32_t HELPER(trace_cpsr_read)(CPUARMState *env) {
    uint32_t res = cpsr_read(env) & ~CPSR_EXEC;
    OperandInfo * oi = load_store_reg(REG_CPSR, res, 0);
    qemu_trace_add_operand(oi, 0x1);
    return res;
}

void HELPER(trace_cpsr_write)(CPUARMState *env, uint32_t val, uint32_t mask) {
    OperandInfo * oi = load_store_reg(REG_CPSR, val, 1);
    qemu_trace_add_operand(oi, 0x2);
}

void HELPER(trace_newframe)(target_ulong pc) {
    qemu_trace_newframe(pc, 0);
}

void HELPER(trace_endframe)(CPUARMState *env, target_ulong old_pc, uint32_t size) {
    qemu_trace_endframe(env, old_pc, size);
}

OperandInfo * load_store_mem(uint32_t addr, uint32_t val, int ls, int len) {
    MemOperand * mo = g_new(MemOperand, 1);
    mem_operand__init(mo);

    mo->address = addr;

    OperandInfoSpecific *ois = g_new(OperandInfoSpecific, 1);
    operand_info_specific__init(ois);
    ois->mem_operand = mo;

    OperandUsage *ou = g_new(OperandUsage, 1);
    operand_usage__init(ou);
    if (ls == 0) {
        ou->read = 1;
    } else {
        ou->written = 1;
    }
    OperandInfo *oi = g_new(OperandInfo, 1);
    operand_info__init(oi);
    oi->bit_length = len * 8;
    oi->operand_info_specific = ois;
    oi->operand_usage = ou;
    oi->value.len = len;
    oi->value.data = g_malloc(oi->value.len);
    memcpy(oi->value.data, &val, len);
    return oi;
}

OperandInfo * load_store_reg(uint32_t reg, uint32_t val, int ls) {
    RegOperand * ro = g_new(RegOperand, 1);
    reg_operand__init(ro);

	switch (reg) {
	case REG_SP: ro->name = g_strdup("SP"); break;
	case REG_LR: ro->name = g_strdup("LR"); break;
	case REG_PC: ro->name = g_strdup("PC"); break;
	case REG_NF: ro->name = g_strdup("NF"); break;
	case REG_ZF: ro->name = g_strdup("ZF"); break;
	case REG_CF: ro->name = g_strdup("CF"); break;
	case REG_VF: ro->name = g_strdup("VF"); break;
	case REG_QF: ro->name = g_strdup("QF"); break;
	case REG_GE: ro->name = g_strdup("GE"); break;
	default: ro->name = g_strdup_printf("R%d", reg); break;
	}

    OperandInfoSpecific *ois = g_new(OperandInfoSpecific, 1);
    operand_info_specific__init(ois);
    ois->reg_operand = ro;

    OperandUsage *ou = g_new(OperandUsage, 1);
    operand_usage__init(ou);
    if (ls == 0) {
        ou->read = 1;
    } else {
        ou->written = 1;
    }
    OperandInfo *oi = g_new(OperandInfo, 1);
    operand_info__init(oi);
    oi->bit_length = 0;
    oi->operand_info_specific = ois;
    oi->operand_usage = ou;
    oi->value.len = 4;
    oi->value.data = g_malloc(oi->value.len);
    memcpy(oi->value.data, &val, 4);

    return oi;
}

void HELPER(log_store_cpsr)(CPUARMState *env)
{
    OperandInfo *oi;
    uint32_t val = cpsr_read(env);

	oi = load_store_reg(REG_NF, (val >> 31) & 0x1, 1);
    qemu_trace_add_operand(oi, 0x2);

	oi = load_store_reg(REG_ZF, (val >> 30) & 0x1, 1);
    qemu_trace_add_operand(oi, 0x2);

	oi = load_store_reg(REG_CF, (val >> 29) & 0x1, 1);
    qemu_trace_add_operand(oi, 0x2);

	oi = load_store_reg(REG_VF, (val >> 28) & 0x1, 1);
    qemu_trace_add_operand(oi, 0x2);

	oi = load_store_reg(REG_QF, (val >> 27) & 0x1, 1);
    qemu_trace_add_operand(oi, 0x2);

	oi = load_store_reg(REG_GE, (val >> 19) & 0xF, 1);
    qemu_trace_add_operand(oi, 0x2);
}

void HELPER(log_read_cpsr)(CPUARMState *env)
{
    OperandInfo *oi;
    uint32_t val = cpsr_read(env);

	oi = load_store_reg(REG_NF, (val >> 31) & 0x1, 0);
    qemu_trace_add_operand(oi, 0x1);

	oi = load_store_reg(REG_ZF, (val >> 30) & 0x1, 0);
    qemu_trace_add_operand(oi, 0x1);

	oi = load_store_reg(REG_CF, (val >> 29) & 0x1, 0);
    qemu_trace_add_operand(oi, 0x1);

	oi = load_store_reg(REG_VF, (val >> 28) & 0x1, 0);
    qemu_trace_add_operand(oi, 0x1);

	oi = load_store_reg(REG_QF, (val >> 27) & 0x1, 0);
    qemu_trace_add_operand(oi, 0x1);

	oi = load_store_reg(REG_GE, (val >> 19) & 0xF, 0);
    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_load_reg)(uint32_t reg, uint32_t val)
{
    qemu_log("This register (r%d) was read. Value 0x%x\n", reg, val);

    OperandInfo *oi = load_store_reg(reg, val, 0);

    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_store_reg)(uint32_t reg, uint32_t val)
{
    qemu_log("This register (r%d) was written. Value: 0x%x\n", reg, val);

    OperandInfo *oi = load_store_reg(reg, val, 1);

    qemu_trace_add_operand(oi, 0x2);
}

void HELPER(trace_ld)(CPUARMState *env, uint32_t val, uint32_t addr, uint32_t opc)
{
	int len;
    qemu_log("This was a read 0x%x addr:0x%x value:0x%x\n", env->regs[15], addr, val);
	switch (opc & MO_SIZE) {
	case MO_8:
        len = 1;
        break;
	case MO_16:
        len = 2;
        break;
	case MO_32:
        len = 4;
        break;
	case MO_64:
        len = 8;
        break;
	default:
        qemu_log("Do not reach\n");
	}
    OperandInfo *oi = load_store_mem(addr, val, 0, len);

    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_st)(CPUARMState *env, uint32_t val, uint32_t addr, uint32_t opc)
{
    int len;
    qemu_log("This was a store 0x%x addr:0x%x value:0x%x\n", env->regs[15], addr, val);

	switch (opc & MO_SIZE) {
	case MO_8:
        len = 1;
        break;
	case MO_16:
        len = 2;
        break;
	case MO_32:
        len = 4;
        break;
	case MO_64:
        len = 8;
        break;
	default:
        qemu_log("Do not reach\n");
	}
    OperandInfo *oi = load_store_mem(addr, val, 1, len);

    qemu_trace_add_operand(oi, 0x2);
}

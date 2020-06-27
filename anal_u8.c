/* radare nX-U8/100 analysis plugin - LGPL - Copyright 2020 - cetus9 */

#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>

#include "u8_disas.h"

// u8inst[U8_INS_NUM] contains instruction data

// analyse opcodes
static int u8_anop(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *buf, int len, RAnalOpMask mask)
{
	int ret;
	struct u8_cmd cmd;

	memset (op, '\0', sizeof(RAnalOp));
	op->size = -1;
	op->addr = addr;
	op->type = R_ANAL_OP_TYPE_UNK;
	op->family = R_ANAL_OP_FAMILY_CPU;
	op->stackop = R_ANAL_STACK_NULL;
	op->jump = op->fail = -1;
	op->addr = addr;
	op->ptr = op->val = -1;
	op->refptr = 0;

	ret = op->size = u8_decode_opcode(buf, len, &cmd);

	if(ret < 0)
		return ret;

	switch(cmd.type)
	{
		// ADD instructions
		case U8_ADD_R:
		case U8_ADDC_R:
		case U8_ADD_O:
		case U8_ADDC_O:
		case U8_ADD_ER:
		case U8_ADD_ER_O:
		case U8_ADD_SP_O:
			op->type = R_ANAL_OP_TYPE_ADD; break;

		// AND instructions
		case U8_AND_R:
		case U8_AND_O:
			op->type = R_ANAL_OP_TYPE_AND; break;

		// CMP instructions
		case U8_CMP_R:
		case U8_CMPC_R:
		case U8_CMP_O:
		case U8_CMPC_O:
		case U8_CMP_ER:
			op->type = R_ANAL_OP_TYPE_CMP; break;

		// Register MOV instructions
		case U8_MOV_R:
		case U8_MOV_O:
		case U8_MOV_ER:
		case U8_MOV_ER_O:
		case U8_MOV_ECSR_R:
		case U8_MOV_ELR_ER:
		case U8_MOV_EPSW_R:
		case U8_MOV_ER_ELR:
		case U8_MOV_ER_SP:
		case U8_MOV_PSW_R:
		case U8_MOV_PSW_O:
		case U8_MOV_R_ECSR:
		case U8_MOV_R_EPSW:
		case U8_MOV_R_PSW:
		case U8_MOV_SP_ER:

		// Coprocessor MOV instructions
		case U8_MOV_CR_R:
		case U8_MOV_CER_EA:
		case U8_MOV_CER_EAP:
		case U8_MOV_CR_EA:
		case U8_MOV_CR_EAP:
		case U8_MOV_CXR_EA:
		case U8_MOV_CXR_EAP:
		case U8_MOV_CQR_EA:
		case U8_MOV_CQR_EAP:
		case U8_MOV_R_CR:
		case U8_MOV_EA_CER:
		case U8_MOV_EAP_CER:
		case U8_MOV_EA_CR:
		case U8_MOV_EAP_CR:
		case U8_MOV_EA_CXR:
		case U8_MOV_EAP_CXR:
		case U8_MOV_EA_CQR:
			op->type = R_ANAL_OP_TYPE_MOV; break;
		case U8_OR_R:
		case U8_OR_O:
			op->type = R_ANAL_OP_TYPE_OR; break;
		case U8_XOR_R:
		case U8_XOR_O:
			op->type = R_ANAL_OP_TYPE_XOR; break;
		case U8_SUB_R:
		case U8_SUBC_R:
			op->type = R_ANAL_OP_TYPE_SUB; break;
		case U8_SLL_R:
		case U8_SLLC_R:
		case U8_SLL_O:
		case U8_SLLC_O:
			op->type = R_ANAL_OP_TYPE_SHL; break;
		case U8_SRA_R:
		case U8_SRA_O:
			op->type = R_ANAL_OP_TYPE_SAR; break;
		case U8_SRL_R:
		case U8_SRLC_R:
		case U8_SRL_O:
		case U8_SRLC_O:
			op->type = R_ANAL_OP_TYPE_SHR; break;

		// Load instructions
		case U8_L_ER_EA:
		case U8_L_ER_EAP:
		case U8_L_ER_ER:
		case U8_L_ER_D16_ER:
		case U8_L_ER_D6_BP:
		case U8_L_ER_D6_FP:
		case U8_L_ER_DA:
		case U8_L_R_EA:
		case U8_L_R_EAP:
		case U8_L_R_ER:
		case U8_L_R_D16_ER:
		case U8_L_R_D6_BP:
		case U8_L_R_D6_FP:
		case U8_L_R_DA:
		case U8_L_XR_EA:
		case U8_L_XR_EAP:
		case U8_L_QR_EA:
		case U8_L_QR_EAP:
			op->type = R_ANAL_OP_TYPE_LOAD; break;

		// Store instructions
		case U8_ST_ER_EA:
		case U8_ST_ER_EAP:
		case U8_ST_ER_ER:
		case U8_ST_ER_D16_ER:
		case U8_ST_ER_D6_BP:
		case U8_ST_ER_D6_FP:
		case U8_ST_ER_DA:
		case U8_ST_R_EA:
		case U8_ST_R_EAP:
		case U8_ST_R_ER:
		case U8_ST_R_D16_ER:
		case U8_ST_R_D6_BP:
		case U8_ST_R_D6_FP:
		case U8_ST_R_DA:
		case U8_ST_XR_EA:
		case U8_ST_XR_EAP:
		case U8_ST_QR_EA:
		case U8_ST_QR_EAP:
			op->type = R_ANAL_OP_TYPE_STORE; break;

		// Push/pop instructions
		case U8_PUSH_ER:
		case U8_PUSH_QR:
		case U8_PUSH_R:
		case U8_PUSH_XR:
			op->stackop = R_ANAL_STACK_SET; 	// is this useful?
			op->type = R_ANAL_OP_TYPE_PUSH; break;
		case U8_POP_ER:
		case U8_POP_QR:
		case U8_POP_R:
		case U8_POP_XR:
			op->stackop = R_ANAL_STACK_GET;
			op->type = R_ANAL_OP_TYPE_POP; break;

		// Register list stack instructions
		// FIXME: programming model could be established here, from use of CSR, LCSR, ECSR
		case U8_PUSH_RL:
			op->type = R_ANAL_OP_TYPE_PUSH; break;

		case U8_POP_RL:
			// certain types of 'pop' may act as subroutine or interrupt returns
			// (see nX-U8/100 Core Ref. Ch.1, S.4 - Exception Levels and Backup Registers)
			switch(cmd.op1)
			{
				// FIXME: investigate 3, 7, a, b, f (also include pc).
				case 0x2:	// pc (return type A-2)
				case 0x6:	// psw, pc (return types B-1-2, C-1)
				case 0xe:	// pc, psw, lr (return types B-2-2, C-2)
					op->type = R_ANAL_OP_TYPE_RET;
					break;

				default:
					op->type = R_ANAL_OP_TYPE_POP;
					break;
			}
			break;

		// EA register data transfer instructions
		case U8_LEA_ER:
		case U8_LEA_D16_ER:
		case U8_LEA_DA:
			op->type = R_ANAL_OP_TYPE_LEA; break;

		// ALU Instructions
		case U8_DAA_R:
		case U8_DAS_R:
		case U8_NEG_R:
			op->type = R_ANAL_OP_TYPE_NULL; break;

		// Bit access instructions
		case U8_SB_R:
		case U8_RB_R:
		case U8_TB_R:
		case U8_SB_DBIT:
		case U8_RB_DBIT:
		case U8_TB_DBIT:
			op->type = R_ANAL_OP_TYPE_NULL; break;

		// PSW access instructions      (no operands)
		case U8_EI:
		case U8_DI:
		case U8_SC:
		case U8_RC:
		case U8_CPLC:
			op->type = R_ANAL_OP_TYPE_NULL; break;

		// Conditional relative branch instructions
		case U8_BGE_RAD:
		case U8_BLT_RAD:
		case U8_BGT_RAD:
		case U8_BLE_RAD:
		case U8_BGES_RAD:
		case U8_BLTS_RAD:
		case U8_BGTS_RAD:
		case U8_BLES_RAD:
		case U8_BNE_RAD:
		case U8_BEQ_RAD:
		case U8_BNV_RAD:
		case U8_BOV_RAD:
		case U8_BPS_RAD:
		case U8_BNS_RAD:
			op->type = R_ANAL_OP_TYPE_CJMP;
			op->jump = addr + sizeof(cmd.opcode) + 		// next instruction word, plus
				((st8)cmd.op1 * sizeof(cmd.opcode));	//   op1 words (+ive or -ive)
			op->fail = addr + sizeof(cmd.opcode);
			break;
		case U8_BAL_RAD:
			op->type = R_ANAL_OP_TYPE_JMP;
			op->jump = addr + sizeof(cmd.opcode) +		// next instruction word
				((st8)cmd.op1 * sizeof(cmd.opcode));	//   op1 words (+ive or -ive)
				// cannot fail
			break;
		// Sign extension instruction
		case U8_EXTBW_ER:
			op->type = R_ANAL_OP_TYPE_NULL; break;

		// Software interrupt instructions
		case U8_SWI_O:
			op->type = R_ANAL_OP_TYPE_SWI; break;
		case U8_BRK:
			op->type = R_ANAL_OP_TYPE_TRAP; break;
		// Branch instructions
		case U8_B_AD:
			op->type = R_ANAL_OP_TYPE_CALL; break;
		case U8_BL_AD:
			// simulate segment register
			op->jump = (cmd.op1 * 0x10000) + cmd.s_word;
			op->type = R_ANAL_OP_TYPE_CALL; break;
		case U8_B_ER:
		case U8_BL_ER:
			op->type = R_ANAL_OP_TYPE_RCALL; break;

		// Multiplication and division instructions
		case U8_MUL_ER:
			op->type = R_ANAL_OP_TYPE_MUL; break;
		case U8_DIV_ER:
			op->type = R_ANAL_OP_TYPE_DIV; break;

		// Miscellaneous (no operands)
		case U8_INC_EA:
		case U8_DEC_EA:
			op->type = R_ANAL_OP_TYPE_NULL; break;
			break;

		// Return instructions
		case U8_RT:
		case U8_RTI:
			op->type = R_ANAL_OP_TYPE_RET; break;
		case U8_NOP:
			op->type = R_ANAL_OP_TYPE_NOP; break;

		case U8_ILL:
		default:
			op->type = R_ANAL_OP_TYPE_ILL;

	}
	return op->size;
}

// generate mask for signature matching
// FIXME: some extra thought here would improve matching accuracy
static ut8 *u8_anal_mask(RAnal *anal, int size, const ut8 *data, ut64 at)
{
	RAnalOp *op = NULL;
	ut8 *ret = NULL;
	int i;
	struct u8_cmd cmd;

	if(!data)
	{
		return NULL;
	}

	if(!(op = r_anal_op_new()))
	{
		return NULL;
	}

	// mask array = length of function
	if(!(ret = malloc(size)))
	{
		r_anal_op_free(op);
		return NULL;
	}

	memset(ret, 0xff, size);

	for(i=0; i+1<size; i+=op->size)
	{
		op->size = u8_decode_opcode(data, size, &cmd);

		if(op->size < 2)
		{
			break;
		}

		if(op->size == 4)	// second word of 2 word instruction is always data
		{
			ret[i + 2] = 0;
			ret[i + 3] = 0;
		}

		ret[i] = u8inst[cmd.type].ins_mask;
		ret[i + 1] = u8inst[cmd.type].ins_mask >> 8;
	}

	r_anal_op_free (op);

	return ret;
}

struct r_anal_plugin_t r_anal_plugin_u8 =
{
	.name = "u8",
	.desc = "nX-U8/100 analysis plugin",
	.license = "LGPL3",
	.arch = "u8",
	.bits = 8 | 16,
	.anal_mask = u8_anal_mask,
	.op = &u8_anop,
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin =
{
	.type = R_LIB_TYPE_ANAL,
	.data = &r_anal_plugin_u8,
	.version = R2_VERSION
};
#endif

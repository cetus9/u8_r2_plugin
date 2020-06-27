#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

// r2 specific
#include <r_types.h>

// The struct u8inst[U8_INS_NUM] contains instruction definitions
#include "u8_disas.h"

// operand string format util macro
#define fmt_op_str(x, args...)	snprintf(cmd->operands, sizeof(cmd->operands), x, ##args)

// for signed 6-bit integers (Disp6)
int isneg_6bit(ut8 n)
{
	return (n>>5) & 0x01;	// msb determines sign
}

// for signed 7-bit integers (#imm7)
int isneg_7bit(ut8 n)
{
	return (n>>6) & 0x01;	// msb determines sign
}

// calculate absolute value for signed 6-bit number (Disp6)
ut8 abs_6bit(ut8 n)
{
	if(isneg_6bit(n))		// -ive number
		return (~n+1 & 0x3f);	//	flip bits and add 1...
	else 				// +ive number
		return n & 0x3f;	//	...or just mask out top 2 bits;
}

// calculate absolute value for signed 7-bit number (#imm7)
ut8 abs_7bit(ut8 n)
{
	if(isneg_7bit(n))		// -ive number
		return (~n+1 & 0x7f);	//	flip bits and add 1...
	else				// +ive number
		return n & 0x7f;	//	...or just mask out top bit;
}

// get instruction type (e.g. U8_MOV_..) for given opcode
int u8_decode_inst(ut16 opcode)
{
	ut16 t;
	int i;

	// iterate through master table of instructions, returning on match
	// FIXME: this can be done more efficiently
	for(i=0; i<U8_INS_NUM; i++)
	{
		if((opcode & u8inst[i].ins_mask) == u8inst[i].ins)
   		return i;
	}
	// or error
	return U8_ILL;
}

// extract operand from first word
ut16 u8_decode_operand(ut16 inst, ut16 mask)
{
	int n;

	// number of bits to shift, based on operand mask
	// FIXME: remove redundant masks
	switch(mask)
	{
		case 0x0800:
			n=11; break;
		case 0x0c00:
			n=10; break;
		case 0x0e00:
			n=9; break;
		case 0x0f00:
		case 0x0700:
			n=8; break;
		case 0x00e0:
			n=5; break;
		case 0x00f0:
		case 0x0070:
			n=4; break;
		case 0x00ff:
		case 0x007f:
		case 0x003f:
		default:
			n=0;
	}

	return (inst & mask)>>n;
}

// build opcode string
int u8_decode_opcode(const ut8 *buf, int len, struct u8_cmd *cmd)
{
	unsigned int i=0, addr;

	ut16 inst, s_word, prefix=0;
	ut16 op1, op2;

	// simplify L/ST handling with separate prefix logic
	ut8 pre_pseg=0, pre_dsr=0, pre_r=0;
	char prefix_str[8] = "";

	if(len < 2)			// machine words are at least 2 bytes
		return -1;

	inst = r_read_at_le16(buf, 0);
	i++;				// count number of words read

	cmd->type = u8_decode_inst(inst);

	switch(cmd->type)
	{
		case U8_PRE_PSEG:
			pre_pseg = u8_decode_operand(inst, u8inst[cmd->type].op1_mask);
			snprintf(prefix_str, sizeof(prefix_str), "%02xh:", pre_pseg);
			prefix=inst;
			break;
		case U8_PRE_DSR:
			snprintf(prefix_str, sizeof(prefix_str), "dsr:");
			pre_dsr=1;
			prefix=inst;
			break;
		case U8_PRE_R:
			pre_r = u8_decode_operand(inst, u8inst[cmd->type].op1_mask);
			snprintf(prefix_str, sizeof(prefix_str), "r%d:", pre_r);
			prefix=inst;
			break;
	}

	if(prefix)
	{
		// FIXME: check that instruction after prefix is L/ST 
		//	otherwise prefix is misidentified
		if(len < (i+1)*2)			// buffer in bytes
			return -1;

		inst = r_read_at_le16(buf, i*2);	// read first real instruction word after prefix
		i++;

		cmd->type = u8_decode_inst(inst);
	}

	// if instruction type is 2 words long, read second word
	if(u8inst[cmd->type].len == 2)
	{
		if(len < (i+1)*2)			// buffer in bytes
			return -1;

		s_word = r_read_at_le16(buf, i*2);	// read second (possibly third) word from stream
		i++;
	}

	// set instruction mnemonic
	strncpy(cmd->instr, u8inst[cmd->type].name, sizeof(cmd->instr));

	// extract first operand from instruction word 1
	if(u8inst[cmd->type].ops == 1)			// single operand
	{
		op1 = u8_decode_operand(inst, u8inst[cmd->type].op1_mask);
		cmd->op1 = op1;
	}
	// ...or extract both operands from instruction word 1
	if(u8inst[cmd->type].ops == 2)			// 2 operands
	{
		op1 = u8_decode_operand(inst, u8inst[cmd->type].op1_mask);
		op2 = u8_decode_operand(inst, u8inst[cmd->type].op2_mask);
		cmd->op1 = op1;
		cmd->op2 = op2;
	}

	// Display operands with correct formatting
	switch(cmd->type)
	{
		// 8-bit register instructions
		case U8_ADD_R:
		case U8_AND_R:
		case U8_ADDC_R:
		case U8_CMP_R:
		case U8_CMPC_R:
		case U8_MOV_R:
		case U8_OR_R:
		case U8_XOR_R:
		case U8_SUB_R:
		case U8_SUBC_R:
		case U8_SLL_R:
		case U8_SLLC_R:
		case U8_SRA_R:
		case U8_SRL_R:
		case U8_SRLC_R:
			fmt_op_str("r%d, r%d", op1, op2);
			break;

		// 8-bit register/object instructions
		case U8_ADD_O:
		case U8_AND_O:
		case U8_ADDC_O:
		case U8_CMP_O:
		case U8_CMPC_O:
		case U8_MOV_O:
		case U8_OR_O:
		case U8_XOR_O:
		case U8_SLL_O:
		case U8_SLLC_O:
		case U8_SRA_O:
		case U8_SRL_O:
		case U8_SRLC_O:
			fmt_op_str("r%d, #%xh", op1, op2);
			break;

		// 16-bit extended register instructions
		case U8_ADD_ER:
		case U8_MOV_ER:
		case U8_CMP_ER:
			fmt_op_str("er%d, er%d", op1, op2);
			break;

		// Extended register/object instructions #imm7
		case U8_ADD_ER_O:
		case U8_MOV_ER_O:
			if(isneg_7bit(op2))
				fmt_op_str("er%d, #-%xh", op1, abs_7bit(op2));
			else
				fmt_op_str("er%d, #%xh", op1, abs_7bit(op2));
			break;

		// Extended register load/store instructions
		case U8_L_ER_EA:
		case U8_ST_ER_EA:
			fmt_op_str("er%d, %s[ea]", op1, prefix_str);
			break;
		case U8_L_ER_EAP:
		case U8_ST_ER_EAP:
			fmt_op_str("er%d, %s[ea+]", op1, prefix_str);
			break;
		case U8_L_ER_ER:
		case U8_ST_ER_ER:
			fmt_op_str("er%d, %s[er%d]", op1, prefix_str, op2);
			break;
		case U8_L_ER_D16_ER:
		case U8_ST_ER_D16_ER:
			fmt_op_str("er%d, %04xh[er%d]", op1, s_word, op2);
			break;
		case U8_L_ER_D6_BP:
		case U8_ST_ER_D6_BP:
			if(isneg_6bit(op2))
				fmt_op_str("er%d, -%xh[bp]", op1, abs_6bit(op2));
			else
				fmt_op_str("er%d, %xh[bp]", op1, abs_6bit(op2));
			break;
		case U8_L_ER_D6_FP:
		case U8_ST_ER_D6_FP:
			if(isneg_6bit(op2))
				fmt_op_str("er%d, -%xh[fp]", op1, abs_6bit(op2));
			else
				fmt_op_str("er%d, %xh[fp]", op1, abs_6bit(op2));
			break;
		case U8_L_ER_DA:
		case U8_ST_ER_DA:
			fmt_op_str("er%d, %04xh", op1, s_word);
			break;

		// Register load/store instructions
		case U8_L_R_EA:
		case U8_ST_R_EA:
			fmt_op_str("r%d, [ea]", op1);
			break;
		case U8_L_R_EAP:
		case U8_ST_R_EAP:
			fmt_op_str("r%d, [ea+]", op1);
			break;
		case U8_L_R_ER:
		case U8_ST_R_ER:
			fmt_op_str("r%d, [er%d]", op1, op2);
			break;
		case U8_L_R_D16_ER:
		case U8_ST_R_D16_ER:
			fmt_op_str("r%d, %s%04xh[er%d]", op1, prefix_str, s_word, op2);
			break;
		case U8_L_R_D6_BP:
		case U8_ST_R_D6_BP:
			if(isneg_6bit(op2))
				fmt_op_str("r%d, %s-%xh[bp]", op1, prefix_str, abs_6bit(op2));
			else
				fmt_op_str("r%d, %s%xh[bp]", op1, prefix_str, abs_6bit(op2));
			break;
		case U8_L_R_D6_FP:
		case U8_ST_R_D6_FP:
			if(isneg_6bit(op2))
				fmt_op_str("r%d, %s-%xh[fp]", op1, prefix_str, abs_6bit(op2));
			else
				fmt_op_str("r%d, %s%xh[fp]", op1, prefix_str, abs_6bit(op2));
			break;
		case U8_L_R_DA:
		case U8_ST_R_DA:
			fmt_op_str("r%d, %s%04xh", op1, prefix_str, s_word);
			break;

		// Double/quad word register load/store instructions
		case U8_L_XR_EA:
		case U8_ST_XR_EA:
			fmt_op_str("xr%d, %s[ea]", op1, prefix_str);
			break;
		case U8_L_XR_EAP:
		case U8_ST_XR_EAP:
			fmt_op_str("xr%d, %s[ea+]", op1, prefix_str);
			break;
		case U8_L_QR_EA:
		case U8_ST_QR_EA:
			fmt_op_str("qr%d, %s[ea]", op1, prefix_str);
			break;
		case U8_L_QR_EAP:
		case U8_ST_QR_EAP:
			fmt_op_str("qr%d, %s[ea+]", op1, prefix_str);
			break;

		// Control register access instructions
		case U8_ADD_SP_O:
			fmt_op_str("sp, #%xh", op1);
			break;
		case U8_MOV_ECSR_R:
			fmt_op_str("ecsr, r%d", op1);
			break;
		case U8_MOV_ELR_ER:
			fmt_op_str("elr, er%d", op1);
			break;
		case U8_MOV_EPSW_R:
			fmt_op_str("epsw, r%d", op1);
			break;
		case U8_MOV_ER_ELR:
			fmt_op_str("er%d, elr", op1);
			break;
		case U8_MOV_ER_SP:
			fmt_op_str("er%d, sp", op1);
			break;
		case U8_MOV_PSW_R:
			fmt_op_str("psw, r%d", op1);
			break;
		case U8_MOV_PSW_O:
			fmt_op_str("psw, #%xh", op1);
			break;
		case U8_MOV_R_ECSR:
			fmt_op_str("r%d, ecsr", op1);
			break;
		case U8_MOV_R_EPSW:
			fmt_op_str("r%d, epsw", op1);
			break;
		case U8_MOV_R_PSW:
			fmt_op_str("r%d, psw", op1);
			break;
		case U8_MOV_SP_ER:
			fmt_op_str("sp, er%d", op1);
			break;

		// Push/pop instructions
		case U8_PUSH_ER:
		case U8_POP_ER:
			fmt_op_str("er%d", op1);
			break;
		case U8_PUSH_QR:
		case U8_POP_QR:
			fmt_op_str("qr%d", op1);
			break;
		case U8_PUSH_R:
		case U8_POP_R:
			fmt_op_str("r%d", op1);
			break;
		case U8_PUSH_XR:
		case U8_POP_XR:
			fmt_op_str("xr%d", op1);
			break;

		// Register list stack instructions
		case U8_PUSH_RL:
			switch(op1)			// parse 4-bit list
			{
				case 1:
					fmt_op_str("ea"); break;
				case 2:
					fmt_op_str("elr"); break;
				case 3:
					fmt_op_str("ea, elr"); break;
				case 4:
					fmt_op_str("epsw"); break;
				case 5:
					fmt_op_str("epsw, ea"); break;
				case 6:
					fmt_op_str("epsw, elr"); break;
				case 7:
					fmt_op_str("epsw, elr, ea"); break;
				case 8:
					fmt_op_str("lr"); break;
				case 9:
					fmt_op_str("lr, ea"); break;
				case 0xa:
					fmt_op_str("lr, elr"); break;
				case 0xb:
					fmt_op_str("lr, ea, elr"); break;
				case 0xc:
					fmt_op_str("lr, epsw"); break;
				case 0xd:
					fmt_op_str("lr, epsw, ea"); break;
				case 0xe:
					fmt_op_str("lr, epsw, elr"); break;
				case 0xf:
					fmt_op_str("lr, epsw, elr, ea"); break;
				default:
					fmt_op_str("?");
			}
			break;

		case U8_POP_RL:
			switch(op1)			// parse 4-bit list
			{
				case 1:
					fmt_op_str("ea"); break;
				case 2:
					fmt_op_str("pc"); break;
				case 3:
					fmt_op_str("ea, pc"); break;
				case 4:
					fmt_op_str("psw"); break;
				case 5:
					fmt_op_str("ea, psw"); break;
				case 6:
					fmt_op_str("pc, psw"); break;
				case 7:
					fmt_op_str("ea, pc, psw"); break;
				case 8:
					fmt_op_str("lr"); break;
				case 9:
					fmt_op_str("ea, lr"); break;
				case 0xa:
					fmt_op_str("pc, lr"); break;
				case 0xb:
					fmt_op_str("ea, pc, lr"); break;
				case 0xc:
					fmt_op_str("lr, psw"); break;
				case 0xd:
					fmt_op_str("ea, psw, lr"); break;
				case 0xe:
					fmt_op_str("pc, psw, lr"); break;
				case 0xf:
					fmt_op_str("ea, pc, psw, lr"); break;
				default:
					fmt_op_str("?");
			}
			break;

		// Coprocessor data transfer instructions
		case U8_MOV_CR_R:
			fmt_op_str("cr%d, r%d", op1, op2);
			break;
		case U8_MOV_CER_EA:
			fmt_op_str("cer%d, [ea]", op1);
			break;
		case U8_MOV_CER_EAP:
			fmt_op_str("cer%d, [ea+]", op1);
			break;
		case U8_MOV_CR_EA:
			fmt_op_str("cr%d, [ea]", op1);
			break;
		case U8_MOV_CR_EAP:
			fmt_op_str("cr%d, [ea+]", op1);
			break;
		case U8_MOV_CXR_EA:
			fmt_op_str("cxr%d, [ea]", op1);
			break;
		case U8_MOV_CXR_EAP:
			fmt_op_str("cxr%d, [ea+]", op1);
			break;
		case U8_MOV_CQR_EA:
			fmt_op_str("cqr%d, [ea]", op1);
			break;
		case U8_MOV_CQR_EAP:
			fmt_op_str("cqr%d, [ea+]", op1);
			break;
		case U8_MOV_R_CR:
			fmt_op_str("r%d, cr%d", op1, op2);
			break;
		case U8_MOV_EA_CER:
			fmt_op_str("[ea], cer%d", op1);
			break;
		case U8_MOV_EAP_CER:
			fmt_op_str("[ea+], cer%d", op1);
			break;
		case U8_MOV_EA_CR:
			fmt_op_str("[ea], cr%d", op1);
			break;
		case U8_MOV_EAP_CR:
			fmt_op_str("[ea+], cr%d", op1);
			break;
		case U8_MOV_EA_CXR:
			fmt_op_str("[ea], cxr%d", op1);
			break;
		case U8_MOV_EAP_CXR:
			fmt_op_str("[ea+], cxr%d", op1);
			break;
		case U8_MOV_EA_CQR:
			fmt_op_str("[ea], cqr%d", op1);
			break;
		case U8_MOV_EAP_CQR:
			fmt_op_str("[ea+], cqr%d", op1);
			break;

		// EA register data transfer instructions
		case U8_LEA_ER:
			fmt_op_str("[er%d]", op1);
			break;
		case U8_LEA_D16_ER:
			fmt_op_str("%04xh[er%d]", s_word, op1);
			break;
		case U8_LEA_DA:
			fmt_op_str("%04xh", s_word);
			break;

		// ALU Instructions
		case U8_DAA_R:
		case U8_DAS_R:
		case U8_NEG_R:
			fmt_op_str("r%d", op1);
			break;

		// Bit access instructions
		case U8_SB_R:
		case U8_RB_R:
		case U8_TB_R:
			fmt_op_str("r%d.%d", op1, op2);
			break;
		case U8_SB_DBIT:
		case U8_RB_DBIT:
		case U8_TB_DBIT:
			fmt_op_str("%04xh.%d", s_word, op1);
			break;

		// PSW access instructions (no operands)
		case U8_EI:
		case U8_DI:
		case U8_SC:
		case U8_RC:
		case U8_CPLC:
			break;

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
		case U8_BAL_RAD:
			// handle +ive or -ive address jump cases
			if((st8)op1 < 0)
				fmt_op_str("-%02xh", abs(0-(st8)op1));
			else
				fmt_op_str("+%02xh", (st8)op1);
			break;

		// Sign extension instruction
		case U8_EXTBW_ER:
			fmt_op_str("er%d", op2);
			break;

		// Software interrupt instructions
		case U8_SWI_O:
			fmt_op_str("#%xh", op1);
			break;
		case U8_BRK:
			break;

		// Branch instructions
		case U8_B_AD:
		case U8_BL_AD:
			fmt_op_str("%xh:%04xh", op1, s_word);
			break;
		case U8_B_ER:
		case U8_BL_ER:
			fmt_op_str("er%d", op1);
			break;

		// Multiplication and division instructions
		case U8_MUL_ER:
		case U8_DIV_ER:
			fmt_op_str("er%d, r%d", op1, op2);
			break;

		// Miscellaneous (no operands)
		case U8_INC_EA:
		case U8_DEC_EA:
		case U8_RT:
		case U8_RTI:
		case U8_NOP:
			break;

		case U8_ILL:
		default:
			// will display with 'dw' mnemonic to indicate 'data'
			fmt_op_str("%4xh", inst);
	}

	return i*sizeof(inst);		// 1 or 2 words (up to 3 with prefix)
}

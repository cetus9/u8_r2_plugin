#ifndef U8_DISAS_H
#define U8_DISAS_H

#include <r_types.h>

struct u8_cmd
{
	int type;		// index in instruction table
	ut16 opcode;		// instruction word
	ut16 op1;		// first decoded operand
	ut16 op2;		// second decoded operand
	ut16 s_word;		// optional second data word

	// String of assembly operation mnemonic.
	char instr[6];

	// String of formatted operands.
	char operands[20];
};

int u8_decode_command(const ut8 *instr, int len, struct u8_cmd *cmd);
int u8_decode_opcode(const ut8 *buf, int len, struct u8_cmd *cmd);
int u8_decode_inst(ut16 inst);

// define u8 instructions
#define U8_INS_NUM	159		// 155 + 3 prefix codes + 'unknown'

// Arithmetic instructions
#define U8_ADD_R		0
#define U8_ADD_O		1
#define U8_ADD_ER		2
#define U8_ADD_ER_O		3
#define U8_ADDC_R		4
#define U8_ADDC_O		5
#define U8_AND_R		6
#define U8_AND_O		7
#define U8_CMP_R		8
#define U8_CMP_O		9
#define U8_CMPC_R		10
#define U8_CMPC_O		11
#define U8_MOV_ER		12
#define U8_MOV_ER_O		13
#define U8_MOV_R		14
#define U8_MOV_O		15
#define U8_OR_R			16
#define U8_OR_O			17
#define U8_XOR_R		18
#define U8_XOR_O		19
#define U8_CMP_ER		20
#define U8_SUB_R		21
#define U8_SUBC_R		22

// Shift instructions
#define U8_SLL_R		23
#define U8_SLL_O		24
#define U8_SLLC_R		25
#define U8_SLLC_O		26
#define U8_SRA_R		27
#define U8_SRA_O		28
#define U8_SRL_R		29
#define U8_SRL_O		30
#define U8_SRLC_R		31
#define U8_SRLC_O		32

// Load/store instructions
#define U8_L_ER_EA		33
#define U8_L_ER_EAP		34
#define U8_L_ER_ER		35
#define U8_L_ER_D16_ER		36
#define U8_L_ER_D6_BP		37
#define U8_L_ER_D6_FP		38
#define U8_L_ER_DA		39
#define U8_L_R_EA		40
#define U8_L_R_EAP		41
#define U8_L_R_ER		42
#define U8_L_R_D16_ER		43
#define U8_L_R_D6_BP		44
#define U8_L_R_D6_FP		45
#define U8_L_R_DA		46
#define U8_L_XR_EA		47
#define U8_L_XR_EAP		48
#define U8_L_QR_EA		49
#define U8_L_QR_EAP		50
#define U8_ST_ER_EA		51
#define U8_ST_ER_EAP		52
#define U8_ST_ER_ER		53
#define U8_ST_ER_D16_ER		54
#define U8_ST_ER_D6_BP		55
#define U8_ST_ER_D6_FP		56
#define U8_ST_ER_DA		57
#define U8_ST_R_EA		58
#define U8_ST_R_EAP		59
#define U8_ST_R_ER		60
#define U8_ST_R_D16_ER		61
#define U8_ST_R_D6_BP		62
#define U8_ST_R_D6_FP		63
#define U8_ST_R_DA		64
#define U8_ST_XR_EA		65
#define U8_ST_XR_EAP		66
#define U8_ST_QR_EA		67
#define U8_ST_QR_EAP		68

// Control register access instructions
#define U8_ADD_SP_O		69
#define U8_MOV_ECSR_R		70
#define U8_MOV_ELR_ER		71
#define U8_MOV_EPSW_R		72
#define U8_MOV_ER_ELR		73
#define U8_MOV_ER_SP		74
#define U8_MOV_PSW_R		75
#define U8_MOV_PSW_O		76
#define U8_MOV_R_ECSR		77
#define U8_MOV_R_EPSW		78
#define U8_MOV_R_PSW		79
#define U8_MOV_SP_ER		80

// Push/pop instructions
#define U8_PUSH_ER		81
#define U8_PUSH_QR		82
#define U8_PUSH_R		83
#define U8_PUSH_XR		84
#define U8_PUSH_RL		85
#define U8_POP_ER		86
#define U8_POP_QR		87
#define U8_POP_R		88
#define U8_POP_XR		89
#define U8_POP_RL		90

// Coprocessor data transfer instructions
#define U8_MOV_CR_R		91
#define U8_MOV_CER_EA		92
#define U8_MOV_CER_EAP		93
#define U8_MOV_CR_EA		94
#define U8_MOV_CR_EAP		95
#define U8_MOV_CXR_EA		96
#define U8_MOV_CXR_EAP		97
#define U8_MOV_CQR_EA		98
#define U8_MOV_CQR_EAP		99
#define U8_MOV_R_CR		100
#define U8_MOV_EA_CER		101
#define U8_MOV_EAP_CER		102
#define U8_MOV_EA_CR		103
#define U8_MOV_EAP_CR		104
#define U8_MOV_EA_CXR		105
#define U8_MOV_EAP_CXR		106
#define U8_MOV_EA_CQR		107
#define U8_MOV_EAP_CQR		108

// EA register data transfer instructions
#define U8_LEA_ER		109
#define U8_LEA_D16_ER		110
#define U8_LEA_DA		111

// ALU Instructions
#define U8_DAA_R		112
#define U8_DAS_R		113
#define U8_NEG_R		114

// Bit access instructions
#define U8_SB_R			115
#define U8_SB_DBIT		116
#define U8_RB_R			117
#define U8_RB_DBIT		118
#define U8_TB_R			119
#define U8_TB_DBIT		120

// PSW access instructions
#define U8_EI			121
#define U8_DI			122
#define U8_SC			123
#define U8_RC			124
#define U8_CPLC			125

// Conditional relative branch instructions
#define U8_BGE_RAD		126
#define U8_BLT_RAD		127
#define U8_BGT_RAD		128
#define U8_BLE_RAD		129
#define U8_BGES_RAD		130
#define U8_BLTS_RAD		131
#define U8_BGTS_RAD		132
#define U8_BLES_RAD		133
#define U8_BNE_RAD		134
#define U8_BEQ_RAD		135
#define U8_BNV_RAD		136
#define U8_BOV_RAD		137
#define U8_BPS_RAD		138
#define U8_BNS_RAD		139
#define U8_BAL_RAD		140

// Sign extension instruction
#define U8_EXTBW_ER		141

// Software interrupt instructions
#define U8_SWI_O		142
#define U8_BRK			143

// Branch instructions
#define U8_B_AD			144
#define U8_B_ER			145
#define U8_BL_AD		146
#define U8_BL_ER		147

// Multiplication and division instructions
#define U8_MUL_ER		148
#define U8_DIV_ER		149

// Miscellaneous
#define U8_INC_EA		150
#define U8_DEC_EA		151
#define U8_RT			152
#define U8_RTI			153
#define U8_NOP			154

// DSR prefix for load/store
#define U8_PRE_PSEG		155
#define U8_PRE_DSR		156
#define U8_PRE_R		157

// Undefined
#define U8_ILL			158

typedef struct u8inst_t
{
	unsigned int id;		// instruction ID
	unsigned char name[6];		// mnemonic for instruction
	unsigned int len;		// 1 or 2 - instruction length in words (16 or 32 bits)
	unsigned int ops;		// number of operands in word 1

	ut8 flags;			// flags affected (C, Z, S, OV, MIE, HC)
	ut16 ins;			// word 1 instruction pattern
	ut16 ins_mask;			// word 1 instruction mask
	ut16 op1_mask;			// word 1 first operand mask
	ut16 op2_mask;			// word 1 second operand mask

	ut16 prefix;			// DSR load/store prefix

} u8inst_t;

extern u8inst_t u8inst[U8_INS_NUM];

#endif /* U8_DISAS_H */

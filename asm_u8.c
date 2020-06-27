/* radare nX-U8/100 asm plugin - LGPL - Copyright 2020 - cetus9 */

#include <stdio.h>
#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>

#include "u8_disas.h"

static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len)
{
	struct u8_cmd cmd =
	{
        	.instr = "",
        	.operands = ""
	};

	if (len < 2) return -1;
	int ret = u8_decode_opcode(buf, len, &cmd);

	if (ret > 0)
	{
		r_strbuf_set(&op->buf_asm, sdb_fmt("%s %s", cmd.instr, cmd.operands));
	}
	return op->size = ret;
}

RAsmPlugin r_asm_plugin_u8 =
{
	.name = "u8",
	.license = "LGPL3",
	.desc = "nX-U8/100 disassembly plugin",
	.arch = "u8",
	.bits = 8 | 16,
	.endian = R_SYS_ENDIAN_LITTLE,
	.disassemble = &disassemble
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin =
{
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_u8,
	.version = R2_VERSION
};
#endif

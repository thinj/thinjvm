/*
 * instructions.h
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_
#include "types.h"
#include "frame.h"

typedef void (*instruction)(void);
//#define INSTRUCTION(x) void x(void)

#define INS_UNDEFINED inUndefinedInstruction(__FILE__, __LINE__)

#define INS_BEGIN(X) lbl_##X:  { \
	/***/

#define INS_END \
        goto nextInstruction; \
}

#define IFINS(NAME, CONDITION) \
		INS_BEGIN(NAME)  \
			s2 offset = getS2FromCode();\
			\
			if (CONDITION) { \
				context.programCounter -= 3;\
				context.programCounter += offset; \
			} \
		INS_END

//cat instruction1.c | grep INS_BEGIN | sed 's/INS_BEGIN(/\&\&lbl_/g' | sed 's/).*/,/g' > dims
//cat instruction1.c | grep IFINS | sed 's/IFINS(/\&\&lbl_/g' | sed 's/,.*/,/g'  >> dims


#define JUMP_TABLE void *jumpTable[] = { \
	/* 00 (0x00) */&&lbl_f_nop, \
	/* 01 (0x01) */&&lbl_f_aconst_null, \
	/* 02 (0x02) */&&lbl_f_iconst_m1, \
	/* 03 (0x03) */&&lbl_f_iconst_0, \
	/* 04 (0x04) */&&lbl_f_iconst_1, \
	/* 05 (0x05) */&&lbl_f_iconst_2, \
	/* 06 (0x06) */&&lbl_f_iconst_3, \
	/* 07 (0x07) */&&lbl_f_iconst_4, \
	/* 08 (0x08) */&&lbl_f_iconst_5, \
	/* 09 (0x09) */&&lbl_f_lconst_0, \
	/* 10 (0x0a) */&&lbl_f_lconst_1, \
	/* 11 (0x0b) */&&lbl_f_fconst_0, \
	/* 12 (0x0c) */&&lbl_f_fconst_1, \
	/* 13 (0x0d) */&&lbl_f_fconst_2, \
	/* 14 (0x0e) */&&lbl_f_dconst_0, \
	/* 15 (0x0f) */&&lbl_f_dconst_1, \
	/* 16 (0x10) */&&lbl_f_bipush, \
	/* 17 (0x11) */&&lbl_f_sipush, \
	/* 18 (0x12) */&&lbl_f_ldc, \
	/* 19 (0x13) */&&lbl_f_ldc_w, \
	/* 20 (0x14) */&&lbl_f_ldc2_w, \
	/* 21 (0x15) */&&lbl_f_iload, \
	/* 22 (0x16) */&&lbl_f_lload, \
	/* 23 (0x17) */&&lbl_f_fload, \
	/* 24 (0x18) */&&lbl_f_dload, \
	/* 25 (0x19) */&&lbl_f_aload, \
	/* 26 (0x1a) */&&lbl_f_iload_0, \
	/* 27 (0x1b) */&&lbl_f_iload_1, \
	/* 28 (0x1c) */&&lbl_f_iload_2, \
	/* 29 (0x1d) */&&lbl_f_iload_3, \
	/* 30 (0x1e) */&&lbl_f_lload_0, \
	/* 31 (0x1f) */&&lbl_f_lload_1, \
	/* 32 (0x20) */&&lbl_f_lload_2, \
	/* 33 (0x21) */&&lbl_f_lload_3, \
	/* 34 (0x22) */&&lbl_f_fload_0, \
	/* 35 (0x23) */&&lbl_f_fload_1, \
	/* 36 (0x24) */&&lbl_f_fload_2, \
	/* 37 (0x25) */&&lbl_f_fload_3, \
	/* 38 (0x26) */&&lbl_f_dload_0, \
	/* 39 (0x27) */&&lbl_f_dload_1, \
	/* 40 (0x28) */&&lbl_f_dload_2, \
	/* 41 (0x29) */&&lbl_f_dload_3, \
	/* 42 (0x2a) */&&lbl_f_aload_0, \
	/* 43 (0x2b) */&&lbl_f_aload_1, \
	/* 44 (0x2c) */&&lbl_f_aload_2, \
	/* 45 (0x2d) */&&lbl_f_aload_3, \
	/* 46 (0x2e) */&&lbl_f_iaload, \
	/* 47 (0x2f) */&&lbl_f_laload, \
	/* 48 (0x30) */&&lbl_f_faload, \
	/* 49 (0x31) */&&lbl_f_daload, \
	/* 50 (0x32) */&&lbl_f_aaload, \
	/* 51 (0x33) */&&lbl_f_baload, \
	/* 52 (0x34) */&&lbl_f_caload, \
	/* 53 (0x35) */&&lbl_f_saload, \
	/* 54 (0x36) */&&lbl_f_istore, \
	/* 55 (0x37) */&&lbl_f_lstore, \
	/* 56 (0x38) */&&lbl_f_fstore, \
	/* 57 (0x39) */&&lbl_f_dstore, \
	/* 58 (0x3a) */&&lbl_f_astore, \
	/* 59 (0x3b) */&&lbl_f_istore_0, \
	/* 60 (0x3c) */&&lbl_f_istore_1, \
	/* 61 (0x3d) */&&lbl_f_istore_2, \
	/* 62 (0x3e) */&&lbl_f_istore_3, \
	/* 63 (0x3f) */&&lbl_f_lstore_0, \
	/* 64 (0x40) */&&lbl_f_lstore_1, \
	/* 65 (0x41) */&&lbl_f_lstore_2, \
	/* 66 (0x42) */&&lbl_f_lstore_3, \
	/* 67 (0x43) */&&lbl_f_fstore_0, \
	/* 68 (0x44) */&&lbl_f_fstore_1, \
	/* 69 (0x45) */&&lbl_f_fstore_2, \
	/* 70 (0x46) */&&lbl_f_fstore_3, \
	/* 71 (0x47) */&&lbl_f_dstore_0, \
	/* 72 (0x48) */&&lbl_f_dstore_1, \
	/* 73 (0x49) */&&lbl_f_dstore_2, \
	/* 74 (0x4a) */&&lbl_f_dstore_3, \
	/* 75 (0x4b) */&&lbl_f_astore_0, \
	/* 76 (0x4c) */&&lbl_f_astore_1, \
	/* 77 (0x4d) */&&lbl_f_astore_2, \
	/* 78 (0x4e) */&&lbl_f_astore_3, \
	/* 79 (0x4f) */&&lbl_f_iastore, \
	/* 80 (0x50) */&&lbl_f_lastore, \
	/* 81 (0x51) */&&lbl_f_fastore, \
	/* 82 (0x52) */&&lbl_f_dastore, \
	/* 83 (0x53) */&&lbl_f_aastore, \
	/* 84 (0x54) */&&lbl_f_bastore, \
	/* 85 (0x55) */&&lbl_f_castore, \
	/* 86 (0x56) */&&lbl_f_sastore, \
	/* 87 (0x57) */&&lbl_f_pop, \
	/* 88 (0x58) */&&lbl_f_pop2, \
	/* 089 (0x59) */&&lbl_f_dup, \
	/* 090 (0x5a) */&&lbl_f_dup_x1, \
	/* 091 (0x5b) */&&lbl_f_dup_x2, \
	/* 092 (0x5c) */&&lbl_f_dup2, \
	/* 093 (0x5d) */&&lbl_f_dup2_x1, \
	/* 094 (0x5e) */&&lbl_f_dup2_x2, \
	/* 095 (0x5f) */&&lbl_f_swap, \
	/* 096 (0x60) */&&lbl_f_iadd, \
	/* 097 (0x61) */&&lbl_f_ladd, \
	/* 098 (0x62) */&&lbl_f_fadd, \
	/* 099 (0x63) */&&lbl_f_dadd, \
	/* 100 (0x64) */&&lbl_f_isub, \
	/* 101 (0x65) */&&lbl_f_lsub, \
	/* 102 (0x66) */&&lbl_f_fsub, \
	/* 103 (0x67) */&&lbl_f_dsub, \
	/* 104 (0x68) */&&lbl_f_imul, \
	/* 105 (0x69) */&&lbl_f_lmul, \
	/* 106 (0x6a) */&&lbl_f_fmul, \
	/* 107 (0x6b) */&&lbl_f_dmul, \
	/* 108 (0x6c) */&&lbl_f_idiv, \
	/* 109 (0x6d) */&&lbl_f_ldiv, \
	/* 110 (0x6e) */&&lbl_f_fdiv, \
	/* 111 (0x6f) */&&lbl_f_ddiv, \
	/* 112 (0x70) */&&lbl_f_irem, \
	/* 113 (0x71) */&&lbl_f_lrem, \
	/* 114 (0x72) */&&lbl_f_frem, \
	/* 115 (0x73) */&&lbl_f_drem, \
	/* 116 (0x74) */&&lbl_f_ineg, \
	/* 117 (0x75) */&&lbl_f_lneg, \
	/* 118 (0x76) */&&lbl_f_fneg, \
	/* 119 (0x77) */&&lbl_f_dneg, \
	/* 120 (0x78) */&&lbl_f_ishl, \
	/* 121 (0x79) */&&lbl_f_lshl, \
	/* 122 (0x7a) */&&lbl_f_ishr, \
	/* 123 (0x7b) */&&lbl_f_lshr, \
	/* 124 (0x7c) */&&lbl_f_iushr, \
	/* 125 (0x7d) */&&lbl_f_lushr, \
	/* 126 (0x7e) */&&lbl_f_iand, \
	/* 127 (0x7f) */&&lbl_f_land, \
	/* 128 (0x80) */&&lbl_f_ior, \
	/* 129 (0x81) */&&lbl_f_lor, \
	/* 130 (0x82) */&&lbl_f_ixor, \
	/* 131 (0x83) */&&lbl_f_lxor, \
	/* 132 (0x84) */&&lbl_f_iinc, \
	/* 133 (0x85) */&&lbl_f_i2l, \
	/* 134 (0x86) */&&lbl_f_i2f, \
	/* 135 (0x87) */&&lbl_f_i2d, \
	/* 136 (0x88) */&&lbl_f_l2i, \
	/* 137 (0x89) */&&lbl_f_l2f, \
	/* 138 (0x8a) */&&lbl_f_l2d, \
	/* 139 (0x8b) */&&lbl_f_f2i, \
	/* 140 (0x8c) */&&lbl_f_f2l, \
	/* 141 (0x8d) */&&lbl_f_f2d, \
	/* 142 (0x8e) */&&lbl_f_d2i, \
	/* 143 (0x8f) */&&lbl_f_d2l, \
	/* 144 (0x90) */&&lbl_f_d2f, \
	/* 145 (0x91) */&&lbl_f_i2b, \
	/* 146 (0x92) */&&lbl_f_i2c, \
	/* 147 (0x93) */&&lbl_f_i2s, \
	/* 148 (0x94) */&&lbl_f_lcmp, \
	/* 149 (0x95) */&&lbl_f_fcmpl, \
	/* 150 (0x96) */&&lbl_f_fcmpg, \
	/* 151 (0x97) */&&lbl_f_dcmpl, \
	/* 152 (0x98) */&&lbl_f_dcmpg, \
	/* 153 (0x99) */&&lbl_f_ifeq, \
	/* 154 (0x9a) */&&lbl_f_ifne, \
	/* 155 (0x9b) */&&lbl_f_iflt, \
	/* 156 (0x9c) */&&lbl_f_ifge, \
	/* 157 (0x9d) */&&lbl_f_ifgt, \
	/* 158 (0x9e) */&&lbl_f_ifle, \
	/* 159 (0x9f) */&&lbl_f_if_icmpeq, \
	/* 160 (0xa0) */&&lbl_f_if_icmpne, \
	/* 161 (0xa1) */&&lbl_f_if_icmplt, \
	/* 162 (0xa2) */&&lbl_f_if_icmpge, \
	/* 163 (0xa3) */&&lbl_f_if_icmpgt, \
	/* 164 (0xa4) */&&lbl_f_if_icmple, \
	/* 165 (0xa5) */&&lbl_f_if_acmpeq, \
	/* 166 (0xa6) */&&lbl_f_if_acmpne, \
	/* 167 (0xa7) */&&lbl_f_goto, \
	/* 168 (0xa8) */&&lbl_f_jsr, \
	/* 169 (0xa9) */&&lbl_f_ret, \
	/* 170 (0xaa) */&&lbl_f_tableswitch, \
	/* 171 (0xab) */&&lbl_f_lookupswitch, \
	/* 172 (0xac) */&&lbl_f_ireturn, \
	/* 173 (0xad) */&&lbl_f_lreturn, \
	/* 174 (0xae) */&&lbl_f_freturn, \
	/* 175 (0xaf) */&&lbl_f_dreturn, \
	/* 176 (0xb0) */&&lbl_f_areturn, \
	/* 177 (0xb1) */&&lbl_f_vreturn, \
	/* 178 (0xb2) */&&lbl_f_getstatic, \
	/* 179 (0xb3) */&&lbl_f_putstatic, \
	/* 180 (0xb4) */&&lbl_f_getfield, \
	/* 181 (0xb5) */&&lbl_f_putfield, \
	/* 182 (0xb6) */&&lbl_f_invokevirtual, \
	/* 183 (0xb7) */&&lbl_f_invokespecial, \
	/* 184 (0xb8) */&&lbl_f_invokestatic, \
	/* 185 (0xb9) */&&lbl_f_invokeinterface, \
	/* 186 (0xba) */&&lbl_f_unused, \
	/* 187 (0xbb) */&&lbl_f_new, \
	/* 188 (0xbc) */&&lbl_f_newarray, \
	/* 189 (0xbd) */&&lbl_f_anewarray, \
	/* 190 (0xbe) */&&lbl_f_arraylength, \
	/* 191 (0xbf) */&&lbl_f_athrow, \
	/* 192 (0xc0) */&&lbl_f_checkcast, \
	/* 193 (0xc1) */&&lbl_f_instanceof, \
	/* 194 (0xc2) */&&lbl_f_monitorenter, \
	/* 195 (0xc3) */&&lbl_f_monitorexit, \
	/* 196 (0xc4) */&&lbl_f_wide, \
	/* 197 (0xc5) */&&lbl_f_multianewarray, \
	/* 198 (0xc6) */&&lbl_f_ifnull, \
	/* 199 (0xc7) */&&lbl_f_ifnonnull, \
	/* 200 (0xc8) */&&lbl_f_goto_w, \
	/* 201 (0xc9) */&&lbl_f_jsr_w, \
	/* 202 (0xca) */&&lbl_f_breakpoint, \
	/* 203 (0xcb) */&&lbl_f_thinj_undefined, \
	/* 204 (0xcc) */&&lbl_f_thinj_undefined, \
	/* 205 (0xcd) */&&lbl_f_thinj_undefined, \
	/* 206 (0xce) */&&lbl_f_thinj_undefined, \
	/* 207 (0xcf) */&&lbl_f_thinj_undefined, \
	/* 208 (0xd0) */&&lbl_f_thinj_undefined, \
	/* 209 (0xd1) */&&lbl_f_thinj_undefined, \
	/* 210 (0xd2) */&&lbl_f_thinj_undefined, \
	/* 211 (0xd3) */&&lbl_f_thinj_undefined, \
	/* 212 (0xd4) */&&lbl_f_thinj_undefined, \
	/* 213 (0xd5) */&&lbl_f_thinj_undefined, \
	/* 214 (0xd6) */&&lbl_f_thinj_undefined, \
	/* 215 (0xd7) */&&lbl_f_thinj_undefined, \
	/* 216 (0xd8) */&&lbl_f_thinj_undefined, \
	/* 217 (0xd9) */&&lbl_f_thinj_undefined, \
	/* 218 (0xda) */&&lbl_f_thinj_undefined, \
	/* 219 (0xdb) */&&lbl_f_thinj_undefined, \
	/* 220 (0xdc) */&&lbl_f_thinj_undefined, \
	/* 221 (0xdd) */&&lbl_f_thinj_undefined, \
	/* 222 (0xde) */&&lbl_f_thinj_undefined, \
	/* 223 (0xdf) */&&lbl_f_thinj_undefined, \
	/* 224 (0xe0) */&&lbl_f_thinj_undefined, \
	/* 225 (0xe1) */&&lbl_f_thinj_undefined, \
	/* 226 (0xe2) */&&lbl_f_thinj_undefined, \
	/* 227 (0xe3) */&&lbl_f_thinj_undefined, \
	/* 228 (0xe4) */&&lbl_f_thinj_undefined, \
	/* 229 (0xe5) */&&lbl_f_thinj_undefined, \
	/* 230 (0xe6) */&&lbl_f_thinj_undefined, \
	/* 231 (0xe7) */&&lbl_f_thinj_undefined, \
	/* 232 (0xe8) */&&lbl_f_thinj_undefined, \
	/* 233 (0xe9) */&&lbl_f_thinj_undefined, \
	/* 234 (0xea) */&&lbl_f_thinj_undefined, \
	/* 235 (0xeb) */&&lbl_f_thinj_undefined, \
	/* 236 (0xec) */&&lbl_f_thinj_undefined, \
	/* 237 (0xed) */&&lbl_f_thinj_undefined, \
	/* 238 (0xee) */&&lbl_f_thinj_undefined, \
	/* 239 (0xef) */&&lbl_f_thinj_undefined, \
	/* 240 (0xf0) */&&lbl_f_thinj_undefined, \
	/* 241 (0xf1) */&&lbl_f_thinj_undefined, \
	/* 242 (0xf2) */&&lbl_f_thinj_undefined, \
	/* 243 (0xf3) */&&lbl_f_thinj_undefined, \
	/* 244 (0xf4) */&&lbl_f_thinj_undefined, \
	/* 245 (0xf5) */&&lbl_f_thinj_undefined, \
	/* 246 (0xf6) */&&lbl_f_thinj_undefined, \
	/* 247 (0xf7) */&&lbl_f_thinj_undefined, \
	/* 248 (0xf8) */&&lbl_f_thinj_undefined, \
	/* 249 (0xf9) */&&lbl_f_thinj_undefined, \
	/* 250 (0xfa) */&&lbl_f_thinj_undefined, \
	/* 251 (0xfb) */&&lbl_f_thinj_undefined, \
	/* 252 (0xfc) */&&lbl_f_thinj_undefined, \
	/* 253 (0xfd) */&&lbl_f_thinj_undefined, \
	/* 254 (0xfe) */&&lbl_f_impdep1, \
	/* 255 (0xff) */&&lbl_f_impdep2, \
}

/**
 * Table for opcode and the associated instruction
 */
typedef struct __inWithOpcode {
	instruction ins;
	u1 opcode;
	char* name;
	u1 length;
} insWithOpcode;

// Testing:
extern const instruction allIns[];

extern const insWithOpcode allInstructions[];

#endif /* INSTRUCTIONS_H_ */

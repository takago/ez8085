
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *  
 *                       Copyright 2009 Daisuke TAKAGO
 *                   takago@neptune.kanazawa-it.ac.jp
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  This file is part of  "EZ8085"
 *
 *  EZ8085 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EZ8085 program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with EZ8085.  If not, see <http://www.gnu.org/licenses/>.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "iset8085.h"
#include "cpu8085.h"



// レジスタ
struct REG reg;

// 64KBメモリ
BYTE mem[MEMSIZE];

static void chk_p(BYTE X)
{
	int cnt = 8;
	int p = 1;
	/* parity */
	while (0 < cnt) {
		if ((X & 1) == 1)
			p = 1 - p;
		X >>= 1;
		cnt--;
	}
	reg.F.bit.p = p;
}

static void chk_c(WORD X)
{
	reg.F.bit.c = (X >> 8) & 1;
}

static void chk_s(BYTE X)
{
	reg.F.bit.s = (X >> 7) & 1;
}

static void chk_z(BYTE X)
{
	reg.F.bit.z = (X == 0) ? 1 : 0;
}

static void chk_a(BYTE X, BYTE oldX)
{
	if ((oldX & 0xF0) != (X & 0xF0))
		reg.F.bit.a = 1;
	else
		reg.F.bit.a = 0;
}


static void chk_all(WORD X, WORD oldX)
{
	chk_a(X, oldX);
	chk_p(X);
	chk_s(X);
	chk_z(X);
	chk_c(X);
}


static void push(WORD data)
{
	reg.SP--;
	mem[reg.SP] = data >> 8;
	reg.SP--;
	mem[reg.SP] = data;
}

static WORD pop(void)
{
	WORD tmp;
	tmp = mem[reg.SP];
	reg.SP++;
	tmp |= mem[reg.SP] << 8;
	reg.SP++;
	return tmp;
}



void reg_reset()
{
	reg.PC = 0x8000;
	reg.SP = 0xFFFF;
	reg.A = 0;
	reg.BC = 0;
	reg.DE = 0;
	reg.HL = 0;
	chk_p(reg.A);
	chk_c(reg.A);
	chk_s(reg.A);
	chk_z(reg.A);
}

// 命令長を返す
int disas(int adr)
{
	int i;

	for (i = 0; i < N_INST; i++) {
		if (mem[adr] == inst[i].code) {
			break;
		}
	}
	if (i == N_INST) {
		printf("%02hhX      -->  (unknown instruction)\n", mem[adr]);
		return 0;
	}

	if (inst[i].len == 2) {
		printf("%02hhX%02hhX    -->  %s%02hhXh\n", mem[adr], mem[adr + 1],
			   inst[i].str, mem[adr + 1]);
	}
	else if (inst[i].len == 3) {
		printf("%02hhX%02hhX%02hhX  -->  %s%02hhX%02hhXh\n", mem[adr],
			   mem[adr + 1], mem[adr + 2], inst[i].str, mem[adr + 2],
			   mem[adr + 1]);
	}
	else {
		printf("%02hhX      -->  %s\n", mem[adr], inst[i].str);
	}
	return inst[i].len;
}



/* return 1:ならHLT
 * それ以外は0
 */
int exec()
{
	WORD adr = reg.PC;
	unsigned int tmp;
	WORD tword;
	WORD oldX;

	reg.PC += disas(adr);
	switch (mem[adr]) {
	case 0xCE:
		oldX = reg.A;
		reg.A += mem[adr + 1] + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8F:
		oldX = reg.A;
		reg.A += reg.A + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x88:
		oldX = reg.A;
		reg.A += (reg.BC >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x89:
		oldX = reg.A;
		reg.A += (reg.BC & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8A:
		oldX = reg.A;
		reg.A += (reg.DE >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8B:
		oldX = reg.A;
		reg.A += (reg.DE & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8C:
		oldX = reg.A;
		reg.A += (reg.HL >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8D:
		oldX = reg.A;
		reg.A += (reg.HL & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x8E:
		oldX = reg.A;
		reg.A += mem[reg.HL] + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x87:
		oldX = reg.A;
		reg.A += reg.A;
		chk_all(reg.A, oldX);
		break;
	case 0x80:
		oldX = reg.A;
		reg.A += (reg.BC >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x81:
		oldX = reg.A;
		reg.A += (reg.BC & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x82:
		oldX = reg.A;
		reg.A += (reg.DE >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x83:
		oldX = reg.A;
		reg.A += (reg.DE & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x84:
		oldX = reg.A;
		reg.A += (reg.HL >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x85:
		oldX = reg.A;
		reg.A += (reg.HL & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x86:
		oldX = reg.A;
		reg.A += mem[reg.HL];
		chk_p(reg.A);
		chk_all(reg.A, oldX);
		break;
	case 0xC6:
		oldX = reg.A;
		reg.A += mem[adr + 1];
		chk_all(reg.A, oldX);
		break;
	case 0xA7:
		reg.A &= reg.A;
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA0:
		reg.A &= (reg.BC >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA1:
		reg.A &= (reg.BC & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA2:
		reg.A &= (reg.DE >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA3:
		reg.A &= (reg.DE & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
	case 0xA4:
		reg.A &= (reg.HL >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA5:
		reg.A &= (reg.HL & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA6:
		reg.A &= mem[reg.HL];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xE6:
		reg.A &= mem[adr + 1];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xCD:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		push(reg.PC);
		reg.PC = tmp;
		break;
	case 0xDC:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.c == 1) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xD4:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.c == 0) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xF4:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.s == 0) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xFC:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.s == 1) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xEC:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.p == 1) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xE4:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.p == 0) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xCC:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.z == 1) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0xC4:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.z == 0) {
			push(reg.PC);
			reg.PC = tmp;
		}
		break;
	case 0x2F:
		reg.A = ~reg.A;
		break;
	case 0x3F:
		reg.F.bit.c = ~reg.F.bit.c;
		break;
	case 0xBF:	// CMP A
		oldX = reg.A;
		tmp = reg.A - (reg.A & 0xFF);
		chk_all(tmp, oldX);
		break;
	case 0xB8:	// CMP B
		oldX = reg.A;
		tmp = reg.A - (reg.BC >> 8);
		chk_all(tmp, oldX);
		break;
	case 0xB9:	// CMP C
		oldX = reg.A;
		tmp = reg.A - (reg.BC & 0xFF);
		chk_all(tmp, oldX);
		break;
	case 0xBA:	// CMP D
		oldX = reg.A;
		tmp = reg.A - (reg.DE >> 8);
		chk_all(tmp, oldX);
		break;
	case 0xBB:	// CMP E
		oldX = reg.A;
		tmp = reg.A - (reg.DE & 0xFF);
		chk_all(tmp, oldX);
		break;
	case 0xBC:	// CMP H
		oldX = reg.A;
		tmp = reg.A - (reg.HL >> 8);
		chk_all(tmp, oldX);
		break;
	case 0xBD:	// CMP L
		oldX = reg.A;
		tmp = reg.A - (reg.HL & 0xFF);
		chk_all(tmp, oldX);
		break;
	case 0xBE:	// CMP M
		oldX = reg.A;
		tmp = reg.A - mem[reg.HL];
		chk_all(tmp, oldX);
		break;
	case 0xFE:	// CPI
		oldX = reg.A;
		tmp = reg.A - mem[adr + 1];
		chk_all(tmp, oldX);
		break;
	case 0x27:	// DAA
		oldX = reg.A;
		// 下位桁補正
		if ((0xA <= (reg.A & 0x0F)) || (reg.F.bit.a == 1))
			reg.A = (reg.A & 0xF0) | ((reg.A & 0x0F) + 6);
		// 上位桁補正   
		if ((0xA0 <= (reg.A & 0xF0)) || (reg.F.bit.c == 1))
			reg.A = ((reg.A & 0xF0) + 0x60) | (reg.A & 0x0F);
		chk_all(reg.A, oldX);
		break;
	case 0x09:	// DAD B
		tmp = reg.HL + reg.BC;
		reg.HL = tmp;
		reg.F.bit.c = tmp >> 16;
		break;
	case 0x19:	// DAD D
		tmp = reg.HL + reg.DE;
		reg.HL = tmp;
		reg.F.bit.c = tmp >> 16;
		break;
	case 0x29:	// DAD H
		tmp = reg.HL + reg.HL;
		reg.HL = tmp;
		reg.F.bit.c = tmp >> 16;
		break;
		break;
	case 0x39:	// DAD SP
		tmp = reg.HL + reg.SP;
		reg.HL = tmp;
		reg.F.bit.c = tmp >> 16;
		break;
	case 0x3D:
		oldX = reg.A;
		reg.A = (reg.A & 0xFF) - 1;
		chk_s(reg.A);
		chk_a(reg.A, oldX);
		chk_z(reg.A);
		chk_p(reg.A);
		break;
	case 0x05:
		oldX = reg.BC >> 8;
		reg.BC = (((reg.BC >> 8) - 1) << 8) + (reg.BC & 0xFF);
		chk_s(reg.BC >> 8);
		chk_a(reg.BC >> 8, oldX);
		chk_z(reg.BC >> 8);
		chk_p(reg.BC >> 8);
		break;
	case 0x0D:
		oldX = reg.BC & 0xFF;
		reg.BC = (((reg.BC & 0xFF) - 1) & 0xFF) + (reg.BC & 0xFF00);
		chk_s(reg.BC);
		chk_a(reg.BC, oldX);
		chk_z(reg.BC);
		chk_p(reg.BC);
		break;
	case 0x15:
		oldX = reg.DE >> 8;
		reg.DE = (((reg.DE >> 8) - 1) << 8) + (reg.DE & 0xFF);
		chk_s(reg.DE >> 8);
		chk_a(reg.DE >> 8, oldX);
		chk_z(reg.DE >> 8);
		chk_p(reg.DE >> 8);
		break;
	case 0x1D:
		oldX = reg.DE & 0xFF;
		reg.DE = (((reg.DE & 0xFF) - 1) & 0xFF) + (reg.DE & 0xFF00);
		chk_s(reg.DE);
		chk_a(reg.DE, oldX);
		chk_z(reg.DE);
		chk_p(reg.DE);
		break;
	case 0x25:
		oldX = reg.HL >> 8;
		reg.HL = (((reg.HL >> 8) - 1) << 8) + (reg.HL & 0xFF);
		chk_s(reg.HL >> 8);
		chk_a(reg.HL >> 8, oldX);
		chk_z(reg.HL >> 8);
		chk_p(reg.HL >> 8);
		break;
	case 0x2D:
		oldX = reg.HL & 0xFF;
		reg.HL = (((reg.HL & 0xFF) - 1) & 0xFF) + (reg.HL & 0xFF00);
		chk_s(reg.HL);
		chk_a(reg.HL, oldX);
		chk_z(reg.HL);
		chk_p(reg.HL);
		break;
	case 0x35:
		oldX = mem[reg.HL];
		mem[reg.HL] = mem[reg.HL] - 1;
		chk_s(mem[reg.HL]);
		chk_a(mem[reg.HL], oldX);
		chk_z(mem[reg.HL]);
		chk_p(mem[reg.HL]);
		break;
	case 0x0B:
		reg.BC--;
		break;
	case 0x1B:
		reg.DE--;
		break;
	case 0x2B:
		reg.HL--;
		break;
	case 0x3B:
		reg.SP--;
		break;
	case 0xF3:
		/* 未実装　 */
		break;
	case 0xFB:
		/* 未実装　 */
		break;
	case 0x76:
		reg.PC += 1;
		return 1;
		break;
	case 0xDB:
		/* 未実装　 */
		break;
	case 0x3C:
		oldX = reg.A;
		reg.A = (reg.A & 0xFF) + 1;
		chk_s(reg.A);
		chk_a(reg.A, oldX);
		chk_z(reg.A);
		chk_p(reg.A);
		break;
	case 0x04:
		oldX = reg.BC >> 8;
		reg.BC = (((reg.BC >> 8) + 1) << 8) + (reg.BC & 0xFF);
		chk_s(reg.BC >> 8);
		chk_a(reg.BC >> 8, oldX);
		chk_z(reg.BC >> 8);
		chk_p(reg.BC >> 8);
		break;
	case 0x0C:
		oldX = reg.BC & 0xFF;
		reg.BC = (((reg.BC & 0xFF) + 1) & 0xFF) + (reg.BC & 0xFF00);
		chk_s(reg.BC);
		chk_a(reg.BC, oldX);
		chk_z(reg.BC);
		chk_p(reg.BC);
		break;
	case 0x14:
		oldX = reg.DE >> 8;
		reg.DE = (((reg.DE >> 8) + 1) << 8) + (reg.DE & 0xFF);
		chk_s(reg.DE >> 8);
		chk_a(reg.DE >> 8, oldX);
		chk_z(reg.DE >> 8);
		chk_p(reg.DE >> 8);
		break;
	case 0x1C:
		oldX = reg.DE & 0xFF;
		reg.DE = (((reg.DE & 0xFF) + 1) & 0xFF) + (reg.DE & 0xFF00);
		chk_s(reg.DE);
		chk_a(reg.DE, oldX);
		chk_z(reg.DE);
		chk_p(reg.DE);
		break;
	case 0x24:
		oldX = reg.HL >> 8;
		reg.HL = (((reg.HL >> 8) + 1) << 8) + (reg.HL & 0xFF);
		chk_s(reg.HL >> 8);
		chk_a(reg.HL >> 8, oldX);
		chk_z(reg.HL >> 8);
		chk_p(reg.HL >> 8);
		break;
	case 0x2C:
		oldX = reg.HL & 0xFF;
		reg.HL = (((reg.HL & 0xFF) + 1) & 0xFF) + (reg.HL & 0xFF00);
		chk_s(reg.HL);
		chk_a(reg.HL, oldX);
		chk_z(reg.HL);
		chk_p(reg.HL);
		break;
	case 0x34:
		oldX = mem[reg.HL];
		mem[reg.HL] = mem[reg.HL] + 1;
		chk_s(mem[reg.HL]);
		chk_a(mem[reg.HL], oldX);
		chk_z(mem[reg.HL]);
		chk_p(mem[reg.HL]);
		break;
	case 0x03:
		reg.BC++;
		break;
	case 0x13:
		reg.DE++;
		break;
	case 0x23:
		reg.HL++;
		break;
	case 0x33:
		reg.SP++;
		break;
	case 0xC3:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.PC = tmp;
		break;
	case 0xDA:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.c == 1) {
			reg.PC = tmp;
		}
		break;
	case 0xD2:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.c == 0) {
			reg.PC = tmp;
		}
		break;
	case 0xF2:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.s == 0) {
			reg.PC = tmp;
		}
		break;
	case 0xFA:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.s == 1) {
			reg.PC = tmp;
		}
		break;
	case 0xEA:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.p == 1) {
			reg.PC = tmp;
		}
		break;
	case 0xE2:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.p == 0) {
			reg.PC = tmp;
		}
		break;
	case 0xCA:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.z == 1) {
			reg.PC = tmp;
		}
		break;
	case 0xC2:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		if (reg.F.bit.z == 0) {
			reg.PC = tmp;
		}
		break;
	case 0x3A:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.A = mem[tmp];
		break;
	case 0x0A:
		reg.A = mem[reg.BC];
		break;
	case 0x1A:
		reg.A = mem[reg.DE];
		break;
	case 0x2A:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.HL = (mem[tmp + 1] << 8) | mem[tmp];
		break;
	case 0x01:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.BC = tmp;
		break;
	case 0x11:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.DE = tmp;
		break;
	case 0x21:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.HL = tmp;
		break;
	case 0x31:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		reg.SP = tmp;
		break;
	case 0x7F:
		reg.A = reg.A;
		break;
	case 0x78:
		reg.A = reg.BC >> 8;
		break;
	case 0x79:
		reg.A = reg.BC & 0xFF;
		break;
	case 0x7A:
		reg.A = reg.DE >> 8;
		break;
	case 0x7B:
		reg.A = reg.DE & 0xFF;
		break;
	case 0x7C:
		reg.A = reg.HL >> 8;
		break;
	case 0x7D:
		reg.A = reg.HL & 0xFF;
		break;
	case 0x47:
		reg.BC = ((reg.A & 0xFF) << 8) | (reg.BC & 0xFF);
		break;
	case 0x40:
		reg.BC = (reg.BC & 0xFF00) | (reg.BC & 0xFF);
		break;
	case 0x41:
		reg.BC = ((reg.BC & 0xFF) << 8) | (reg.BC & 0xFF);
		break;
	case 0x42:
		reg.BC = (reg.DE & 0xFF00) | (reg.BC & 0xFF);
		break;
	case 0x43:
		reg.BC = ((reg.DE & 0xFF) << 8) | (reg.BC & 0xFF);
		break;
	case 0x44:
		reg.BC = (reg.HL & 0xFF00) | (reg.BC & 0xFF);
		break;
	case 0x45:
		reg.BC = ((reg.HL & 0xFF) << 8) | (reg.BC & 0xFF);
		break;
	case 0x4F:
		reg.BC = (reg.A & 0xFF) | (reg.BC & 0xFF00);
		break;
	case 0x48:
		reg.BC = (reg.BC >> 8) | (reg.BC & 0xFF00);
		break;
	case 0x49:
		reg.BC = (reg.BC & 0xFF) | (reg.BC & 0xFF00);
		break;
	case 0x4A:
		reg.BC = (reg.DE >> 8) | (reg.BC & 0xFF00);
		break;
	case 0x4B:
		reg.BC = (reg.DE & 0xFF) | (reg.BC & 0xFF00);
		break;
	case 0x4C:
		reg.BC = (reg.HL >> 8) | (reg.BC & 0xFF00);
		break;
	case 0x4D:
		reg.BC = (reg.HL & 0xFF) | (reg.BC & 0xFF00);
		break;
	case 0x57:
		reg.DE = ((reg.A & 0xFF) << 8) | (reg.DE & 0xFF);
		break;
	case 0x50:
		reg.DE = (reg.BC & 0xFF00) | (reg.DE & 0xFF);
		break;
	case 0x51:
		reg.DE = ((reg.BC & 0xFF) << 8) | (reg.DE & 0xFF);
		break;
	case 0x52:
		reg.DE = (reg.DE & 0xFF00) | (reg.DE & 0xFF);
		break;
	case 0x53:
		reg.DE = ((reg.DE & 0xFF) << 8) | (reg.DE & 0xFF);
		break;
	case 0x54:
		reg.DE = (reg.HL & 0xFF00) | (reg.DE & 0xFF);
		break;
	case 0x55:
		reg.DE = ((reg.HL & 0xFF) << 8) | (reg.DE & 0xFF);
		break;
	case 0x5F:
		reg.DE = (reg.A & 0xFF) | (reg.DE & 0xFF00);
		break;
	case 0x58:
		reg.DE = (reg.BC >> 8) | (reg.DE & 0xFF00);
		break;
	case 0x59:
		reg.DE = (reg.BC & 0xFF) | (reg.DE & 0xFF00);
		break;
	case 0x5A:
		reg.DE = (reg.DE >> 8) | (reg.DE & 0xFF00);
		break;
	case 0x5B:
		reg.DE = (reg.DE & 0xFF) | (reg.DE & 0xFF00);
		break;
	case 0x5C:
		reg.DE = (reg.HL >> 8) | (reg.DE & 0xFF00);
		break;
	case 0x5D:
		reg.DE = (reg.HL & 0xFF) | (reg.DE & 0xFF00);
		break;
	case 0x67:
		reg.HL = ((reg.A & 0xFF) << 8) | (reg.HL & 0xFF);
		break;
	case 0x60:
		reg.HL = (reg.BC & 0xFF00) | (reg.HL & 0xFF);
		break;
	case 0x61:
		reg.HL = ((reg.BC & 0xFF) << 8) | (reg.HL & 0xFF);
		break;
	case 0x62:
		reg.HL = (reg.DE & 0xFF00) | (reg.HL & 0xFF);
		break;
	case 0x63:
		reg.HL = ((reg.DE & 0xFF) << 8) | (reg.HL & 0xFF);
		break;
	case 0x64:
		reg.HL = (reg.HL & 0xFF00) | (reg.HL & 0xFF);
		break;
	case 0x65:
		reg.HL = ((reg.HL & 0xFF) << 8) | (reg.HL & 0xFF);
		break;
	case 0x6F:
		reg.HL = (reg.A & 0xFF) | (reg.HL & 0xFF00);
		break;
	case 0x68:
		reg.HL = (reg.BC >> 8) | (reg.HL & 0xFF00);
		break;
	case 0x69:
		reg.HL = (reg.BC & 0xFF) | (reg.HL & 0xFF00);
		break;
	case 0x6A:
		reg.HL = (reg.DE >> 8) | (reg.HL & 0xFF00);
		break;
	case 0x6B:
		reg.HL = (reg.DE & 0xFF) | (reg.HL & 0xFF00);
		break;
	case 0x6C:
		reg.HL = (reg.HL >> 8) | (reg.HL & 0xFF00);
		break;
	case 0x6D:
		reg.HL = (reg.HL & 0xFF) | (reg.HL & 0xFF00);
		break;
	case 0x77:
		mem[reg.HL] = reg.A;
		break;
	case 0x70:
		mem[reg.HL] = reg.BC >> 8;
		break;
	case 0x71:
		mem[reg.HL] = reg.BC;
		break;
	case 0x72:
		mem[reg.HL] = reg.DE >> 8;
		break;
	case 0x73:
		mem[reg.HL] = reg.DE;
		break;
	case 0x74:
		mem[reg.HL] = reg.HL >> 8;
		break;
	case 0x75:
		mem[reg.HL] = reg.HL;
		break;
	case 0x7E:
		reg.A = mem[reg.HL];
		break;
	case 0x46:
		reg.BC = mem[reg.HL] << 8 | (reg.BC & 0x00FF);
		break;
	case 0x4E:
		reg.BC = mem[reg.HL] | (reg.BC & 0xFF00);
		break;
	case 0x56:
		reg.DE = mem[reg.HL] << 8 | (reg.DE & 0x00FF);
		break;
	case 0x5E:
		reg.DE = mem[reg.HL] | (reg.DE & 0xFF00);
		break;
	case 0x66:
		reg.HL = mem[reg.HL] << 8 | (reg.HL & 0x00FF);
		break;
	case 0x6E:
		reg.HL = mem[reg.HL] | (reg.HL & 0xFF00);
		break;
	case 0x3E:
		reg.A = mem[adr + 1];
		break;
	case 0x06:
		reg.BC = mem[adr + 1] << 8 | (reg.BC & 0x00FF);
		break;
	case 0x0E:
		reg.BC = mem[adr + 1] | (reg.BC & 0xFF00);
		break;
	case 0x16:
		reg.DE = mem[adr + 1] << 8 | (reg.DE & 0x00FF);
		break;
	case 0x1E:
		reg.DE = mem[adr + 1] | (reg.DE & 0xFF00);
		break;
	case 0x26:
		reg.HL = mem[adr + 1] << 8 | (reg.HL & 0x00FF);
		break;
	case 0x2E:
		reg.HL = mem[adr + 1] | (reg.HL & 0xFF00);
		break;
	case 0x36:
		mem[reg.HL] = mem[adr + 1];
		break;
	case 0x00:
		break;
	case 0xB7:
		reg.A |= reg.A;
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB0:
		reg.A |= (reg.BC >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB1:
		reg.A |= (reg.BC & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB2:
		reg.A |= (reg.DE >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB3:
		reg.A |= (reg.DE & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB4:
		reg.A |= (reg.HL >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB5:
		reg.A |= (reg.HL & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xB6:
		reg.A |= mem[reg.HL];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xF6:
		reg.A |= mem[adr + 1];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xD3:
		/*未実装 */
		break;
	case 0xE9:
		reg.PC = reg.HL;
		break;
	case 0xC1:
		reg.BC = pop();
		break;
	case 0xD1:
		reg.DE = pop();
		break;
	case 0xE1:
		reg.HL = pop();
		break;
	case 0xF1:
		tword = pop();
		reg.F.byte = tword & 0xFF;
		reg.A = (tword >> 8);
		break;
	case 0xC5:
		push(reg.BC);
		break;
	case 0xD5:
		push(reg.DE);
		break;
	case 0xE5:
		push(reg.HL);
		break;
	case 0xF5:
		tword = reg.F.byte | (reg.A << 8);
		push(tword);
		break;
	case 0x17:	// RAL
		tmp = reg.F.bit.c;
		reg.F.bit.c = reg.A >> 7;
		reg.A <<= 1;
		reg.A |= tmp;
		break;
	case 0x1F:	// RAR
		tmp = reg.F.bit.c;
		reg.F.bit.c = reg.A & 1;
		reg.A >>= 1;
		reg.A |= (tmp << 7);
		break;
	case 0x07:
		reg.F.bit.c = reg.A >> 7;
		reg.A <<= 1;
		reg.A |= reg.F.bit.c & 1;
		break;
	case 0x0F:
		reg.F.bit.c = reg.A & 1;
		reg.A >>= 1;
		reg.A |= reg.F.bit.c << 7;
		break;
	case 0xC9:
		reg.PC = pop();
		break;
	case 0xD8:
		if (reg.F.bit.c == 1) {
			reg.PC = pop();
		}
		break;
	case 0xD0:
		if (reg.F.bit.c == 0) {
			reg.PC = pop();
		}
		break;
	case 0xF0:
		if (reg.F.bit.s == 0) {
			reg.PC = pop();
		}
		break;
	case 0xF8:
		if (reg.F.bit.s == 1) {
			reg.PC = pop();
		}
		break;
	case 0xE8:
		if (reg.F.bit.p == 1) {
			reg.PC = pop();
		}
		break;
	case 0xE0:
		if (reg.F.bit.p == 0) {
			reg.PC = pop();
		}
		break;
	case 0xC8:
		if (reg.F.bit.z == 1) {
			reg.PC = pop();
		}
		break;
	case 0xC0:
		if (reg.F.bit.z == 0) {
			reg.PC = pop();
		}
		break;
	case 0x20:
		/*未実装 */
		break;
	case 0xC7:
		/*未実装 */
		break;
	case 0xCF:
		/*未実装 */
		break;
	case 0xD7:
		/*未実装 */
		break;
	case 0xDF:
		/*未実装 */
		break;
	case 0xE7:
		/*未実装 */
		break;
	case 0xEF:
		/*未実装 */
		break;
	case 0xF7:
		/*未実装 */
		break;
	case 0xFF:
		/*未実装 */
		break;
	case 0xDE:
		oldX = reg.A;
		reg.A -= mem[adr + 1] + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9F:
		oldX = reg.A;
		reg.A -= reg.A + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x98:
		oldX = reg.A;
		reg.A -= (reg.BC >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x99:
		oldX = reg.A;
		reg.A -= (reg.BC & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9A:
		oldX = reg.A;
		reg.A -= (reg.DE >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9B:
		oldX = reg.A;
		reg.A -= (reg.DE & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9C:
		oldX = reg.A;
		reg.A -= (reg.HL >> 8) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9D:
		oldX = reg.A;
		reg.A -= (reg.HL & 0xFF) + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x9E:
		oldX = reg.A;
		reg.A -= mem[reg.HL] + reg.F.bit.c;
		chk_all(reg.A, oldX);
		break;
	case 0x97:
		oldX = reg.A;
		reg.A -= reg.A;
		chk_all(reg.A, oldX);
		break;
	case 0x90:
		oldX = reg.A;
		reg.A -= (reg.BC >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x91:
		oldX = reg.A;
		reg.A -= (reg.BC & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x92:
		oldX = reg.A;
		reg.A -= (reg.DE >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x93:
		oldX = reg.A;
		reg.A -= (reg.DE & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x94:
		oldX = reg.A;
		reg.A -= (reg.HL >> 8);
		chk_all(reg.A, oldX);
		break;
	case 0x95:
		oldX = reg.A;
		reg.A -= (reg.HL & 0xFF);
		chk_all(reg.A, oldX);
		break;
	case 0x96:
		oldX = reg.A;
		reg.A -= mem[reg.HL];
		chk_all(reg.A, oldX);
		break;
	case 0xD6:
		oldX = reg.A;
		reg.A -= mem[adr + 1];
		chk_all(reg.A, oldX);
		break;
	case 0x22:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		mem[tmp] = reg.HL & 0xFF;
		mem[tmp + 1] = (reg.HL >> 8);
		break;
	case 0x30:
		/*未実装 */
		break;
	case 0xF9:
		reg.SP = reg.HL;
		break;
	case 0x32:
		tmp = (mem[adr + 2] << 8) + mem[adr + 1];
		mem[tmp] = reg.A & 0xFF;
		break;
	case 0x02:
		mem[reg.BC] = reg.A & 0xFF;
		break;
	case 0x12:
		mem[reg.DE] = reg.A & 0xFF;
		break;
	case 0x37:
		reg.F.bit.c = 1;
		break;
	case 0xEB:
		tmp = reg.HL;
		reg.HL = reg.DE;
		reg.DE = tmp;
		break;
	case 0xAF:
		reg.A ^= reg.A;
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA8:
		reg.A ^= (reg.BC >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xA9:
		reg.A ^= (reg.BC & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xAA:
		reg.A ^= (reg.DE >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xAB:
		reg.A ^= (reg.DE & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
	case 0xAC:
		reg.A ^= (reg.HL >> 8);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xAD:
		reg.A ^= (reg.HL & 0xFF);
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xAE:
		reg.A ^= mem[reg.HL];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xEE:
		reg.A ^= mem[adr + 1];
		chk_s(reg.A);
		chk_z(reg.A);
		chk_p(reg.A);
		reg.F.bit.c = 0;
		reg.F.bit.a = 0;
		break;
	case 0xE3:
		reg.HL = (mem[reg.SP + 1] << 8) | mem[reg.SP];
		break;
	default:
		printf(" exec. exception. 8085 was halted.");
		return 1;
		break;
	}
	return 0;
}

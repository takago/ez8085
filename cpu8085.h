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

#ifndef CPU8085_H
#   define CPU8085_H
#   include "common.h"

typedef struct REG
{
	WORD A;
	WORD BC;
	WORD DE;
	WORD HL;
	union flag
	{
		BYTE byte;
		struct
		{
			BYTE c:1;			// carry
			BYTE reserved2:1;
			BYTE p:1;			// parity
			BYTE reserved1:1;
			BYTE a:1;			// aux
			BYTE reserved0:1;
			BYTE z:1;			// zero
			BYTE s:1;			// sign
		} __attribute__ ((packed)) bit;
	} F;
	WORD PC;
	WORD SP;
} REG;

extern struct REG reg;
extern BYTE mem[];

// レジスタリセット
void reg_reset();

// 命令長を返す
int disas(int adr);

/* 1命令実行する
 * 戻り値 1 : HLTを実行
 *        0 : 他
 */
int exec();

#endif

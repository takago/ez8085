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


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "symbol.h"

int sym_entry(struct SYMBOL_TABLE *sym_tbl, char *sname)
{
	if (sym_tbl == NULL)
		return ERR_SYMBOL;

	int i;
	// シンボルが既に登録されているか調べる
	for (i = 0; i < sym_tbl->size; i++) {
		if (strcmp(sym_tbl->symbol[i].name, sname) == 0) {
			return ERR_SYMBOL;
		}
	}
	// 新しいシンボルをシンボルテーブルに追加する
	sym_tbl->symbol[i].flag = 0;
	sym_tbl->symbol[i].addr = sym_tbl->size;
	strcpy(sym_tbl->symbol[i].name, sname);
	sym_tbl->size++;	// シンボルの数をひとつ増やす
	return sym_tbl->symbol[i].addr;
}

// シンボルはあるが，アドレスが確定してない:0,アドレスが確定した:1，シンボルがないか:ERR_SYMBOL 調べる
int sym_getflag(struct SYMBOL_TABLE *sym_tbl, char *sname)
{
	int i;
	for (i = 0; i < sym_tbl->size; i++) {
		if (strcmp(sym_tbl->symbol[i].name, sname) == 0) {
			return sym_tbl->symbol[i].flag;
		}
	}
	return ERR_SYMBOL;
}

// シンボルテーブルに登録されたシンボルの割り当てアドレスを変更する
int sym_setadr(struct SYMBOL_TABLE *sym_tbl, char *sname, int newadr)
{
	if (sym_tbl == NULL)
		return ERR_SYMBOL;
	int i;
	for (i = 0; i < sym_tbl->size; i++) {
		if (strcmp(sym_tbl->symbol[i].name, sname) == 0) {
			sym_tbl->symbol[i].addr = newadr;
			sym_tbl->symbol[i].flag = 1;
			return newadr;
		}
	}
	return ERR_SYMBOL;
}

int sym_getadr(struct SYMBOL_TABLE *sym_tbl, char *sname)
{
	if (sym_tbl == NULL)
		return ERR_SYMBOL;
	int i;
	for (i = 0; i < sym_tbl->size; i++) {
		if (strcmp(sym_tbl->symbol[i].name, sname) == 0) {
			return sym_tbl->symbol[i].addr;
		}
	}
	return ERR_SYMBOL;
}


void init_sym_tbl(struct SYMBOL_TABLE *sym_tbl)
{
	if (sym_tbl == NULL)
		return;
	sym_tbl->size = 0;
}

void disp_sym_tbl(struct SYMBOL_TABLE *sym_tbl)
{
	if (sym_tbl == NULL)
		return;
	int i;
	for (i = 0; i < sym_tbl->size; i++) {
		if (i == 0)
			printf("\n\n  SYMBOL TABLE\n  idx  name       addr\n");
		printf("  %04hX %-10s", i, sym_tbl->symbol[i].name);
		if (sym_tbl->symbol[i].flag == 0)
			printf(" UNKNOWN\n");
		else
			printf(" %04hX\n", sym_tbl->symbol[i].addr);
	}
}

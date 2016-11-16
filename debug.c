
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
#include <ctype.h>

#include "cpu8085.h"
#include "symbol.h"
#include "iset8085.h"

#include<unistd.h>
#include<signal.h>

struct SYMBOL_TABLE sym_tbl;


int cont;						// 連続実行

/* 割込みハンドラの定義(連続実行モードを止めるため) */
void stop()
{
	cont = 0;
}


int strtol2(char *bp, char **endptr)
{
	int tmp;
	// 空白を読み飛ばす
	while (isblank(*bp))
		bp++;
	if (strncmp(bp, "0x", 2) == 0 || strncmp(bp, "0X", 2) == 0)
		tmp = strtol(bp, endptr, 16);
	else {
		tmp = strtol(bp, endptr, 16);
		if (toupper(**endptr) == 'H')
			(*endptr)++;
		else
			tmp = strtol(bp, endptr, 10);
	}
	return tmp;
}


void assem(FILE * fp)
{
	int i;
	int tmp0;
	char *endptr;
	int lc0;
	int lc = reg.PC;			// アセンブリ用のロケーションカウンタ
	char buff[BSIZE];
	char *bp;
	char *bp0;
	WORD KARI_ADR[100];			// 未解決シンボル格納領域
	WORD kari_adr = 0;

	init_sym_tbl(&sym_tbl);	// シンボルテーブル初期化
	printf("  %04hX : ", lc);
	int errf;
	while (fgets(buff, BSIZE, fp) != NULL) {
		if (fp != stdin)
			printf("%s", buff);

		bp = buff + strlen(buff) - 2;
		while (isblank(*bp))
			bp--;
		bp++;
		*bp = '\n';
		bp++;
		*bp = '\0';

		bp = buff;
		while (isblank(*bp))
			bp++;


		/* remove comment  */
		if (*bp == ';') {
			//　コメントはスキップ
			goto ERR_RECOVERY;
		}
		bp = index(buff, ';');
		if (bp != NULL) {
			bp--;
			while (isblank(*bp))
				bp--;
			bp++;
			*bp = '\n';
			bp++;
			*bp = '\0';
		}

		lc0 = lc;
		bp = buff;


		/* 全て大文字に変更 */
		while (isblank(*bp))
			bp++;	// 空白をスキップ

		tmp0 = strlen(bp);
		for (errf = 0, i = 0; i < tmp0; i++) {
			if (strncmp("DS ", bp, 3) == 0)
				break;
			bp[i] = toupper(bp[i]);
		}



		if (*bp == '\0') {
			break;
		}
		if (!isalpha(*bp) && *bp != '\n')
			errf = 1;

		if (errf) {
		  ERR_RECOVERY:
			cup(1);
			printf("  %04hX : ", lc);
			cdr();
			continue;
		}


		/* ラベルを発見したら，シンボルテーブルに登録 */
		bp0 = index(bp, ':');
		if (bp0 != NULL) {
			*bp0 = '\0';
			for (i = 0; i < strlen(bp); i++) {
				if (!isalnum(bp[i]) && (bp[i] != '_')) {
					// ラベルはアルファベットか_であるべき
					goto ERR_RECOVERY;
				}
			}
			sym_entry(&sym_tbl, bp);
			sym_setadr(&sym_tbl, bp, lc);	// アドレスを登録
			cup(1);
			cdr();
			printf("  %04hX : %s:\n", lc, bp);
			bp = bp0 + 1;
			while (isblank(*bp))
				bp++;	// 空白をスキップ
			printf("  %04hX : %s", lc, bp);
			if (*bp == '\n')
				goto ERR_RECOVERY;
		}
		if (strcmp(bp, "\n") == 0) {
			// キー入力の場合は，\nで終了
			// ファイル読み込みの場合は続行
			if (fp == stdin) {
				cup(1);
				break;
			}
			else {
				printf("\n");
				goto ERR_RECOVERY;
			}
		}
		else if (strncmp(bp, "ORG ", 4) == 0) {	// org
			printf("  %04hX", lc);
			lc = strtol2(bp + 4, &endptr);
			cup(1);
			cdr();
			printf(" :   %s", bp);
		}
		else if (strncmp(bp, "DB ", 3) == 0) {	// db
			bp[strlen(bp) - 1] = '\0';
			tmp0 = strtol2(bp + 3, &endptr);
			mem[lc++] = tmp0;
			cup(1);
			cdr();
			printf("  %04hX :   %-10s  -->  %02hhX", lc0, bp, mem[lc - 1]);
			while (*endptr != '\0') {
				endptr++;
				mem[lc++] = strtol2(endptr, &endptr);
				printf("%02hhX", mem[lc - 1]);
			}
			printf("\n");
		}
		else if (strncmp(bp, "DW ", 3) == 0) {	//dw
			bp[strlen(bp) - 1] = '\0';
			tmp0 = strtol2(bp + 3, &endptr);
			mem[lc++] = tmp0;
			cup(1);
			cdr();
			printf("  %04hX :   %-10s  -->  %02hhX", lc0, bp, mem[lc - 1]);
			mem[lc++] = tmp0 >> 8;
			printf("%02hhX", mem[lc - 1]);
			while (*endptr != '\0') {
				endptr++;
				tmp0 = strtol2(endptr, &endptr);
				mem[lc++] = tmp0;
				printf("%02hhX", mem[lc - 1]);
				mem[lc++] = tmp0 >> 8;
				printf("%02hhX", mem[lc - 1]);
			}
			printf("\n");
		}
		else if (strncmp(bp, "DS ", 3) == 0) {
			bp += 3;
			bp[strlen(bp) - 1] = '\0';
			cup(1);
			cdr();
			printf("  %04hX :   DS %-10s  -->  ", lc0, bp);
			while (*bp != '\0') {
				if ((*bp == '\'') && (*(bp + 2) == '\'')) {
					mem[lc++] = *(bp + 1);
					printf("%02hhX", mem[lc - 1]);
					bp += 2;
				}
				else if (*bp == '"') {
					bp++;
					if (index(bp, '"') == NULL)
						break;
					while (*bp != '"') {
						mem[lc++] = *bp;
						printf("%02hhX", mem[lc - 1]);
						bp++;
					}
				}
				bp++;
			}
			printf("\n");
		}
		else {	/* 命令セットテーブルからサーチ */
			for (i = 0; i < N_INST; i++) {
				if (strncmp(bp, inst[i].str, strlen(inst[i].str)) == 0) {
					break;
				}
			}
			if (i == N_INST) {
				// 命令セットになかったらエラー
				goto ERR_RECOVERY;
			}
			mem[lc++] = inst[i].code;

			bp[strlen(bp) - 1] = '\0';
			cup(1);
			cdr();
			printf("  %04hX :   %-10s  -->  %02hhX", lc0, bp, mem[lc - 1]);
			bp += strlen(inst[i].str);
			if (inst[i].len == 2) {	// 1バイト
				tmp0 = strtol2(bp, &endptr);
				mem[lc++] = tmp0;
				printf("%02hhX", mem[lc - 1]);
			}
			else if (inst[i].len == 3) {	/* アドレスを読み出す */
				// 未解決ラベルならシンボルテーブルに登録，シンボルテーブルにあればそれを引き出す
				// 数値ならそのままいれる
				if (isdigit(*bp)) {
					tmp0 = strtol2(bp, &endptr);
				}
				else {
					tmp0 = sym_getadr(&sym_tbl, bp);
					if (tmp0 == ERR_SYMBOL) {
						tmp0 = sym_entry(&sym_tbl, bp);
					}
					if (sym_getflag(&sym_tbl, bp) == 0) {
						KARI_ADR[kari_adr++] = lc;
					}
				}
				mem[lc++] = tmp0;
				printf("%02hhX", mem[lc - 1]);
				mem[lc++] = tmp0 >> 8;
				printf("%02hhX", mem[lc - 1]);

			}
			printf("\n");
		}
		printf("  %04hX : ", lc);
	}


	// アセンブリモードを抜ける前に未解決シンボルを全て解決する

	disp_sym_tbl(&sym_tbl);
	for (i = 0; i < kari_adr; i++) {
		if (i == 0)
			printf("\n  RESOLVING ADDRESSES\n");
		int idx;
		idx = mem[KARI_ADR[i]] + mem[KARI_ADR[i] + 1];
		mem[KARI_ADR[i]] = (sym_tbl.symbol[idx].addr);
		mem[KARI_ADR[i] + 1] = (sym_tbl.symbol[idx].addr) >> 8;
		printf("  %04hX : ", KARI_ADR[i] - 1);
		disas(KARI_ADR[i] - 1);
	}
	printf("\n");
}

void reg_disp()
{
	printf
	  ("   F(sz-a-p-c)  A  B C  D E  H L [HL]   SP    PC NEXT_INSTRUCTION\n");
	printf("  %02hhX(", reg.F.byte);
	printf("%d", reg.F.bit.s);
	printf("%dx", reg.F.bit.z);
	printf("%dx", reg.F.bit.a);
	printf("%dx", reg.F.bit.p);
	printf("%d) ", reg.F.bit.c);
	printf("%02hhX ", reg.A);
	printf("%04hX ", reg.BC);
	printf("%04hX ", reg.DE);
	printf("%04hX  ", reg.HL);
	printf("%02hX  ", mem[reg.HL]);
	printf("%04hX  ", reg.SP);
	printf("%04hX:", reg.PC);
	disas(reg.PC);

}

void mem_dump(int adr, int length)
{
	int i;
	printf("   - HEX DUMP -\n");
	printf("   ADDR : +0 +1 +2 +3 +4 +5 +6 +7 - +8 +9 +A +B +C +D +E +F\n");
	if (adr % 16 != 0) {
		printf("   %04hX : ", adr - (adr % 16));
		for (i = adr - (adr % 16); i < adr; i++) {
			printf("   ");
			if (i % 16 == 7)
				printf("- ");
		}

	}

	for (i = adr; i < adr + length; i++) {
		if (i % 16 == 0)
			printf("   %04hX : ", i);
		printf("%02hhX ", mem[i]);
		if (i % 16 == 7)
			printf("- ");
		if (i % 16 == 15)
			printf("\n");
	}

	if (i % 16 != 0)
		printf("\n");
}

void mem_adump(int adr, int length)
{
	int i;
	printf("   - ASCII DUMP -\n");
	printf("   ADDR : +0 +1 +2 +3 +4 +5 +6 +7 - +8 +9 +A +B +C +D +E +F\n");
	if (adr % 16 != 0) {
		printf("   %04hX : ", adr - (adr % 16));
		for (i = adr - (adr % 16); i < adr; i++) {
			printf("   ");
			if (i % 16 == 7)
				printf("- ");
		}

	}

	for (i = adr; i < adr + length; i++) {
		if (i % 16 == 0)
			printf("   %04hX : ", i);
		if (isprint(mem[i]))
			printf(" %c ", mem[i]);
		else
			printf("XX ");
		if (i % 16 == 7)
			printf("- ");
		if (i % 16 == 15)
			printf("\n");
	}

	if (i % 16 != 0)
		printf("\n");
}


// ブレークポイント
#define MAXBP 100
struct
{
	WORD addr;
	int enable;
} BRKP[MAXBP];

void set_brkp(WORD adr)
{
	int i;
	for (i = 0; i < MAXBP; i++) {
		if (BRKP[i].enable == 0) {
			BRKP[i].enable = 1;
			BRKP[i].addr = adr;
			break;
		}
	}
}

int main()
{


	reg_reset();
	int hlt = 0;
	char buff[BSIZE];
	char *bp;
	char *endptr;

	int tmp0, tmp1, tmp2;
	int i;



	cls();

	printf("+---------------------------------------------------------------\n"
		   "| EZ8085 version %s\n"
		   "|%50s\n|%50s\n", VER,
		   "Copyright 2009 Daisuke TAKAGO", "takago@neptune.kanazawa-it.ac.jp");
	printf
	  ("+---------------------------------------------------------------\n");
	while (1) {
		bp = buff;
		reg_disp();	// レジスタの表示
		printf(" ? ");
		while (fgets(bp, BSIZE, stdin) != NULL) {
			if (strcmp("\n", bp) == 0 || strcmp("step\n", bp) == 0) {
				if (hlt == 1) {
					printf
					  (" 8085 CPU is halted. Type \"resume\" to resume CPU.\n");
					printf(" ? ");
					continue;
				}
				printf
				  ("----------------------------------------------------------------\n");
				printf("*EXEC %04hX:", reg.PC);
				hlt = exec();	// 1命令実行
				break;

			}
			else if (strcmp("cont\n", bp) == 0) {
				signal(SIGINT, stop);	/* 割込みハンドラの登録 */
				cont = 1;
				printf("\n");
				do {
					printf("*EXEC %04hX:", reg.PC);

					// テンポラリブレークポイントかチェック
					for (i = 0; i < MAXBP; i++) {
						if (BRKP[i].enable) {
							if (BRKP[i].addr == reg.PC) {
								cdr();
								printf("\r !TEMPORARY BREAKPOINT! \n");
								reg_disp();
								BRKP[i].enable = 0;
								signal(SIGINT, SIG_DFL);	/* 割込みハンドラをデフォルトに戻す */
								goto NEXTP;
							}
						}
					}


					hlt = exec();	// 1命令実行
					if (hlt) {
						cont = 0;
						break;
					}
				}
				while (cont != 0);
				signal(SIGINT, SIG_DFL);	/* 割込みハンドラをデフォルトに戻す */
			}
			else if (strcmp("exit\n", bp) == 0) {
				return 0;
			}
			else if (strncmp("brk", bp, 3) == 0) {	/* ブレークポイント */
				bp += 3;
				tmp0 = strtol2(bp, &endptr);
				set_brkp(tmp0);
			}
			else if (strncmp("dump", bp, 4) == 0) {
				bp += 4;
				// dump 4000h 10 のようにすると4000h番地から10バイト表示
				tmp0 = strtol2(bp, &endptr);
				endptr++;
				tmp1 = strtol2(endptr, &endptr);
				mem_dump(tmp0, tmp1);
			}
			else if (strncmp("adump", bp, 5) == 0) {
				bp += 5;
				// adump 4000h 10 のようにすると4000h番地から10バイト表示
				tmp0 = strtol2(bp, &endptr);
				endptr++;
				tmp1 = strtol2(endptr, &endptr);
				mem_adump(tmp0, tmp1);
			}
			else if (strcmp("reg\n", bp) == 0) {
				reg_disp();
			}
			else if (strcmp("reset\n", bp) == 0) {
				reg_reset();
			}
			if (strcmp("resume\n", bp) == 0) {
				hlt = 0;
			}
			else if (strncmp("PC=", bp, 3) == 0) {
				reg.PC = strtol2(bp + 3, &endptr);
			}
			else if (strncmp("A=", bp, 2) == 0) {
				reg.A = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("BC=", bp, 3) == 0) {
				reg.BC = strtol2(bp + 3, &endptr);
			}
			else if (strncmp("DE=", bp, 3) == 0) {
				reg.DE = strtol2(bp + 3, &endptr);
			}
			else if (strncmp("HL=", bp, 3) == 0) {
				reg.HL = strtol2(bp + 3, &endptr);
			}
			else if (strncmp("SP=", bp, 3) == 0) {
				reg.SP = strtol2(bp + 3, &endptr);
			}
			else if (strncmp("F=", bp, 2) == 0) {
				reg.F.byte = strtol2(bp + 2, &endptr) & 0xD5;
			}
			else if (strncmp("s=", bp, 2) == 0) {
				reg.F.bit.s = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("z=", bp, 2) == 0) {
				reg.F.bit.z = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("a=", bp, 2) == 0) {
				reg.F.bit.a = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("p=", bp, 2) == 0) {
				reg.F.bit.p = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("c=", bp, 2) == 0) {
				reg.F.bit.c = strtol2(bp + 2, &endptr);
			}
			else if (strncmp("B=", bp, 2) == 0) {
				reg.BC = (strtol2(bp + 2, &endptr) << 8) | (reg.BC & 0xFF);
			}
			else if (strncmp("C=", bp, 2) == 0) {
				reg.BC = (strtol2(bp + 2, &endptr) & 0xFF) | (reg.BC & 0xFF00);
			}
			else if (strncmp("D=", bp, 2) == 0) {
				reg.DE = (strtol2(bp + 2, &endptr) << 8) | (reg.DE & 0xFF);
			}
			else if (strncmp("E=", bp, 2) == 0) {
				reg.DE = (strtol2(bp + 2, &endptr) & 0xFF) | (reg.DE & 0xFF00);
			}
			else if (strncmp("H=", bp, 2) == 0) {
				reg.HL = (strtol2(bp + 2, &endptr) << 8) | (reg.HL & 0xFF);
			}
			else if (strncmp("L=", bp, 2) == 0) {
				reg.HL = (strtol2(bp + 2, &endptr) & 0xFF) | (reg.HL & 0xFF00);
			}
			else if (strncmp("load", bp, 4) == 0) {
				bp += 4;
				// load 4000 12,34,56,78 のようにしてロードする
				tmp0 = strtol2(bp, &endptr);
				if (*endptr != '\0') {
					while (*endptr != '\n') {
						endptr++;
						mem[tmp0++] = strtol2(endptr, &endptr);
					}
				}
			}
			else if (strncmp("hexload", bp, 7) == 0) {

				char minibuff[3];
				minibuff[2] = '\0';
				bp += 7;
				printf
				  ("  NOTE: input the HEX-sequence without a \"0x\" prefix or \"h\" suffix.\n");
				tmp0 = strtol2(bp, &endptr);
				printf("   [%04X]=", tmp0);
				while (fgets(buff, BSIZE, stdin) != NULL) {
					if (strcmp(buff, "\n") == 0)
						break;
					buff[strlen(buff) - 1] = '\0';
					bp = buff;
					while (*bp != '\0') {
						minibuff[0] = *bp;
						bp++;
						minibuff[1] = *bp;
						bp++;
						mem[tmp0++] = strtol(minibuff, &endptr, 16);
					}
					printf("   [%04X]=", tmp0);
				}


			}
			else if (strncmp("fill", bp, 4) == 0) {
				bp += 4;
				tmp0 = strtol2(bp, &endptr);
				if (*endptr != '\n'){
					endptr++;
					tmp1 = strtol2(endptr, &endptr);
					if (*endptr != '\n'){
						endptr++;
						tmp2 = strtol2(endptr, &endptr);
						for (i = tmp0; i < tmp0 + tmp1; i++)
							mem[i] = tmp2;
					}
				}
			}
			else if (strncmp("[", bp, 1) == 0) {
				bp++;
				tmp0 = strtol2(bp, &endptr);
				if (strncmp("]=", endptr, 2) == 0) {
					tmp1 = strtol2(endptr + 2, &endptr);
					mem[tmp0] = tmp1;
				}
				else if (strncmp("]\n", endptr, 2) == 0) {
					printf("   %02hhX\n", mem[tmp0]);
				}
			}
			else if (strcmp("cls\n", bp) == 0) {
				cls();
			}
			else if (strncmp("asm", bp, 3) == 0) {	// アセンブル
				bp[strlen(bp) - 1] = '\0';
				bp += 3;
				while (isblank(*bp)) {
					bp++;
				}
				FILE *fp = fopen(bp, "rb");
				if (fp == NULL) {
					assem(stdin);
				}
				else {
					assem(fp);
					fclose(fp);
				}
			}
			else if (strncmp("disas", bp, 5) == 0) {
				bp += 5;
				// 逆アセンブル　dasm 8000 10 　...16命令逆アセンブル
				tmp0 = strtol2(bp, &endptr);
				tmp1 = strtol2(endptr + 1, &endptr);
				while (tmp1--) {
					printf("  %04hX : ", tmp0);
					tmp2 = disas(tmp0);
					if (tmp2 == 0) {
						printf("    disassemble process was aborted.\n");
						break;
					}
					tmp0 += tmp2;
				}
			}
			else if (strcmp("help\n", bp) == 0) {
				printf("   asm    ... input mnemonic and assemble\n");
				printf
				  ("             (unsupport inst. DI,EI,IN,OUT,SIM,RIM,RST n)\n");
				printf("   disas addr len       ... disassemble\n");
				printf("   load addr xx,xx,xx   ... load data seq.\n");
				printf("   hexload addr         ... load data seq.\n");
				printf("   fill addr len xx     ... fill memory with xx.\n");
				printf("   [xxxx]=xx            ... load data\n");
				printf("   [xxxx]               ... show memory content\n");
				printf("   help                 ... show this message\n");
				printf("   exit\n");
				printf("   reg                  ... show registers\n");
				printf("   PC=xxxx              ... set PC\n");
				printf("   A=xx                 ... set A\n");
				printf("   B=xx                 ... set B\n");
				printf("   BC=xxxx              ... set BC\n");
				printf("   F=xx                 ... set F\n");
				printf("   c=x                  ... set c\n");
				printf("   ...\n");
				printf("   reset                ... initialize registers\n");
				printf("   dump addr len        ... memory dump (hex)\n");
				printf("   adump addr len        ... memory dump (ascii)\n");
				printf("   step or ENTER        ... single step execution\n");
				printf("   cont                 ... continuous execution\n");
				printf
				  ("   brk xxxx             ... set temporary break point\n");
				printf("   cls                  ... clear screen\n");
			}
		  NEXTP:
			printf(" ? ");
		}
	}
	return 0;
}

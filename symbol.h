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


#ifndef SYMBOL_H
#   define SYMBOL_H
#   include<limits.h>
#   define ERR_SYMBOL INT_MIN
#   define MAX_SYM   100	// シンボルテーブルの最大エントリ数

/* シンボル */
typedef struct SYMBOL
{
	int flag;					/* sym_setadrされた場合1になる */
	char name[255];				/* シンボル名（変数や関数名）を格納 */
	int addr;					/* その番地...グローバル変数ならスタック上の番地，
								   ローカル変数なら，スタックベースからのオフセット
								   関数ならプログラムメモリ上の番地 */
} SYMBOL;

 /*シンボルテーブル */
typedef struct SYMBOL_TABLE
{
	struct SYMBOL symbol[MAX_SYM];
	int size;					// 現時点で使っているシンボルの個数を記憶
} SYMBOL_TABLE;

/* シンボルテーブルに存在しないシンボルであれば，
   シンボルテーブルに追加して、
   その番地を返す。

   もしシンボルがシンボルテーブルに登録されていれば，エラーを返す．
 */
int sym_entry(struct SYMBOL_TABLE *sym_tbl, char *sname);

// シンボルのアドレスを強制的にセットし直す。
int sym_setadr(struct SYMBOL_TABLE *sym_tbl, char *sname, int newadr);

// シンボルのアドレスを取得する。シンボルがなければ，エラーを返す．
int sym_getadr(struct SYMBOL_TABLE *sym_tbl, char *sname);

// シンボルテーブル初期化（必ず最初に呼び出すこと）
void init_sym_tbl(struct SYMBOL_TABLE *sym_tbl);

// シンボルテーブルを一覧表示する
void disp_sym_tbl(struct SYMBOL_TABLE *sym_tbl);

// シンボルはあるが，アドレスが確定してない:0,アドレスが確定した:1，シンボルがないか:ERR_SYMBOL 調べる
int sym_getflag(struct SYMBOL_TABLE *sym_tbl, char *sname);
#endif

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

#ifndef COMMON_H
#   define COMMON_H
#   define VER "0.15 (20090402)"
#   define cls()  printf("\033[2J")	// clear screen
#   define cup(x)  printf("\033[%dA",x)	//cursor move (to upper)
#   define cdr()  printf("\033[K");	// カーソルより右を削除
typedef unsigned char BYTE;
typedef unsigned short WORD;
#   define BSIZE 255
#   define MEMSIZE 0x10000	// 8085のメモリサイズ 0000-FFFF
#   define N_INST 246	// 8085の命令総数
#endif

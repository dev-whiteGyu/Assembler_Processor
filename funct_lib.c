#include "funct_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
unsigned int PC = 0x400000;
unsigned int Reg[32] = { 0, };
int HI, LO = 0; // High Register, Low Register = 0;
// ----------------------------------MEMORY----------------------------------
int MEM(unsigned int A, unsigned int V, int nRW, int S)
{
	static char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];
	unsigned int sel, offset;
	char* pM;
	sel = A >> 20; offset = A & 0xFFFFF;
	if (sel == 0x004) pM = progMEM;      // program memory
	else if (sel == 0x100) pM = dataMEM; // data memory
	else if (sel == 0x7FF) pM = stakMEM; // stack
	else {
		printf("No memory\n");
		exit(1);
	}
	int bytenum = 0;
	if (S == 0) { //	 byte
		bytenum = 1;
		pM = pM + (sizeof(char)) * offset;
	}
	else if (S == 1) { // half word
		if (offset % 2 != 0)
		{
			printf("Not aligned\n");
			exit(1);
		}
		bytenum = 2;
		pM = pM + (sizeof(char)) * (offset + 1);
	}
	else if (S == 2) { // word
		if (offset % 4 != 0)
		{
			printf("Not aligned\n");
			exit(1);
		}
		bytenum = 4;
		pM = pM + (sizeof(char)) * (offset + 3);
	}
	else {
		printf("No appropriate S\n");
		exit(1);
	}
	if (nRW == 0) { // read
		int returnv = 0;
		for (int i = 1; i <= bytenum; i++)
		{
			returnv += ((*pM) & 0xff) << (8 * (i - 1));
			pM = pM - (sizeof(char));
		}
		return returnv;
	}
	else if (nRW == 1) { // write

		for (int i = 1; i <= bytenum; i++)
		{
			*pM = V & 0Xff;
			V = V >> 8;
			pM = pM - (sizeof(char));
		}
	}
	return 0;
}
// -------------------------------------ALU-------------------------------------
int logicOperation(int X, int Y, int C)
{
	if (C < 0 || C > 3) {
		printf("error in logic operation\n");
		return 1;
	}

	if (C == 0)
		return X & Y;

	else if (C == 1)
		return X | Y;

	else if (C == 2)
		return X ^ Y;

	else
		return ~(X | Y);

}

// V is 5bit shift amount
int shiftOperation(int V, int Y, int C) {

	int ret;
	if (C < 0 || C > 3) {
		printf("error in shift operation\n");
		return 1;
	}
	V = (V & 31); // 5��Ʈ�� �����

	if (C == 0) {

		ret = Y;

	}
	else if (C == 1) {

		ret = Y << V;

	}
	else if (C == 2) {

		ret = (unsigned)Y >> V;

	}
	else {
		if (V >= 0x80000000) { // MSB == 1 �� ��
			int i = 31;
			ret = 0;
			while (i == V) {
				ret = ret + pow(2.0, i);
				i--;
			}
			ret += Y >> V;
		}
		else if (V < 0x8000000) ret = Y >> V; // MSB == 0
		}

	return ret;

}

int addSubtract(int X, int Y, int C)
{
	int ret;
	if (C < 0 || C > 3) {
		printf("error in add/subract operation\n");
		return 1;
	}

	if (C == 0 || C == 2) { // add
		ret = X + Y;
	}
	else { // subtract
		ret = X - Y;
	}

	return ret;
}

int checkZero(int S)
{
	return (S == 0);
}

int checkSetLess(int X, int Y)
{
	return X < Y;
}

int ALU(int X, int Y, int C, int *Z)
{

	int c32, c10;

	int ret;

	c32 = (C >> 2) & 3;

	c10 = C & 3;

	if (c32 == 0) { // shift

		ret = shiftOperation(X, Y, c10);

	}
	else if (c32 == 1) { // set less

		ret = checkSetLess(X, Y);

	}
	else if (c32 == 2) { // addsubtract

		ret = addSubtract(X, Y, c10);
		*Z = checkZero(ret);

	}
	else { // logic

		ret = logicOperation(X, Y, c10);

	}

	return ret;

}
// --------------------------------LOAD PROGRAM---------------------------------
int LOAD_PROGRAM(char argv[])
{
	FILE *fi = NULL;
	int n;
	unsigned int offset;
	unsigned char temp[100000];
	//reset reg
	for(int i=0;i<32;i++){
		Reg[i]=0;
	}
	Reg[29] = 0x80000000; //stack pointer
	Reg[31] = 0x400018; // $ra
	int count = 0;
	fi = fopen(argv, "rb");
	if (fi == NULL) {
		printf("Cannot open file\n");
		return 1;
	}
	while ((n = fgetc(fi)) != EOF) {
		temp[count] = n;
		count++;
	}

	op_num = 0;
	data_num = 0;
	op_num |= (temp[0] << 24);
	op_num |= (temp[1] << 16);
	op_num |= (temp[2] << 8);
	op_num |= (temp[3]);
	data_num |= (temp[4] << 24);
	data_num |= (temp[5] << 16);
	data_num |= (temp[6] << 8);
	data_num |= (temp[7]);
	offset = 0x00400024;
	for (int i = 8; i < op_num * 4 + 8; i += 4) { //read op
		unsigned x = 0;
		x |= (temp[i] << 24);
		x |= (temp[i + 1] << 16);
		x |= (temp[i + 2] << 8);
		x |= (temp[i + 3]);
		MEM(offset, x, 1, 2);
		offset += 4;
	}
	offset = 0x10000000;
	for (int i = op_num * 4 + 8; i < (op_num * 4 + 8) + (data_num * 4); i += 4) {//read data until end of the file
		unsigned x = 0;
		x |= (temp[i] << 24);
		x |= (temp[i + 1] << 16);
		x |= (temp[i + 2] << 8);
		x |= (temp[i + 3]);
		MEM(offset, x, 1, 2);
		offset += 4;
	}
	//---------------------Qtspim������ pc=0x00400000~00x00400020������ ���ɾ�-------------s
	
	offset = 0x00400000;
	MEM(offset, 0x8fa40000, 1, 2);
	MEM(offset + 4, 0x27a50004, 1, 2);
	MEM(offset + 8, 0x24a60004, 1, 2);
	MEM(offset + 12, 0x00041080, 1, 2);
	MEM(offset + 16, 0x00c23021, 1, 2);
	MEM(offset + 20, 0x0c100009, 1, 2);
	MEM(offset + 24, 0x00000000, 1, 2);
	MEM(offset + 28, 0x3402000a, 1, 2);
	MEM(offset + 32, 0x0000000c, 1, 2);
	op_num += 9; 

	fclose(fi);
	return 0;
}

// --------------------------------getIR---------------------------------
IR getIR(unsigned int x)
{
	IR ret;
	unsigned int opc = x >> 26;
	if (opc == 0) { //R format
		unsigned int temp = x;
		ret.RI.fn = temp % (64);
		temp = temp >> 6;
		ret.RI.sh = temp % (32);
		temp = temp >> 5;
		ret.RI.rd = temp % (32);
		temp = temp >> 5;
		ret.RI.rt = temp % (32);
		temp = temp >> 5;
		ret.RI.rs = temp % (32);
		temp = temp >> 5;
		ret.RI.opcode = opc;
	}
	else {
		if (opc == 2) { // J format - j
			ret.JI.jt_add = (x % (67108864));
			ret.JI.opcode = opc;
		}
		else if (opc == 3) { // J format - jal
			ret.JI.jt_add = (x % (67108864));
			ret.JI.opcode = opc;
		}
		else {// I format
			unsigned int temp = x;
			ret.II.opof = temp % (65536);
			temp = temp >> 16;
			ret.II.rs = temp % (32);
			temp = temp >> 5;
			ret.II.rt = temp % (32);
			temp = temp >> 5;
			ret.II.opcode = opc;
		}
	}
	return ret;
}
int getOP(unsigned int x)
{
	unsigned int opc = x >> 26;
	return opc;
}

// --------------------------------step-----------------------------------------
void step() {
	unsigned int instReg = MEM(PC, 0, 0, 2);
	unsigned int opc = getOP(instReg);
	int z = 0; // ALU �Ű����� z
	IR ret;
	ret = getIR(instReg);
	PC += 4; //instructino fetch
	if (opc == 0) { //R format
		if (ret.RI.fn == 32) { //add
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 8, &z);
		}
		else if (ret.RI.fn == 33) { //addu
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 8, &z);
		}
		else if (ret.RI.fn == 34) { //sub
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 9, &z);
		}
		else if (ret.RI.fn == 24) { // mul
			Reg[ret.RI.rd] = Reg[ret.RI.rs] * Reg[ret.RI.rt];
		}
		else if (ret.RI.fn == 36) {	// and 
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 12, &z);
		}
		else if (ret.RI.fn == 37) {	// or
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 13, &z);
		}
		else if (ret.RI.fn == 38) {	// xor 
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 14, &z);
		}
		else if (ret.RI.fn == 39) {	// nor 
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 15, &z);
		}
		else if (ret.RI.fn == 42) {	// slt
			Reg[ret.RI.rd] = ALU(Reg[ret.RI.rs], Reg[ret.RI.rt], 4, &z);
		}
		else if (ret.RI.fn == 8) { //jr
			PC = Reg[ret.RI.rs];
			MEM(PC, 0, 0, 2);
		}
		else if (ret.RI.fn == 16) { //mfhi
			ret.RI.rd = Reg[33];
		}
		else if (ret.RI.fn == 18) { //mflo
			ret.RI.rd = Reg[34];
		}
		else if (ret.RI.fn == 12) { //syscall
			printf("Program exit\n");
		}
		else if (ret.RI.fn == 0) { // sll
			Reg[ret.RI.rd] = ALU(ret.RI.sh, Reg[ret.RI.rt], 1, &z);
		}
		else if (ret.RI.fn == 2) { // srl
			Reg[ret.RI.rd] = ALU(ret.RI.sh, Reg[ret.RI.rt], 2, &z);
		}
		else if (ret.RI.fn == 3) { // sra
			Reg[ret.RI.rd] = ALU(ret.RI.sh, Reg[ret.RI.rt], 3, &z);
		}
		else {
			printf("Undefined Instruction\n");
		}
 	}
	else if (opc == 2) { // J format - j
		PC = ret.JI.jt_add << 2;
		MEM(PC, 0, 0, 2);
	}

	else if (opc == 3) { // J format - jal
		Reg[31] = PC;
		PC = ret.JI.jt_add << 2;
		MEM(PC, 0, 0, 2);
	}

	else { // I format
		if (opc == 1) { // bltz
			if (Reg[ret.II.rs] < 0) {
				PC += (ret.II.opof*4 - 4);
				MEM(PC, 0, 0, 2);
			}
		}
		else if (opc == 4) { // beq
			if (Reg[ret.II.rs] == Reg[ret.II.rt]) {
				PC += (ret.II.opof*4 - 4);
				MEM(PC, 0, 0, 2);
			}
		}
		else if (opc == 5) { // bne
			if (Reg[ret.II.rs] != Reg[ret.II.rt]) {
				PC += (ret.II.opof*4 - 4);
				MEM(PC, 0, 0, 2);
			}
		} 
		else if (opc == 8) { // addi
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 8, &z);
		}
		else if (opc == 9) { // addiu
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 8, &z);
		}
		else if (opc == 10) { // slti
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 4, &z);
		}
		else if (opc == 12) { // andi
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 12, &z);
		}
		else if (opc == 13) { // ori
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 13, &z);
		}
		else if (opc == 14) { // xori
			Reg[ret.RI.rs] = ALU(Reg[ret.RI.rt], ret.II.opof, 14, &z);
		}
		else if (opc == 15) { // lui
			Reg[ret.II.rs] += ret.II.opof * 65536;
		}
		else if (opc == 32) { //lb
			Reg[ret.II.rs] = MEM(Reg[ret.II.rt] + ret.II.opof, 0, 0, 0);
		}
		else if (opc == 35) { // lw
			Reg[ret.II.rs] = MEM(Reg[ret.II.rt] + ret.II.opof, 0, 0, 2);
		}
		else if (opc == 36) { // lbu
			Reg[ret.II.rs] = MEM(Reg[ret.II.rt] + ret.II.opof, 0, 0, 0);
		}
		else if (opc == 40) { // sb
			MEM(Reg[ret.II.rt] + ret.II.opof, Reg[ret.II.rs], 1, 0);
		}
		else if (opc == 43) { // sw
			MEM(Reg[ret.II.rt] + ret.II.opof,Reg[ret.II.rs], 1, 2);
		}
		else {
			printf("Undefined Instruction\n");
		}
	}
}

// --------------------------------show Register--------------------------------
void showRegister(void) {
	char *reg[32] = { "r0","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra" };
	printf("\n--------------------------------[Register]--------------------------------\n");
	printf("PC = %8x\n\n",PC);
	printf("HI = %8x\n", HI);
	printf("LO = %8x\n\n", LO);
	for (int i = 0; i < 32; i++) {
		printf("R%d [%s] = %8x\n", i, reg[i], Reg[i]);
	}
	printf("----------------------------------------------------------------------------\n");
}
// --------------------------------show Memory--------------------------------
void showMemory(void) {
	printf("\n------------------------------number of data: %d-----------------------------\n", data_num);
	for (int i = 0; i < data_num; i++) {
		// i+1 ��° �����ʹ� MEM(0x00400000+(i*4)�� ������.
		printf("(%8x) %8x\n", 0x10000000 + (i * 4), MEM(0x10000000 + (i * 4), 0, 0, 2));
	}
	printf("----------------------------------------------------------------------------\n");
}
// --------------------------------Set Register--------------------------------
int REG(unsigned int A, unsigned int V, unsigned int nRW) { 
	if (nRW == 0) { //read
		return Reg[A];
	}
	else if (nRW == 1) { //write
		Reg[A] = V;
		return 0;
	}
}//----------------------------------Set PC------------------------------------------
void setPC(unsigned int val) {
	PC = val;
}
int getPC() {
	return PC;
}
//------------------------------------go---------------------------------------------
void go(unsigned int sPoint, unsigned int bPoint) { //spoint = start Point, bPoint = break Point
	if (PC == 0 || PC == bPoint)
		step();
	while (PC != 0 && PC != bPoint)
		step();
}


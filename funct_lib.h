#ifndef FUNCT_LIB_DOT_H
#define FUNCT_LIB_DOT_H
typedef union instructionRegister {
	struct RFormat {	// instruction R format
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int sh : 5;
		unsigned int fn : 6;
	} RI;
	struct IFormat {	// instruction I format
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int opof : 16;
	} II;
	struct JFormat {	// instruction J format
		unsigned int opcode : 6;
		unsigned int jt_add : 26;
	} JI;
} IR;       // IR: instruction register

unsigned int op_num; //명령어 개수
unsigned int data_num; //데이터 개수

//functions
int MEM(unsigned int A, unsigned int V, int nRW, int S);
int ALU(int X, int Y, int C, int *Z);
int LOAD_PROGRAM(char argv[]);
IR getIR(unsigned int x);
int getOP(unsigned int x);
void step(void);
void showRegister(void);
void showMemory(void);
int REG(unsigned int A, unsigned int V, unsigned int nRW);
void setPC(unsigned int val);
void go(unsigned int sPoint, unsigned int bPoint); //sPoint = start Point, bPoint = break Point
int getPC();

#endif /* FUNCT_LIB_DOT_H */
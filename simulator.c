#include "funct_lib.h"
#include <stdio.h>
#include <string.h>

void test(void)
{
	char str[1000];
	scanf("%s", str);
	//str: load�� ���� �̸�

	LOAD_PROGRAM(str);
	printf("\n---------------------------number of operator: %d---------------------------\n", op_num);
	for (int i = 0; i < op_num; i++) {
		// i+1 ��° ���ɾ�� MEM(0x00400024+(i*4)�� ������.
		unsigned int operator = MEM(0x00400000 + (i * 4), 0, 0, 2);
		printf("(%8x) %8x\n", 0x00400000 + (i * 4), operator);
		IR x;
		x = getIR(operator);
		unsigned int op = getOP(operator);
		if (op == 0) // R format
		{
			printf("op:%8x rs:%8x rt:%8x rd:%8x sh:%8x fn:%8x\n", x.RI.opcode, x.RI.rs, x.RI.rt, x.RI.rd, x.RI.sh, x.RI.fn);
		}
		else {
			if (op != 2) // I format
			{
				//opof means operand/offset
				printf("op:%8x rs:%8x rt:%8x operand/offset:%8x\n", x.II.opcode, x.II.rs, x.II.rt, x.II.opof);
			}
			else // J format
			{
				printf("op:%8x jump target address:%8x\n", x.JI.opcode, x.JI.jt_add);
			}
		}
	}
	printf("\n-----------------------------number of data: %d------------------------------\n", data_num);
	for (int i = 0; i < data_num; i++) {
		// i+1 ��° �����ʹ� MEM(0x00400024+(i*4)�� ������.
		printf("(%8x) %8x\n", 0x10000000 + (i * 4), MEM(0x10000000 + (i * 4), 0, 0, 2));
	}
	printf("----------------------------------------------------------------------------\n");
}


int main() {
	unsigned int breakPoint = 0;
	printf("Input command : ");

	while (1) {
		char input[10];
		scanf("%s", input);
		if (strcmp(input,"l")==0) { //Program Initialization
			printf("Input file name: ");
			test();
			printf("\nSet break point: ");
			scanf("%x", &breakPoint);
			setPC(breakPoint);
		}
		else if (strcmp(input,"j")==0) { // jump program
			unsigned int pc;
			printf("Input jump point: ");
			scanf("%x", &pc);
			setPC(pc);
		}
		else if (strcmp(input,"g")==0) { //go program
			if (getPC() >= breakPoint) {
				go(getPC, (op_num - 1) * 4 + 0x00400000); 
			}
			else {
				go(getPC(), breakPoint);
			}
			setPC(breakPoint); //breakPoint�� ���ƿ� ����
		}
		else if (strcmp(input,"s")==0) step();
		else if (strcmp(input,"r")==0) showRegister();
		else if (strcmp(input,"m")==0) showMemory();
		else if (strcmp(input,"sr")==0) { //set register
			int address, value;
			printf("Input register number: ");
			scanf("%d", &address);
			printf("Input value: ");
			scanf("%x", &value);
			REG(address, value, 1);
		}
		else if (strcmp(input,"sm")==0) { //set memory
			int address, value;
			printf("Input memory location: ");
			scanf("%x", &address);
			printf("Input value: ");
			scanf("%x", &value);
			MEM(address, value, 1, 2);
		}
		else if (strcmp(input,"x")==0) break;
		else if (strcmp(input,"man")==0) { //manual
			printf("---------------------------~ Manual of this program ~---------------------------\n");
			printf("l [filename] [break point] : LOAD FILE and start from the break point\n");
			printf("j [jump point] : JUMP PROGRAM\n");
			printf("g : GO PROGRAM\n");
			printf("s : STEP\n");
			printf("r : VIEW REGISTER\n");
			printf("m : VIEW MEMORY\n");
			printf("x : PROGRAM EXIT\n");
			printf("sr [register number] [value] : SET REGISTER\n");
			printf("sm [location] [value] : SET MEMORY \n");
			printf("man : MANUAL\n");
			printf("--------------------------------------------------------------------------------\n");
		}
		else {
			continue;
		}
		printf("Input command : ");

	}
	return 0;
}
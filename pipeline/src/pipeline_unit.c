#include "../header/CPU/cpu.h"
#include "../header/bit_functions/bit_functions.h"
#include "../header/logic_gates/logic_gates.h"
#include "../header/memory/memory.h"

#include<stdio.h>

int main(void){
	reg[29] = 0x01000000;
	reg[31] = 0xffffffff;
	
	FILE* file = fopen("HW3/input3/input.bin", "r");
	CPU cpu;
	int i = 0;
	while(fgets(mem+i, 5, file) != NULL){
		i+=4;
	}
	
	printf("data hazard\n");
	printf("detect and wait: 0, forwarding: 1\t: ");
	scanf("%d", &use_forward);
	if ((use_forward != 0) && (use_forward != 1)){
		return 0;
	}
	printf("control hazard\n");
	printf("detect and wait: 0, static branch prediction(ANT): 1\t: ");
	scanf("%d", &use_ant);
	if ((use_ant != 0) && (use_ant != 1)){
		return 0;
	}
	
	init_latch(&cpu);
	
	while (pc != 0xffffffff){
		if (cycle == 10000){
			break;
		}
		fetch(&cpu, mem);
		decode(&cpu);
		execute(&cpu);
		memory_operation(&cpu, mem);
		write_back(&cpu, mem);
		set_latch(&cpu);
		printf("\n\n");
	}
	print_result();
	fclose(file);
	return 0;
}
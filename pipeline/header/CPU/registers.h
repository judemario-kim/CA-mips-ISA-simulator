#ifndef _REGISTERS_H_
#define _REGISTERS_H_

typedef struct register_file{
	//input
	int read_reg1;
	int read_reg2;
	int write_reg;
	int write_data;
	//control_signal
	char reg_write;
	//output
	int read_data1;
	int read_data2;
}Reg_file;

int pc;
int reg[32];

typedef struct ifid{
	int instruction;
	int pc;
	int valid;
}IFID;

typedef struct idex{
	int cu_signal;
	int acu_signal;
	int rs;
	int rt;
	int rd;
	int shamt;
	int imm;
	int pc;
	int address;
	int read_data1;
	int read_data2;
	int valid;
}IDEX;

typedef struct exmem{
	int cu_signal;
	int acu_signal;
	int rt;
	int d_mem_write_data;
	int r_file_write_reg;
	int alu_result;
	int pc;
	int valid;
}EXMEM;

typedef struct memwb{
	int cu_signal;
	int acu_signal;
	int alu_result;
	int mem_result;
	int r_file_write_reg;
	int pc;
	int valid;
}MEMWB;

void set_register_file(Reg_file* r_file, int read_reg1, int read_reg2);
void set_register_file_write(Reg_file* r_file, int write_reg, char reg_write, int write_data);
void store_register_file_write_data(Reg_file r_file);
int get_register_file_read_data1(Reg_file r_file);
int get_register_file_read_data2(Reg_file r_file);

#endif
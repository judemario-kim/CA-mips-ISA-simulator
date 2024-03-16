#ifndef _CPU_H_
#define _CPU_H_

#include "../logic_gates/logic_gates.h"

#include "alu.h"
#include "alu_cu.h"
#include "control_unit.h"
#include "cpu.h"
#include "registers.h"
#include "data_memory.h"
#include "forwarding.h"
#include "hazard_detection.h"
#include "instruction_memory.h"

int use_forward;
int use_ant;
int cycle;
int R_type_num;
int I_type_num;
int J_type_num;
int Branch_num;
int Mem_access_num;

// ((~_type*)&inst)->~
typedef struct rtype_instruction{
	unsigned int funct: 6;
	unsigned int shamt: 5;
	unsigned int rd: 5;
	unsigned int rt: 5;
	unsigned int rs: 5;
	unsigned int opcode: 6;
} R_type;

typedef struct itype_instruction{
	unsigned int imm: 16;
	unsigned int rt: 5;
	unsigned int rs: 5;
	unsigned int opcode: 6;
} I_type;

typedef struct jtype_instruction{
	unsigned int address: 26;
	unsigned int opcode: 6;
} J_type;

typedef struct pc_mux_unit{
	Mux pc_next_br;
	Mux pc_next_j;
	Mux pc_next_jr;
}Pc_mux_unit;

typedef struct wd_mux_unit{
	Mux pc_alumem_mux;
}Wd_mux_unit;

typedef struct cpu{
	//units
	ALU alu;
	Alu_control_unit acu;
	Control_unit c_unit;
	Data_mem d_mem;
	Inst_mem i_mem;
	Reg_file r_file;
	
	HD_unit hd_unit;
	FW_unit fw_unit;
	//
	
	IFID ifid_in;
	IFID ifid_out;
	IDEX idex_in;
	IDEX idex_out;
	EXMEM exmem_in;
	EXMEM exmem_out;
	MEMWB memwb_in;
	MEMWB memwb_out;
	
	//logic_gates
	Mux pc_04_mux;
	Adder pc_adder;
	Adder jal_adder;
	Extension_unit sign_extension_unit;
	Extension_unit zero_extension_unit;
	Shifter lui_shifter;
	Shifter branch_address_shifter;
	Shifter jump_address_shifter;
	Adder branch_adder;
	Mux write_reg_mux;
	Mux alu_src_mux;
	Mux alu_mem_mux;
	Extension_unit lh_extension_unit;
	Extension_unit lb_extension_unit;
	
	Pc_mux_unit pc_mux_unit;
	Wd_mux_unit wd_mux_unit;
	
	Mux forward_a_mux;
	Mux forward_b_mux;
	
}CPU;

void fetch(CPU* cpu, char* mem);
void decode(CPU* cpu);
void execute(CPU* cpu);
void memory_operation(CPU* cpu, char* mem);
void write_back(CPU* cpu, char* mem);
void pc_update(CPU* cpu);
void print_result();

void init_latch(CPU* cpu);
void set_latch(CPU* cpu);
void set_ifid(CPU* cpu);
void set_idex(CPU* cpu);
void set_exmem(CPU* cpu);
void set_memwb(CPU* cpu);

#endif
#include "../../header/CPU/cpu.h"

#include<stdio.h>
#include<string.h>

int use_forward = 0;
int use_ant = 0;

int cycle = 0;
int R_type_num = 0;
int I_type_num = 0;
int J_type_num = 0;
int Branch_num = 0;
int Mem_access_num = 0;

void fetch(CPU* cpu, char* mem){
	printf("[%d//%d] [%d//%d] [%d//%d] [%d//%d]\n", cpu->ifid_in.valid, cpu->ifid_out.valid, cpu->idex_in.valid, cpu->idex_out.valid, cpu->exmem_in.valid, cpu->exmem_out.valid, cpu->memwb_in.valid, cpu->memwb_out.valid);
	cycle++;
	set_instruction_memory(&(cpu->i_mem), pc, mem);
	
	printf("cycle[%d] (pc: 0x%x)\n", cycle, pc);
	printf("[Fetch instruction] ");

	//
	set_adder(	&(cpu->pc_adder),
			 	pc,
			 	4,
			 	0);
	pc = get_adder_result(cpu->pc_adder);
	//
	if (cpu->ifid_in.valid == 1){
		cpu->ifid_in.instruction = get_instruction_memory_data(cpu->i_mem);
		cpu->ifid_in.pc = get_adder_result(cpu->pc_adder);	
	}
	else{
		printf("[fetch fail]\n");
		cpu->ifid_in.valid = -1;
	}

	printf("0x%08x\n\n", cpu->ifid_in.instruction);
	
	return;
}

void decode(CPU* cpu){
	if (cpu->ifid_out.valid == 0){
		printf("================\n= decode stall =\n================\n");
		return;
	}
	int instruction = cpu->ifid_out.instruction;
	set_control_unit(	&(cpu->c_unit),
						((R_type*)&instruction)->opcode);
	Control_unit_signal c_unit_signal = get_control_unit_signal(cpu->c_unit);
	
	Acu_input acu_input;
	acu_input.alu_op = c_unit_signal.alu_op;
	acu_input.funct = ((R_type*)&instruction)->funct;
	set_alu_control_unit(	&(cpu->acu),
							acu_input);

	set_register_file(	&(cpu->r_file), 
					 	((R_type*)&instruction)->rs,
					  	((R_type*)&instruction)->rt);

	set_hd_unit_ch(&(cpu->hd_unit), ((R_type*)&instruction)->opcode);
	if (use_ant == 0 && cpu->hd_unit.ch_signal.wait){
		printf("++++++++++++++++++\n+ control hazard +\n++++++++++++++++++\n\n");
		pc -= 8;
		cpu->ifid_in.valid = 0;
	}
	
	if (use_forward == 1){
		if (((*(Control_unit_signal*)(&cpu->idex_out.cu_signal)).mem_to_reg != 0) && 
		   ((cpu->idex_out.rt == ((R_type*)&instruction)->rs) || (cpu->idex_out.rt == ((R_type*)&instruction)->rt))){
			printf("load-use hazard detected\n");
			printf("stall one cycle\n");
			pc -= 4;
			cpu->ifid_in.valid = -1;
		}
	}
	
	Acu_output acu_output = get_alu_control_unit_output(cpu->acu);
	
	
	//
	cpu->idex_in.cu_signal = (*(int*)(&c_unit_signal));
	cpu->idex_in.acu_signal = (*(int*)(&acu_output));
	cpu->idex_in.rs = ((R_type*)&instruction)->rs;
	cpu->idex_in.rt = ((R_type*)&instruction)->rt;
	cpu->idex_in.rd = ((R_type*)&instruction)->rd;
	cpu->idex_in.shamt = ((R_type*)&instruction)->shamt;
	cpu->idex_in.imm = ((I_type*)&instruction)->imm;
	cpu->idex_in.pc = cpu->ifid_out.pc;
	cpu->idex_in.address = ((J_type*)&instruction)->address;
	cpu->idex_in.read_data1 = get_register_file_read_data1(cpu->r_file);
	cpu->idex_in.read_data2 = get_register_file_read_data2(cpu->r_file);
	cpu->idex_in.valid = cpu->ifid_out.valid;
	//
	
	printf("decode instruction: 0x%08x\n", instruction);
	printf("[Decode instruction] type: ");
	if (instruction == 0){
		printf("Nop\n");
	}
	else if (((R_type*)&instruction)->opcode == R){
		printf("R\n");
		printf("\topcode: 0x%x, ", ((R_type*)&instruction)->opcode);
		printf("rs: 0x%x (R[%d]=0x%x), ", ((R_type*)&instruction)->rs, ((R_type*)&instruction)->rs, reg[((R_type*)&instruction)->rs]);
		printf("rt: 0x%x (R[%d]=0x%x), ", ((R_type*)&instruction)->rt, ((R_type*)&instruction)->rt, reg[((R_type*)&instruction)->rt]);
		printf("rd: 0x%x (%d), ", ((R_type*)&instruction)->rd, ((R_type*)&instruction)->rd);
		printf("shamt: 0x%x, ", ((R_type*)&instruction)->shamt);
		printf("funct: 0x%x\n", ((R_type*)&instruction)->funct);
		R_type_num++;
	}
	else if(((J_type*)&instruction)->opcode == J || ((J_type*)&instruction)->opcode == JAL){
		printf("J\n");
		printf("\topcode: 0x%x, ", ((J_type*)&instruction)->opcode);
		printf("address: 0x%x\n", ((J_type*)&instruction)->address);
		J_type_num++;
	}
	else{
		printf("I\n");
		printf("\topcode: 0x%x, ", ((I_type*)&instruction)->opcode);
		printf("rs: 0x%x (R[%d]=0x%x), ", ((I_type*)&instruction)->rs, ((I_type*)&instruction)->rs, reg[((I_type*)&instruction)->rs]);
		printf("rt: 0x%x (R[%d]=0x%x), ", ((I_type*)&instruction)->rt, ((I_type*)&instruction)->rt, reg[((I_type*)&instruction)->rt]);
		printf("imm: 0x%x\n", ((I_type*)&instruction)->imm);
		I_type_num++;
	}
	printf("\n");
	return;
}

void execute(CPU* cpu){
	if (cpu->idex_out.valid == 0){
		printf("=================\n= execute stall =\n=================\n");
		return;
	}
	Control_unit_signal c_unit_signal = (*(Control_unit_signal*)(&cpu->idex_out.cu_signal));
	Acu_output acu_output = (*(Acu_output*)(&cpu->idex_out.acu_signal));
	
	///
	int data_hazard = 0;
	printf("check data hazard\n");
	printf("rs: %d, rt: %d, exmem_rd: %d, memwb_rd: %d\n", cpu->idex_out.rs, cpu->idex_out.rt, cpu->exmem_out.r_file_write_reg, cpu->memwb_out.r_file_write_reg);
	set_hd_unit_dh(&(cpu->hd_unit), 
				   cpu->idex_out.rs, 
				   cpu->idex_out.rt, 
				   cpu->exmem_out.r_file_write_reg, 
				   cpu->memwb_out.r_file_write_reg);
	set_fw_unit(&(cpu->fw_unit), 
				(*(FW_input*)(&cpu->hd_unit.dh_signal)), 
				(*(Control_unit_signal*)(&cpu->exmem_out.cu_signal)).reg_write, 
				(*(Control_unit_signal*)(&cpu->memwb_out.cu_signal)).reg_write);
	if ((use_forward == 1) && ((cpu->fw_unit.fw_signal.forwardA != 0b00) || (cpu->fw_unit.fw_signal.forwardB != 0b00))){
		printf("++++++++++++++\n+ forwarding +\n++++++++++++++\n\n");
	}
	else if (((cpu->hd_unit.dh_signal.Memhazard_rs || cpu->hd_unit.dh_signal.Memhazard_rt) && (*(Control_unit_signal*)(&cpu->exmem_out.cu_signal)).reg_write) || 
		((cpu->hd_unit.dh_signal.Wbhazard_rs || cpu->hd_unit.dh_signal.Wbhazard_rt) && (*(Control_unit_signal*)(&cpu->memwb_out.cu_signal)).reg_write)){
		printf("+++++++++++++++\n+ data hazard +\n+++++++++++++++\n\n");
		data_hazard = 1;
		if (cpu->hd_unit.ch_signal.wait && use_ant==0){
			pc -= 4;
		}
		else{
			pc -= 12;
		}
		cpu->ifid_in.valid = -1;
		cpu->ifid_out.valid = 0;
		cpu->idex_in.valid = 0;
		cpu->idex_out.valid = 0;
		cpu->exmem_in.valid = 0;
		cpu->exmem_out.valid = 1;
	}
	///
	
	int write_reg_mux_arr[3] = {cpu->idex_out.rt,
							   	cpu->idex_out.rd,
							   	31};
	set_mux(&(cpu->write_reg_mux),
		   	write_reg_mux_arr,
			c_unit_signal.reg_dst);
	
	set_extension_unit(	&(cpu->sign_extension_unit),
					  	cpu->idex_out.imm,
					  	1);
	
	set_extension_unit(	&(cpu->zero_extension_unit),
					  	cpu->idex_out.imm,
					  	0);
	
	set_shifter(&(cpu->lui_shifter),
			   	cpu->idex_out.imm,
			   	16, 1, 0);
	
	set_shifter(&(cpu->branch_address_shifter),
			get_extension_unit_result(cpu->sign_extension_unit),
			2, 1, 0);
	
	int memwb_result = (*(Control_unit_signal*)(&cpu->memwb_out.cu_signal)).mem_to_reg ? cpu->memwb_out.mem_result : cpu->memwb_out.alu_result;
	int forward_a_mux_arr[3] = {reg[cpu->idex_out.rs],
								memwb_result, 
							   	cpu->exmem_out.alu_result};
	int forward_a_signal = use_forward ? cpu->fw_unit.fw_signal.forwardA : 0;
	set_mux(&(cpu->forward_a_mux),
		   	forward_a_mux_arr,
		   	forward_a_signal);
	
	int forward_b_mux_arr[3] = {reg[cpu->idex_out.rt],
								memwb_result,
							   	cpu->exmem_out.alu_result};
	int forward_b_signal = use_forward ? cpu->fw_unit.fw_signal.forwardB : 0;
	set_mux(&(cpu->forward_b_mux),
		   	forward_b_mux_arr,
		   	forward_b_signal);
	
	int alu_src_mux_arr[4] = {	get_mux_result(cpu->forward_b_mux),
							 	get_extension_unit_result(cpu->sign_extension_unit),
							 	get_extension_unit_result(cpu->zero_extension_unit),
							 	get_shifter_result(cpu->lui_shifter)};
	
	set_mux(&(cpu->alu_src_mux),
		   	alu_src_mux_arr,
		   	c_unit_signal.alu_src);
	
	set_alu(&(cpu->alu),
		   	cpu->idex_out.shamt,
		   	get_mux_result(cpu->forward_a_mux),
		   	get_mux_result(cpu->alu_src_mux),
		   	acu_output.alu_control);
	
	Alu_signal alu_signal = get_alu_signal(cpu->alu);
	//pc&branch operation
	set_adder(	&(cpu->branch_adder),
			 	get_shifter_result(cpu->branch_address_shifter),
			 	cpu->idex_out.pc,
			 	0);
	
	int pcsrc = (c_unit_signal.branch & alu_signal.zero) ||
				(c_unit_signal.brchne & ~alu_signal.zero) ||
				(c_unit_signal.bgtz & alu_signal.gtz) ||
				(c_unit_signal.blez & alu_signal.lez) ||
				(c_unit_signal.bltz & alu_signal.ltz);
	set_control_unit_pcsrc(	&(cpu->c_unit),
						  	pcsrc);
	
	set_shifter(&(cpu->jump_address_shifter),
			   	cpu->idex_out.address,
			   	2, 1, 0);
	
	if (c_unit_signal.jump && data_hazard==0){
		printf("jump\n");
		pc = (get_adder_result(cpu->pc_adder)&0xf0000000) | get_shifter_result(cpu->jump_address_shifter);
		printf("pc: %x\n", pc);
	}
	else if (acu_output.jr && data_hazard==0){
		printf("jr\n");
		pc = reg[cpu->idex_out.rs];
		printf("pc: %x\n", pc);
	}
	if (use_ant == 1 && pcsrc && data_hazard == 0){
		printf("======================\nant roll back\n======================\n");
		pc = get_adder_result(cpu->branch_adder);
		cpu->ifid_in.valid = -1;
		cpu->idex_in.valid = 0;
		printf("pc: %x\n", pc);
	}
	else if (use_ant == 0 && pcsrc && data_hazard == 0){
		printf("======================\nbranch\n======================\n");
		pc = get_adder_result(cpu->branch_adder);
		printf("pc: %x\n", pc);
	}
	
	//
	cpu->exmem_in.cu_signal = cpu->idex_out.cu_signal;
	cpu->exmem_in.acu_signal = cpu->idex_out.acu_signal;
	cpu->exmem_in.rt = cpu->idex_out.rt;
	cpu->exmem_in.d_mem_write_data = get_mux_result(cpu->forward_b_mux);
	cpu->exmem_in.r_file_write_reg = get_mux_result(cpu->write_reg_mux);
	cpu->exmem_in.alu_result = get_alu_result(cpu->alu);
	cpu->exmem_in.pc = cpu->idex_out.pc;
	cpu->exmem_in.valid = cpu->idex_out.valid;
	//
	
	printf("[Execute]\n");
	printf("\talu src1: 0x%x, src2: 0x%x, result: 0x%x\n", cpu->alu.input_a, cpu->alu.input_b, cpu->alu.result);
	printf("\n");
	return;
}

void memory_operation(CPU* cpu, char* mem){
	if (cpu->exmem_out.valid == 0){
		printf("================\n= mem_op stall =\n================\n");
		return;
	}
	Control_unit_signal c_unit_signal = (*(Control_unit_signal*)(&cpu->exmem_out.cu_signal));
	
	set_data_memory(&(cpu->d_mem),
				   	cpu->exmem_out.alu_result,
				   	cpu->exmem_out.d_mem_write_data,
				   	mem,
				   	c_unit_signal.sh_b,
				   	c_unit_signal.mem_write);
	
	store_data_memory(cpu->d_mem, mem);
	
	//
	cpu->memwb_in.cu_signal = cpu->exmem_out.cu_signal;
	cpu->memwb_in.acu_signal = cpu->exmem_out.acu_signal;
	cpu->memwb_in.alu_result = cpu->exmem_out.alu_result;
	cpu->memwb_in.mem_result = get_data_memory_data(cpu->d_mem);
	cpu->memwb_in.r_file_write_reg = cpu->exmem_out.r_file_write_reg;
	cpu->memwb_in.pc = cpu->exmem_out.pc;
	cpu->memwb_in.valid = cpu->exmem_out.valid;
	//
	
	if (cpu->d_mem.mem_write){
		printf("[Store] ");
		printf("Mem[0x%08x] <- R[%d] = 0x%x\n", cpu->exmem_out.alu_result, cpu->exmem_out.rt, cpu->exmem_out.d_mem_write_data);
		printf("\n");
		Mem_access_num++;
	}
	return;
}

void write_back(CPU* cpu, char* mem){
	if (cpu->memwb_out.valid == 0){
		printf("============\n= wb stall =\n============\n");
		return;
	}
	Control_unit_signal c_unit_signal = (*(Control_unit_signal*)(&cpu->memwb_out.cu_signal));
	Acu_output acu_output = (*(Acu_output*)(&cpu->memwb_out.acu_signal));
	
	int lw = cpu->memwb_out.mem_result;
	set_extension_unit(	&(cpu->lh_extension_unit),
						(lw&0xffff0000)>>16,
						c_unit_signal.s_zext == 1 ? 0 : 1);
	int lh = get_extension_unit_result(cpu->lh_extension_unit);
					   
	set_extension_unit(	&(cpu->lb_extension_unit),
						(lw&0xff000000)>>24,
						c_unit_signal.s_zext == 1 ? 0 : 1);
	int lb = get_extension_unit_result(cpu->lb_extension_unit);
					   
	int alu_mem_mux_arr[4] = {	cpu->memwb_out.alu_result,
							 	lw,
							 	lh,
							 	lb};
	set_mux(&(cpu->alu_mem_mux),
		   	alu_mem_mux_arr,
		   	c_unit_signal.mem_to_reg);
	
	set_adder(	&(cpu->jal_adder),
			 	cpu->memwb_out.pc,
			 	4,
			 	0);
	
	int pc_alumem_mux_arr[2] = {get_mux_result(cpu->alu_mem_mux),
							   	get_adder_result(cpu->jal_adder)};
	
	set_mux(&(cpu->wd_mux_unit.pc_alumem_mux),
		   	pc_alumem_mux_arr,
		   	c_unit_signal.jal | acu_output.jalr);
	set_register_file_write(&(cpu->r_file), cpu->memwb_out.r_file_write_reg, c_unit_signal.reg_write, get_mux_result(cpu->wd_mux_unit.pc_alumem_mux));
	
	store_register_file_write_data(cpu->r_file);
	if (c_unit_signal.mem_to_reg > 0){
		printf("[Load] ");
		printf("R[%d] <- Mem[0x%08x] = 0x%x\n", cpu->memwb_out.r_file_write_reg, cpu->memwb_out.alu_result, cpu->memwb_out.mem_result);
		printf("\n");
		Mem_access_num++;
	}
	else if (c_unit_signal.reg_write && cpu->memwb_out.r_file_write_reg != 0){
		printf("[Write Back] ");
		printf("R[%d] <- 0x%x\n", cpu->memwb_out.r_file_write_reg, get_mux_result(cpu->wd_mux_unit.pc_alumem_mux));
		printf("\n");
	}
	return;
}

void print_result(){
	printf("Return value (r2): %d\n", reg[2]);
	printf("Total Cycle: %d\n", cycle);
	return;
}

void init_latch(CPU* cpu){
	memset(&cpu->ifid_in, 0, sizeof(IFID));
	memset(&cpu->ifid_out, 0, sizeof(IFID));
	memset(&cpu->idex_in, 0, sizeof(IDEX));
	memset(&cpu->idex_out, 0, sizeof(IDEX));
	memset(&cpu->exmem_in, 0, sizeof(EXMEM));
	memset(&cpu->exmem_out, 0, sizeof(EXMEM));
	memset(&cpu->memwb_in, 0, sizeof(MEMWB));
	memset(&cpu->memwb_out, 0, sizeof(MEMWB));
	cpu->ifid_in.valid = 1;
	cpu->ifid_out.valid = 0;
	cpu->idex_in.valid = 0;
	cpu->idex_out.valid = 0;
	cpu->exmem_in.valid = 0;
	cpu->exmem_out.valid = 0;
	cpu->memwb_in.valid = 0;
	cpu->memwb_out.valid = 0;
	return;
}

void set_latch(CPU* cpu){
	set_ifid(cpu);
	set_idex(cpu);
	set_exmem(cpu);
	set_memwb(cpu);
	return;
}

void set_ifid(CPU* cpu){
	memcpy(&cpu->ifid_out, &cpu->ifid_in, sizeof(IFID));
	int tmp = cpu->ifid_in.valid;
	memset(&cpu->ifid_in, 0, sizeof(IFID));
	if (tmp == -1){
		cpu->ifid_in.valid = 1;
		cpu->ifid_out.valid = 0;
	}
	else{
		cpu->ifid_in.valid = tmp;	
	}
	
	return;
}

void set_idex(CPU* cpu){
	memcpy(&cpu->idex_out, &cpu->idex_in, sizeof(IDEX));
	memset(&cpu->idex_in, 0, sizeof(IDEX));
	return;
}

void set_exmem(CPU* cpu){
	memcpy(&cpu->exmem_out, &cpu->exmem_in, sizeof(EXMEM));
	memset(&cpu->exmem_in, 0, sizeof(EXMEM));
	return;
}

void set_memwb(CPU* cpu){
	memcpy(&cpu->memwb_out, &cpu->memwb_in, sizeof(MEMWB));
	memset(&cpu->memwb_in, 0, sizeof(MEMWB));
	return;
}
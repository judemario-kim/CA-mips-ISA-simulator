#ifndef _FORWARDING_H_
#define _FORWARDING_H_

typedef struct fw_input{
	unsigned char Memhazard_rs: 1;
	unsigned char Memhazard_rt: 1;
	unsigned char Wbhazard_rs: 1;
	unsigned char Wbhazard_rt: 1;
}FW_input;

typedef struct forwarding_signal{
	unsigned char forwardA: 2;
	unsigned char forwardB: 2;
}FW_signal;

typedef struct fw_unit{
	//input
	FW_input fw_input;
	int exmem_reg_write;
	int memwb_reg_write;
	//output
	FW_signal fw_signal;
} FW_unit;

void set_fw_unit(FW_unit* fw_unit, FW_input fw_input, int exmem_reg_write, int memwb_reg_write);
void set_fw_signal(FW_unit* fw_unit);

#endif
#include "../../header/CPU/forwarding.h"

void set_fw_unit(FW_unit* fw_unit, FW_input fw_input, int exmem_reg_write, int memwb_reg_write){
	fw_unit->fw_input = fw_input;
	fw_unit->exmem_reg_write = exmem_reg_write;
	fw_unit->memwb_reg_write = memwb_reg_write;
	
	set_fw_signal(fw_unit);
	return;
}

void set_fw_signal(FW_unit* fw_unit){
	fw_unit->fw_signal.forwardA = 0b00;
	fw_unit->fw_signal.forwardB = 0b00;
	if (fw_unit->exmem_reg_write && fw_unit->fw_input.Memhazard_rs){
		fw_unit->fw_signal.forwardA = 0b10;
	}
	else if (fw_unit->memwb_reg_write && fw_unit->fw_input.Wbhazard_rs){
		fw_unit->fw_signal.forwardA = 0b01;
	}
	if (fw_unit->exmem_reg_write && fw_unit->fw_input.Memhazard_rt){
		fw_unit->fw_signal.forwardB = 0b10;
	}
	else if (fw_unit->memwb_reg_write && fw_unit->fw_input.Wbhazard_rt){
		fw_unit->fw_signal.forwardB = 0b01;
	}
	return;
}
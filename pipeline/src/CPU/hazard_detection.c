#include "../../header/CPU/hazard_detection.h"

void set_hd_unit_ch(HD_unit* hd_unit, int opcode){
	hd_unit->opcode = opcode;
	set_ch_signal(hd_unit);
	return;
}

void set_ch_signal(HD_unit* hd_unit){
	if ((hd_unit->opcode >= 0x04) && (hd_unit->opcode <= 0x07)){
		hd_unit->ch_signal.wait = 1;
	}
	else{
		hd_unit->ch_signal.wait = 0;
	}
	return;
}

void set_hd_unit_dh(HD_unit* hd_unit, int idex_rs, int idex_rt, int exmem_rd, int memwb_rd){
	hd_unit->idex_rs = idex_rs;
	hd_unit->idex_rt = idex_rt;
	hd_unit->exmem_rd = exmem_rd;
	hd_unit->memwb_rd = memwb_rd;
	
	set_dh_signal(hd_unit);
	return;
}

void set_dh_signal(HD_unit* hd_unit){
	hd_unit->dh_signal.Memhazard_rs = 0;
	hd_unit->dh_signal.Memhazard_rt = 0;
	hd_unit->dh_signal.Wbhazard_rs = 0;
	hd_unit->dh_signal.Wbhazard_rt = 0;
	if ((hd_unit->exmem_rd != 0) && (hd_unit->idex_rs == hd_unit->exmem_rd)){
		hd_unit->dh_signal.Memhazard_rs = 1;
	}
	if ((hd_unit->exmem_rd != 0) && (hd_unit->idex_rt == hd_unit->exmem_rd)){
		hd_unit->dh_signal.Memhazard_rt = 1;
	}
	if ((hd_unit->memwb_rd != 0) && (hd_unit->idex_rs == hd_unit->memwb_rd)){
		hd_unit->dh_signal.Wbhazard_rs = 1;
	}
	if ((hd_unit->memwb_rd != 0) && (hd_unit->idex_rt == hd_unit->memwb_rd)){
		hd_unit->dh_signal.Wbhazard_rt = 1;
	}
	return;
}
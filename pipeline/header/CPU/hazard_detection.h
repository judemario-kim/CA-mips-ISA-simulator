#ifndef _HAZARD_DETECTION_H_
#define _HAZARD_DETECTION_H_

typedef struct control_hazard_signal{
	unsigned char wait: 1;
}CH_signal;

typedef struct data_hazard_signal{
	unsigned char Memhazard_rs: 1;
	unsigned char Memhazard_rt: 1;
	unsigned char Wbhazard_rs: 1;
	unsigned char Wbhazard_rt: 1;
} DH_signal;

typedef struct hd_unit{
	//input
	int opcode;
	int idex_rs;
	int idex_rt;
	int exmem_rd;
	int memwb_rd;
	//output
	CH_signal ch_signal;
	DH_signal dh_signal;
} HD_unit;

void set_hd_unit_ch(HD_unit* hd_unit, int opcode);
void set_ch_signal(HD_unit* hd_unit);
void set_hd_unit_dh(HD_unit* hd_unit, int idex_rs, int idex_rt, int exmem_rd, int memwb_rd);
void set_dh_signal(HD_unit* hd_unit);

#endif
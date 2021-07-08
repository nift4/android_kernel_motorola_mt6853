// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef _CONNFEM_API_H
#define _CONNFEM_API_H

#include <linux/types.h>
#include <linux/ioctl.h>

/*******************************************************************************
 *                        C O M P I L E R   F L A G S
 ******************************************************************************/

/*******************************************************************************
 *                                M A C R O S
 ******************************************************************************/
#define CONNFEM_EPAELNA_PIN_COUNT	32
#define CONNFEM_PART_NAME_SIZE	64
#define CONNFEM_FLAG_NAME_SIZE	32
#define CONNFEM_EPAELNA_LAA_PIN_COUNT	8

/*******************************************************************************
 *                            D A T A   T Y P E S
 ******************************************************************************/
enum connfem_type {
	CONNFEM_TYPE_NONE = 0,
	CONNFEM_TYPE_EPAELNA = 1,
	CONNFEM_TYPE_NUM
};

enum connfem_subsys {
	CONNFEM_SUBSYS_NONE = 0,
	CONNFEM_SUBSYS_WIFI = 1,
	CONNFEM_SUBSYS_BT = 2,
	CONNFEM_SUBSYS_NUM
};

struct connfem_ior_is_available_para {
	enum connfem_type fem_type;		//in
	enum connfem_subsys subsys;		//in
	bool is_available;				//out
};

enum connfem_rf_port {
	CONNFEM_PORT_BT = 0,
	CONNFEM_PORT_WF0 = CONNFEM_PORT_BT,
	CONNFEM_PORT_WF1 = 1,
	CONNFEM_PORT_NUM
};

struct connfem_part {
	unsigned char vid;
	unsigned char pid;
};

struct connfem_epaelna_pin {
	unsigned char antsel;
	unsigned char fem;
	bool polarity;
};

struct connfem_epaelna_fem_info {
	unsigned int id;
	struct connfem_part part[CONNFEM_PORT_NUM];
	char part_name[CONNFEM_PORT_NUM][CONNFEM_PART_NAME_SIZE];
};

struct connfem_epaelna_pin_info {
	unsigned int count;
	struct connfem_epaelna_pin pin[CONNFEM_EPAELNA_PIN_COUNT];
};

struct connfem_epaelna_laa_pin {
	unsigned char gpio;
	unsigned char md_mode;
	unsigned char wf_mode;
};

struct connfem_epaelna_laa_pin_info {
	unsigned int count;
	struct connfem_epaelna_laa_pin
		pin[CONNFEM_EPAELNA_LAA_PIN_COUNT];
};

struct connfem_epaelna_flags_wifi {
	bool open_loop;
	bool laa;
};

struct connfem_epaelna_flags_bt {
	bool bypass;
	bool epa_elna;
	bool epa;
	bool elna;
};

/*******************************************************************************
 *                           P U B L I C   D A T A
 ******************************************************************************/

/*******************************************************************************
 *                             F U N C T I O N S
 ******************************************************************************/
extern bool connfem_is_available(enum connfem_type fem_type);
extern int connfem_epaelna_get_fem_info(
			struct connfem_epaelna_fem_info *fem_info);
extern int connfem_epaelna_get_pin_info(
			struct connfem_epaelna_pin_info *pin_info);
extern int connfem_epaelna_laa_get_pin_info(
			struct connfem_epaelna_laa_pin_info *laa_pin_info);
extern int connfem_epaelna_get_flags(enum connfem_subsys subsys, void *flags);
extern int connfem_epaelna_get_flags_string(enum connfem_subsys subsys,
			unsigned int *num_flags, char ***flags);

#endif /* _CONNFEM_API_H */

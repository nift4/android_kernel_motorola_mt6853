// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*
 * The header is provided for connfem kernel module internal used.
 */

#ifndef _CONNFEM_INTERNAL_H
#define _CONNFEM_INTERNAL_H

#include <linux/platform_device.h>

#include "connfem_api.h"

/*******************************************************************************
 *                        C O M P I L E R   F L A G S
 ******************************************************************************/

/*******************************************************************************
 *                              M A C R O S
 ******************************************************************************/
#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "["KBUILD_MODNAME"]" fmt

#define CONNFEM_IOCTL_NUM 0x46
#define CONNFEM_DEVICE_FILE "/dev/connfem"
#define CONNFEM_DEV_WAIT_TIME_INTERVAL  1
#define CONNFEM_DEV_WAIT_TIME_MAX       30
#define CONNFEM_FLAG_NAME_SIZE  32

#define IO_REQUEST_CONNFEM_IS_AVAILABLE 0x0
#define IO_REQUEST_CONNFEM_GET_FLAG_NAMES_STAT 0x1
#define IO_REQUEST_CONNFEM_GET_FLAG_NAMES 0x2
#define IO_REQUEST_CONNFEM_GET_FEM_INFO 0x3

#define CONNFEM_IOR_IS_AVAILABLE _IOR(CONNFEM_IOCTL_NUM, \
					IO_REQUEST_CONNFEM_IS_AVAILABLE, \
					struct connfem_ior_is_available_para)

/* IOC_FUNC_GET_FLAG_NAMES_STAT:
 * Retrieves subsys' flag names container size information for
 * user space allocation.
 */
#define IOC_FUNC_GET_FLAG_NAMES_STAT _IOWR(CONNFEM_IOCTL_NUM, \
				IO_REQUEST_CONNFEM_GET_FLAG_NAMES_STAT, \
				struct connfem_ioctl_get_flag_names_stat)

/* IOC_FUNC_GET_FLAG_NAMES
 * Retrieves subsys' flag names, the "names" container must be allocated
 * before using this command.
 */
#define IOC_FUNC_GET_FLAG_NAMES	_IOWR(CONNFEM_IOCTL_NUM, \
					IO_REQUEST_CONNFEM_GET_FLAG_NAMES, \
					struct connfem_ioctl_get_flag_names)

#define IOC_FUNC_GET_FEM_INFO _IOWR(CONNFEM_IOCTL_NUM, \
					IO_REQUEST_CONNFEM_GET_FEM_INFO, \
					struct connfem_epaelna_fem_info)

/*******************************************************************************
 *                            D A T A   T Y P E S
 ******************************************************************************/
struct connfem_epaelna_total_info {
	struct connfem_epaelna_fem_info fem_info;
	struct connfem_epaelna_pin_info pin_info;
	struct connfem_epaelna_flags_wifi flags_wifi;
	struct connfem_epaelna_flags_bt flags_bt;
	bool connfem_available;
	struct connfem_container *wf_flag_names_cont;
	struct connfem_container *bt_flag_names_cont;
	struct connfem_epaelna_laa_pin_info laa_pin_info;
};

struct connfem_container {
	unsigned int cnt;
	unsigned int entry_sz;
	char buffer[0];
};

struct connfem_ioctl_get_flag_names_stat {
	enum connfem_subsys subsys;    /* IN */
	unsigned int cnt;           /* OUT */
	unsigned int entry_sz;      /* OUT */
};

struct connfem_ioctl_get_flag_names {
	enum connfem_subsys subsys;    /* IN */
	uint64_t names;             /* IN/OUT connfem_container_st* */
};

/*******************************************************************************
 *                F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************/

/*******************************************************************************
 *                         P U B L I C   D A T A
 ******************************************************************************/

/*******************************************************************************
 *                        P R I V A T E   D A T A
 ******************************************************************************/

/*******************************************************************************
 *                           F U N C T I O N S
 ******************************************************************************/
/* connfem_api.c */
void connfem_epaelna_init(struct platform_device *pdev);
bool connfem_is_epaelna_available(void);
int _connfem_epaelna_get_fem_info(struct connfem_epaelna_fem_info *fem_info);
int _connfem_epaelna_get_pin_info(struct connfem_epaelna_pin_info *pin_info);
int _connfem_epaelna_get_flags_wifi(struct connfem_epaelna_flags_wifi *flags);
int _connfem_epaelna_get_flags_bt(struct connfem_epaelna_flags_bt *flags);
int _connfem_epaelna_laa_get_pin_info(
			struct connfem_epaelna_laa_pin_info *laa_pin_info);

/* connfem_dt_parser.c */
void connfem_dt_parser_init(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info);

/* connfem_dt_parser_wifi.c */
void connfem_dt_parser_wifi(struct device_node *devnode,
				struct connfem_epaelna_total_info *total_info,
				int parts_selectd);

/* connfem_dt_parser_bt.c */
void connfem_dt_parser_bt(struct device_node *devnode,
				struct connfem_epaelna_total_info *total_info,
				int parts_selectd);

/* connfem_container.c */
struct connfem_container *connfem_container_alloc(unsigned int cnt,
				unsigned int entry_sz);
void *connfem_container_entry(struct connfem_container *container,
				unsigned int idx);
void **connfem_container_create_pointers_array(struct connfem_container *cont);
int connfem_dt_parser_flag_names(struct device_node *node,
				struct connfem_container **flags_out,
				char ***names_out);

/* connfem_epaelna.c */
struct connfem_container *get_container(enum connfem_subsys subsys);
int get_flag_names_stat_from_total_info(
				struct connfem_ioctl_get_flag_names_stat *stat);

#endif /* _CONNFEM_INTERNAL_H */

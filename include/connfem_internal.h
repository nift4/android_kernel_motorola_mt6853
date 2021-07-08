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

/*******************************************************************************
 *                            D A T A   T Y P E S
 ******************************************************************************/
struct connfem_epaelna_total_info {
	struct connfem_epaelna_fem_info fem_info;
	struct connfem_epaelna_pin_info pin_info;
	struct connfem_epaelna_flags_wifi flags_wifi;
	struct connfem_epaelna_flags_bt flags_bt;
	bool connfem_available;
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
void connfem_epaelna_init(struct platform_device *pdev);
bool connfem_is_epaelna_available(void);
int _connfem_epaelna_get_fem_info(
				struct connfem_epaelna_fem_info *fem_info);

void connfem_dt_parser_init(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info);
void connfem_dt_parser_wifi(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info);
void connfem_dt_parser_bt(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info);

#endif /* _CONNFEM_INTERNAL_H */

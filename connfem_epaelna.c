// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "connfem_internal.h"

/*******************************************************************************
 *                        C O M P I L E R   F L A G S
 ******************************************************************************/

/*******************************************************************************
 *								M A C R O S
 ******************************************************************************/


/*******************************************************************************
 *							D A T A   T Y P E S
 ******************************************************************************/


/*******************************************************************************
 *				  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************/


/*******************************************************************************
 *						   P U B L I C   D A T A
 ******************************************************************************/

/*******************************************************************************
 *						  P R I V A T E   D A T A
 ******************************************************************************/
struct connfem_epaelna_total_info total_info;

/*******************************************************************************
 *							 F U N C T I O N S
 ******************************************************************************/
void connfem_epaelna_init(struct platform_device *pdev)
{
	memset(&total_info, 0, sizeof(total_info));

	connfem_dt_parser_init(pdev, &total_info);

	pr_info("total_info.connfem_available: %d",
			total_info.connfem_available);

#if (CONNFEM_DBG == 1)
	pr_info("total_info.flags_wifi.open_loop: %d",
			total_info.flags_wifi.open_loop);
	pr_info("total_info.flags_bt.bypass: %d",
			total_info.flags_bt.bypass);
	pr_info("total_info.flags_bt.epa_elna: %d",
			total_info.flags_bt.epa_elna);
	pr_info("total_info.flags_bt.epa: %d",
			total_info.flags_bt.epa);
	pr_info("total_info.flags_bt.elna: %d",
			total_info.flags_bt.elna);
#endif
}

bool connfem_is_epaelna_available(void)
{
	bool ret = false;

	if (total_info.connfem_available)
		ret = true;

	pr_info("%s ret: %d", __func__, ret);
	return ret;
}

int _connfem_epaelna_get_fem_info(struct connfem_epaelna_fem_info *fem_info)
{
	int ret = 0;
	int idx = 0;
	int size = 0;

	for (idx = 0; idx < CONNFEM_PORT_NUM; idx++) {
		fem_info->part[idx].pid = total_info.fem_info.part[idx].pid;
		fem_info->part[idx].vid = total_info.fem_info.part[idx].vid;
		size = sizeof(total_info.fem_info.part_name[idx]);
		pr_info("%s size = %d", __func__, size);

		if (size > CONNFEM_PART_NAME_SIZE-1)
			size = CONNFEM_PART_NAME_SIZE-1;

		strncpy(fem_info->part_name[idx],
				total_info.fem_info.part_name[idx], size);
		fem_info->part_name[idx][size] = 0;
	}

	return ret;
}

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
	pr_info("total_info.flags_wifi.laa: %d",
			total_info.flags_wifi.laa);
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
	int idx;

	memcpy(fem_info, &total_info.fem_info,
		sizeof(struct connfem_epaelna_fem_info));

	pr_info("GetFemInfo, id:0x%08x", fem_info->id);
	for (idx = 0; idx < CONNFEM_PORT_NUM; idx++) {
		pr_info("GetFemInfo, [%d]vid:0x%02x,pid:0x%2x,name:'%s'",
			idx,
			fem_info->part[idx].vid,
			fem_info->part[idx].pid,
			fem_info->part_name[idx]);
	}

	return 0;
}

int _connfem_epaelna_get_pin_info(struct connfem_epaelna_pin_info *pin_info)
{
	int idx;

	memcpy(pin_info, &total_info.pin_info,
		sizeof(struct connfem_epaelna_pin_info));

	pr_info("GetPinInfo, count:%d", pin_info->count);
	for (idx = 0; idx < pin_info->count; idx++) {
		pr_info("GetPinInfo, [%d]antsel:%d,fem:0x%02x,polarity:%d",
			idx,
			pin_info->pin[idx].antsel,
			pin_info->pin[idx].fem,
			pin_info->pin[idx].polarity);
	}

	return 0;
}

int _connfem_epaelna_get_flags_wifi(struct connfem_epaelna_flags_wifi *flags)
{
	memcpy(flags, &total_info.flags_wifi,
		sizeof(struct connfem_epaelna_flags_wifi));
	pr_info("GetFlagsWifi, open_loop:%d", flags->open_loop);
	return 0;
}

int _connfem_epaelna_get_flags_bt(struct connfem_epaelna_flags_bt *flags)
{
	memcpy(flags, &total_info.flags_bt,
		sizeof(struct connfem_epaelna_flags_bt));
	pr_info("GetFlagsBt, bypass  :%d", flags->bypass);
	pr_info("GetFlagsBt, epa_elna:%d", flags->epa_elna);
	pr_info("GetFlagsBt, epa     :%d", flags->epa);
	pr_info("GetFlagsBt, elna    :%d", flags->elna);
	return 0;
}

int get_flag_names_stat_from_total_info(
		struct connfem_ioctl_get_flag_names_stat *stat)
{
	int ret = 0;

	if (stat->subsys == CONNFEM_SUBSYS_WIFI &&
		total_info.wf_flag_names_cont) {
		stat->cnt = total_info.wf_flag_names_cont->cnt;
		stat->entry_sz = total_info.wf_flag_names_cont->entry_sz;

	} else if (stat->subsys == CONNFEM_SUBSYS_BT &&
				total_info.bt_flag_names_cont) {
		stat->cnt = total_info.bt_flag_names_cont->cnt;
		stat->entry_sz = total_info.bt_flag_names_cont->entry_sz;

	} else {
		pr_info("[WARN] IOC_FUNC_GET_FLAG_NAMES_STAT, invalid subsys:%d",
				stat->subsys);
		stat->cnt = 0;
		stat->entry_sz = 0;
		ret = -EINVAL;
	}

	return ret;
}

struct connfem_container *get_container(enum connfem_subsys subsys)
{
	struct connfem_container *container = NULL;

	if (subsys == CONNFEM_SUBSYS_WIFI)
		container = total_info.wf_flag_names_cont;
	else if (subsys == CONNFEM_SUBSYS_BT)
		container = total_info.bt_flag_names_cont;
	else
		pr_info("[WARN] %s, invalid subsys:%d", __func__, subsys);

	return container;
}

int _connfem_epaelna_laa_get_pin_info(
		struct connfem_epaelna_laa_pin_info *laa_pin_info)
{
	int idx;

	memcpy(laa_pin_info, &total_info.laa_pin_info,
		sizeof(struct connfem_epaelna_laa_pin_info));

	pr_info("GetLAAPinInfo, count:%d", laa_pin_info->count);
	for (idx = 0; idx < laa_pin_info->count; idx++) {
		pr_info("GetLAAPinInfo, [%d]gpio:%d,md_mode:%d,wf_mode:%d",
			idx,
			laa_pin_info->pin[idx].gpio,
			laa_pin_info->pin[idx].md_mode,
			laa_pin_info->pin[idx].wf_mode);
	}

	return 0;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/of.h>
#include "connfem_internal.h"

/*******************************************************************************
 *								M A C R O S
 ******************************************************************************/
/* wifi flag structure of device tree
 * wifi {
 *	wf0;
 *	wf1;
 *	open-loop;
 *	etsst-dc;
 * };
 */
#define WIFI_FLAG_WIFI0 0
#define WIFI_FLAG_WIFI1 1
#define WIFI_FLAG_OPEN_LOOP 2
#define WIFI_FLAG_ETSSI_DC 3

#define WIFI_FLAG_NAME "wifi"
#define WIFI_FLAG_WIFI0_NAME "wf0"
#define WIFI_FLAG_WIFI1_NAME "wf1"
#define WIFI_FLAG_OPEN_LOOP_NAME "open-loop"
#define WIFI_FLAG_ETSSI_DC_NAME "etssi-dc"

#define CONNFEM_WIFI_FLAG_NAME_SIZE 32

/*******************************************************************************
 *							D A T A   T Y P E S
 ******************************************************************************/
/* ConnFem DT Parser Definition  */
struct connfem_dt_flag_entry_struct {
	const char *name;
	bool *addr;
};

/* Wifi Flags Definition */
struct connfem_wifi_flag_entry_struct {
	bool en;
	char name[CONNFEM_WIFI_FLAG_NAME_SIZE];
};

/******************************************************************************
 * Please modify below structures if you would like to add or remove wifi flags
 *	1. connfem_wifi_flag_struct
 *	2. connfem_wifi_flag_struct wifi_flags
 *	3. connfem_dt_flag_entry_struct wifi_flags_mapping[]
 ******************************************************************************/
struct connfem_wifi_flag_struct {
	struct connfem_wifi_flag_entry_struct open_loop;
	struct connfem_wifi_flag_entry_struct laa;
};

/* Wifi Flags Storage & Mapping Table */
struct connfem_wifi_flag_struct wifi_flags = {
	.open_loop = {false, "open-loop"},
	.laa =  {false, "laa"},
};

struct connfem_dt_flag_entry_struct wifi_flags_mapping[] = {
	{wifi_flags.open_loop.name, &wifi_flags.open_loop.en},
	{wifi_flags.laa.name,  &wifi_flags.laa.en},
	{NULL, NULL}
};

static char **wf_flag_names_pa;

/*******************************************************************************
 *				  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************/


/*******************************************************************************
 *						   P U B L I C   D A T A
 ******************************************************************************/

/*******************************************************************************
 *						  P R I V A T E   D A T A
 ******************************************************************************/


/*******************************************************************************
 *							 F U N C T I O N S
 ******************************************************************************/
void connfem_dt_parser_wifi(struct device_node *devnode,
				struct connfem_epaelna_total_info *total_info,
				int parts_selectd)
{
	struct property *prop;
	struct device_node *np = NULL;
	struct device_node *flags_np = NULL;
	struct device_node *flags_num_np = NULL;
	int idx, err;
	const char *str = NULL;
	char child_name[16];
	struct connfem_container *wf_flag_names_cont = NULL;

	flags_np = of_get_child_by_name(devnode, "wifi");

	snprintf(child_name, sizeof(child_name), "flags-%d", parts_selectd);
#if (CONNFEM_DBG == 1)
		pr_info("composed to flags-%d: %s", parts_selectd, child_name);
#endif

	flags_num_np = of_get_child_by_name(flags_np, child_name);

	err = connfem_dt_parser_flag_names(flags_num_np,
			&total_info->wf_flag_names_cont,
			&wf_flag_names_pa);

	if (err < 0) {
		pr_info("%s, failed to get Wifi flags: %d", __func__, err);
	} else {
		wf_flag_names_cont = total_info->wf_flag_names_cont;
		pr_info("%s, found %d Wifi flags",
				__func__,
				wf_flag_names_cont->cnt);
		for (idx = 0; idx < wf_flag_names_cont->cnt; idx++) {
			str = (char *)connfem_container_entry(
					wf_flag_names_cont, idx);
			pr_info("%s, [%d] Container: '%s',PtrArray: '%s'",
					__func__, idx, str,
					wf_flag_names_pa[idx]);
		}
	}

	for_each_property_of_node(flags_num_np, prop) {

		/* skip properties added automatically */
		if (!of_prop_cmp(prop->name, "name"))
			continue;

		pr_info("wifi node get, flags_num_np->name: %s, prop->name: %s",
				flags_num_np->name, prop->name);
		idx = 0;
		while (wifi_flags_mapping[idx].name != NULL) {
			if (strncmp(prop->name,
				wifi_flags_mapping[idx].name,
				strlen(wifi_flags_mapping[idx].name)) == 0) {
				*wifi_flags_mapping[idx].addr = true;
				pr_info("[WIFI][%d] '%s': %d\n",
						idx,
						wifi_flags_mapping[idx].name,
						*wifi_flags_mapping[idx].addr);
			}
			idx++;
		}
	}
	of_node_put(np);
	of_node_put(flags_num_np);

	total_info->flags_wifi.open_loop = *wifi_flags_mapping[0].addr;
	total_info->flags_wifi.laa = *wifi_flags_mapping[1].addr;

	pr_info("total_info->flags_wifi.open_loop: %d",
			total_info->flags_wifi.open_loop);
	pr_info("total_info->flags_wifi.laa: %d",
			total_info->flags_wifi.laa);
}

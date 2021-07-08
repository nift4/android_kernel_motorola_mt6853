// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/of.h>
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
/* ConnFem DT Parser Definition  */
struct connfem_dt_flag_entry_struct {
	const char *name;
	bool *addr;
};

/*******************************************************************************
 * Please modify below structures if you would like to add or remove bt flags
 *	1. connfem_epaelna_flags_bt on connfem_api.h
 *	2. connfem_dt_flag_entry_struct bt_flags_mapping[]
 ******************************************************************************/
/* BT Flags Storage & Mapping Table */
struct connfem_epaelna_flags_bt bt_flags;

struct connfem_dt_flag_entry_struct bt_flags_mapping[] = {
	{"bypass",      &bt_flags.bypass},
	{"epa_elna",    &bt_flags.epa_elna},
	{"epa",         &bt_flags.epa},
	{"elna",        &bt_flags.elna},
	{NULL, NULL}
};

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

void connfem_dt_parser_bt(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info)
{
	const struct device_node *devnode = pdev->dev.of_node;
	struct device_node *epaelna_node = NULL;
	struct property *prop;
	struct device_node *np = NULL;
	int idx;

	epaelna_node = of_get_child_by_name(devnode, "epa_elna");
	np = of_get_child_by_name(epaelna_node, "bt");
	if (!np) {
		pr_info("Missing bt child");
		return;
	}
	for_each_property_of_node(np, prop) {

		/* skip properties added automatically */
		if (!of_prop_cmp(prop->name, "name"))
			continue;

		pr_info("bt node get, np->name: %s, prop->name: %s",
				np->name, prop->name);
		idx = 0;
		while (bt_flags_mapping[idx].name != NULL) {
			if (strncmp(prop->name, bt_flags_mapping[idx].name,
				strlen(bt_flags_mapping[idx].name)) == 0) {
				*bt_flags_mapping[idx].addr = true;
				pr_info("[BT][%d] '%s': %d\n",
						idx, bt_flags_mapping[idx].name,
						*bt_flags_mapping[idx].addr);
			}
			idx++;
		}
	}
	of_node_put(np);

	memcpy(&total_info->flags_bt, &bt_flags,
			sizeof(struct connfem_epaelna_flags_bt));

	pr_info("total_info.flags_bt.bypass: %d",
			total_info->flags_bt.bypass);
	pr_info("total_info.flags_bt.epa_elna: %d",
			total_info->flags_bt.epa_elna);
	pr_info("total_info.flags_bt.epa: %d",
			total_info->flags_bt.epa);
}

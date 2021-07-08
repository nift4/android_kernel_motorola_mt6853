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

static char **bt_flag_names_pa;

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

void connfem_dt_parser_bt(struct device_node *devnode,
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
	struct connfem_container *bt_flag_names_cont = NULL;

	flags_np = of_find_node_by_name(devnode, "bt");

	snprintf(child_name, sizeof(child_name), "flags-%d", parts_selectd);
#if (CONNFEM_DBG == 1)
		pr_info("composed to flags-%d: %s", parts_selectd, child_name);
#endif

	flags_num_np = of_get_child_by_name(flags_np, child_name);

	err = connfem_dt_parser_flag_names(flags_num_np,
				&total_info->bt_flag_names_cont,
				&bt_flag_names_pa);
	if (err < 0) {
		pr_info("%s, failed to get BT flags: %d", __func__, err);
	} else {
		bt_flag_names_cont = total_info->bt_flag_names_cont;
		pr_info("%s, found %d BT flags",
				__func__,
				bt_flag_names_cont->cnt);
		for (idx = 0; idx < bt_flag_names_cont->cnt; idx++) {
			str = (char *)connfem_container_entry(
					bt_flag_names_cont, idx);
			pr_info("%s, [%d] Container: '%s',PtrArray: '%s'",
					__func__, idx, str,
					bt_flag_names_pa[idx]);
		}
	}

	for_each_property_of_node(flags_num_np, prop) {

		/* skip properties added automatically */
		if (!of_prop_cmp(prop->name, "name"))
			continue;

		pr_info("bt node get, flags_num_np->name: %s, prop->name: %s",
				flags_num_np->name, prop->name);
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
	of_node_put(flags_num_np);

	memcpy(&total_info->flags_bt, &bt_flags,
			sizeof(struct connfem_epaelna_flags_bt));

	pr_info("total_info.flags_bt.bypass: %d",
			total_info->flags_bt.bypass);
	pr_info("total_info.flags_bt.epa_elna: %d",
			total_info->flags_bt.epa_elna);
	pr_info("total_info.flags_bt.epa: %d",
			total_info->flags_bt.epa);
	pr_info("total_info.flags_bt.elna: %d",
			total_info->flags_bt.elna);
}

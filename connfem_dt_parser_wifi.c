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
	struct connfem_wifi_flag_entry_struct wf0;
	struct connfem_wifi_flag_entry_struct wf1;
	struct connfem_wifi_flag_entry_struct open_loop;
	struct connfem_wifi_flag_entry_struct etssi_dc;
};

/* Wifi Flags Storage & Mapping Table */
struct connfem_wifi_flag_struct wifi_flags = {
	.wf0 =       {false, "wf0"},
	.wf1 =       {false, "wf1"},
	.open_loop = {false, "open-loop"},
	.etssi_dc =  {false, "etssi-dc"},
};

struct connfem_dt_flag_entry_struct wifi_flags_mapping[] = {
	{wifi_flags.wf0.name,       &wifi_flags.wf0.en},
	{wifi_flags.wf1.name,       &wifi_flags.wf1.en},
	{wifi_flags.open_loop.name, &wifi_flags.open_loop.en},
	{wifi_flags.etssi_dc.name,  &wifi_flags.etssi_dc.en},
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
void connfem_dt_parser_wifi(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info)
{
	const struct device_node *devnode = pdev->dev.of_node;
	struct device_node *epaelna_node = NULL;
	struct property *prop;
	struct device_node *np = NULL;
	int idx;

	epaelna_node = of_get_child_by_name(devnode, "epa_elna");

	np = of_get_child_by_name(epaelna_node, "wifi");
	if (!np) {
		pr_info("Missing wifi child");
		return;
	}
	for_each_property_of_node(np, prop) {

		/* skip properties added automatically */
		if (!of_prop_cmp(prop->name, "name"))
			continue;

		pr_info("wifi node get, np->name: %s, prop->name: %s",
				np->name, prop->name);
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

	total_info->flags_wifi.open_loop = *wifi_flags_mapping[2].addr;

	pr_info("total_info.flags_wifi.open_loop: %d",
			total_info->flags_wifi.open_loop);
}

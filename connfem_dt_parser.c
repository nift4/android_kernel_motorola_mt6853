// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include "connfem_internal.h"

/*******************************************************************************
 *								M A C R O S
 ******************************************************************************/
/*	Align with firmware FemNo coding
 *	0xAABCDDEE:
 *		AA: reserved
 *		 B:	2G VID
 *		 C:	5G VID
 *		DD:	2G PID
 *		EE:	5G VID
 */

#define VID_2G(n) ((n) << 20)
#define VID_5G(n) ((n) << 16)
#define PID_2G(n) ((n) << 8)
#define PID_5G(n) ((n) << 0)

#define COMPOSED_STATE_NAME_SIZE (CONNFEM_PART_NAME_SIZE * \
				CONNFEM_PORT_NUM + (CONNFEM_PORT_NUM - 1) + 1)
#define FEM_MAPPING_SIZE 3
#define GPIO_NODE_NAME "gpio-hwid"

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

/*******************************************************************************
 *							 F U N C T I O N S
 ******************************************************************************/


/**
 * connfem_dt_parser_init - it is called when driver probe and it will parse
 * all information from device tree and save it to connfem_epaelna_total_info
 *
 * @pdev: platform device is passed from probe function
 * @total_info: structure for saving device tree information
 */
void connfem_dt_parser_init(struct platform_device *pdev,
				struct connfem_epaelna_total_info *total_info)
{
	const struct device_node *devnode = pdev->dev.of_node;
	struct device_node *epaelna_node = NULL;
	int gpio_num = 0;
	int gpio_count = 0;
	int parts_selected = 0;
	int idx, err = 0;
	unsigned int value;
	int parts_count = 0;
	struct device_node *np = NULL;
	int port_num = 0;
	int pid = 0;
	int vid = 0;
	char composed_state_name[COMPOSED_STATE_NAME_SIZE];
	struct pinctrl *pinctrl = NULL;
	struct pinctrl_state *pin_state = NULL;
	char prop_name[16];
	int idx2, idx3, cur_idx;
	int err2, err3;
	unsigned int value2, value3;
	int cnt, cnt2;
	const char *str = NULL;
	struct device_node *pinctrl_elem_np = NULL;
	struct connfem_epaelna_pin_info pin_info = total_info->pin_info;
	struct connfem_epaelna_fem_info fem_info = total_info->fem_info;

	epaelna_node = of_get_child_by_name(devnode, "epa_elna");

	/* Get GPIO value to determine which parts should be used */
	gpio_count = of_gpio_named_count(epaelna_node, GPIO_NODE_NAME);
	if (gpio_count <= 0) {
		pr_info("failed to find %s: %d", GPIO_NODE_NAME, gpio_num);
	} else {
		for (idx = 0; idx < gpio_count; idx++) {
			gpio_num = of_get_named_gpio(epaelna_node,
							GPIO_NODE_NAME, idx);
			if (gpio_is_valid(gpio_num)) {
				err = devm_gpio_request(&pdev->dev, gpio_num,
								GPIO_NODE_NAME);
				if (err < 0) {
					err = -EINVAL;
					pr_info("[WARN] failed to request GPIO %d: index: %d, error: %d",
						   gpio_num, idx, err);
					break;
				}
				value = gpio_get_value(gpio_num);
#if (CONNFEM_DBG == 1)
				pr_info("%s PIN:%d, Value:%d",
						GPIO_NODE_NAME, gpio_num,
						value);
				pr_info("before OR, parts_selected:%d, value:%d, idx: %d",
						parts_selected, value, idx);
#endif
				parts_selected |= value << idx;
#if (CONNFEM_DBG == 1)
				pr_info("after OR, parts_selected: %d, value: %d, idx: %d",
						parts_selected, value, idx);
#endif
			} else {
				err = -EINVAL;
				pr_info("[WARN] failed to find %s: index: %d, gpio: %d, err: %d",
						GPIO_NODE_NAME, idx,
						gpio_num, err);
				break;
			}
		}
	}

	if (err < 0) {
		parts_selected = 0;
		pr_info("Get GPIO value failed, set parts_selected to 0");
	}

	pr_info("parts_selected: %d", parts_selected);

	parts_count = of_property_count_u32_elems(epaelna_node, "parts");
	pr_info("number of parts: %d", parts_count);
	if (parts_count <= 0) {
		pr_info("[WARN] Missing FEM parts");
		return;
	}
	if ((parts_selected + 1) * CONNFEM_PORT_NUM > parts_count) {
		pr_info("[WARN] parts_selected(%d) + 1 * %d > parts_count(%d)",
				parts_selected, CONNFEM_PORT_NUM, parts_count);
		return;
	}
	if ((parts_count % CONNFEM_PORT_NUM) != 0) {
		pr_info("[WARN] parts_count: %d must be multiple of CONNFEM_PORT_NUM: %d",
				parts_count, CONNFEM_PORT_NUM);
		return;
	}

	/* Parts */
	idx = parts_selected * CONNFEM_PORT_NUM;
	port_num = 0;
	pr_info("parts_selected: %d, read parts from %d",
			parts_selected, idx);
	while ((np = of_parse_phandle(epaelna_node, "parts", idx)) &&
			port_num < CONNFEM_PORT_NUM) {
		vid = 0;
		pid = 0;

#if (CONNFEM_DBG == 1)
		pr_info("Found parts[%d] '%s':'%s'",
				idx, np->name, np->full_name);
#endif

		strncpy(fem_info.part_name[port_num],
				np->full_name, CONNFEM_PART_NAME_SIZE-1);
		fem_info.part_name[port_num][CONNFEM_PART_NAME_SIZE-1] = 0;

		err = of_property_read_u32(np, "vid", &vid);
		if (err < 0) {
			pr_info("Fetch parts[%d].vid failed: %d",
					idx, err);
		} else {
			pr_info("Fetch parts[%d].vid ok: %d",
					idx, vid);
			fem_info.part[port_num].vid = vid;
			switch (port_num) {
			case CONNFEM_PORT_WF0:
				fem_info.id |= VID_2G(vid);
				break;
			case CONNFEM_PORT_WF1:
				fem_info.id |= VID_5G(vid);
				break;
			default:
				pr_info("unknown port_num: %d",
						port_num);
				break;
			}
		}

		err = of_property_read_u32(np, "pid", &pid);
		if (err < 0) {
			pr_info("Fetch parts[%d].pid failed: %d",
					idx, err);
		} else {
			pr_info("Fetch parts[%d].pid ok: %d",
					idx, pid);
			fem_info.part[port_num].pid = pid;
			switch (port_num) {
			case CONNFEM_PORT_WF0:
				fem_info.id |= PID_2G(pid);
				break;
			case CONNFEM_PORT_WF1:
				fem_info.id |= PID_5G(pid);
				break;
			default:
				pr_info("unknown port_num: %d",
						port_num);
				break;
			}
		}

		of_node_put(np);

		idx++;
		port_num++;
	}

	pr_info("fem_info.id: 0x%08x", fem_info.id);
#if (CONNFEM_DBG == 1)
	for (idx = 0; idx < CONNFEM_PORT_NUM; idx++) {
		pr_info("fem_info.part[%d].vid: %d",
				idx,
				fem_info.part[idx].vid);
		pr_info("fem_info.part[%d].pid: %d",
				idx,
				fem_info.part[idx].pid);
		pr_info("fem_info.part_name[%d]: %s",
				idx,
				fem_info.part_name[idx]);
	}
#endif

	/* Compose pinctrl-names from parts */
	snprintf(composed_state_name, COMPOSED_STATE_NAME_SIZE-1, "%s_%s",
			 fem_info.part_name[CONNFEM_PORT_WF0],
			 fem_info.part_name[CONNFEM_PORT_WF1]);
	composed_state_name[COMPOSED_STATE_NAME_SIZE-1] = 0;

#if (CONNFEM_DBG == 1)
	pr_info("composed_state_name: %s", composed_state_name);
#endif

	/* Pinctrl - PINMUX */
	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		pr_info("unable to get pinctrl: %ld",
				PTR_ERR(pinctrl));
	} else {
		pin_state = pinctrl_lookup_state(pinctrl, composed_state_name);
		if (IS_ERR(pin_state)) {
			pr_info("unable to find state '%s': %ld",
					composed_state_name,
					PTR_ERR(pin_state));
		} else {
			err = pinctrl_select_state(pinctrl, pin_state);

			if (err < 0)
				pr_info("failed to apply pinctrl: %d", err);
			else
				pr_info("'%s' pinmux successfully applied",
						composed_state_name);
		}
		devm_pinctrl_put(pinctrl);
		pinctrl = NULL;
	}

	/* Pinctrl - FEM Function Mapping */
	idx2 = 0;
	cur_idx = 0;

	/* Find all pinctrl-names */
	cnt = of_property_count_strings(devnode, "pinctrl-names");
	pr_info("number of elements in 'pinctrl-names': %d", cnt);
	for (idx = 0; idx < cnt; idx++) {
		/* List all pinctrl-names */
		err = of_property_read_string_index(devnode, "pinctrl-names",
								idx, &str);
		if (err < 0) {
#if (CONNFEM_DBG == 1)
			pr_info("failed to get pinctrl-names[%d]: %d",
					idx, err);
#endif
			continue;
		}

		/* Compare between pinctrl-names with composed part name */
		if (strncmp(str, composed_state_name,
			strlen(composed_state_name)) != 0) {
			pr_info("pinctrl-names[%d]: '%s'",
			idx, str);
			continue;
		}

		/* Found pinctrl-names == composed part name */
#if (CONNFEM_DBG == 1)
		pr_info("pinctrl-names[%d]: '%s' matched",
				idx, str);
#endif

		snprintf(prop_name,
				 sizeof(prop_name),
				 "pinctrl-%d", idx);
#if (CONNFEM_DBG == 1)
		pr_info("pinctrl-names[%d]: '%s' => '%s'",
				idx, str, prop_name);
#endif

		/* List all nodes of pinctrl-n to find GPIO pinmux */
		while ((pinctrl_elem_np =
				of_parse_phandle(devnode,
				prop_name, idx2))) {
			if (!pinctrl_elem_np) {
				/* Can not find corresponding GPIO pinmux */
				pr_info("[WARN] failed to get '%s', index: %d",
					   prop_name, idx2);
				idx2++;
				continue;
			}
			np = of_find_node_by_name(
				pinctrl_elem_np,
				"pins_cmd_dat");
			if (!np) {
				/* Can not find pins_cmd_dat */
				pr_info("[WARN] failed to get 'pins_cmd_dat' under %s",
						prop_name);
				of_node_put(pinctrl_elem_np);
				pinctrl_elem_np = NULL;
				idx2++;
				continue;
			}
			cnt2 = of_property_count_u32_elems(np, "mapping");
			pr_info("total num of elements in pins_cmd_dat.'mapping': %d",
					cnt2);
			if (cnt2 > 0 && (cnt2 % FEM_MAPPING_SIZE) != 0) {
				pr_info("total num of elements in 'mapping'(%d) is not a multiple of %d",
						cnt2,
						FEM_MAPPING_SIZE);
			} else {
				for (idx3 = 0; idx3 < cnt2;
					idx3 += FEM_MAPPING_SIZE) {
					value = value2 = value3 = 0;
					err = of_property_read_u32_index(
							np, "mapping",
							idx3, &value);
					err2 = of_property_read_u32_index(
							np, "mapping",
							idx3 + 1, &value2);
					err3 = of_property_read_u32_index(
							np, "mapping",
							idx3 + 2, &value3);
					pr_info("[%d]ANTSEL{er:%d,value:%d}, [%d]FemPIN{er:%d,value:%d}, [%d]polarity{er:%d,value:%d}",
						idx3, err, value,
						idx3 + 1, err2, value2,
						idx3 + 2, err3, value3);
					pin_info.pin[cur_idx].antsel = value;
					pin_info.pin[cur_idx].fem = value2;
					pin_info.pin[cur_idx].polarity = value3;
					cur_idx++;
				}
				pin_info.count = cur_idx;
			}
			of_node_put(np);
			np = NULL;

			of_node_put(pinctrl_elem_np);
			pinctrl_elem_np = NULL;
			idx2++;
		}
		break;
	}

	//Set it to true if any pin mapping is existed
	if (pin_info.count > 0)
		total_info->connfem_available = true;

#if (CONNFEM_DBG == 1)
	pr_info("total_info->pin_info.count: %d",
			pin_info.count);
	for (idx = 0; idx < pin_info.count; idx++) {
		pr_info("total_info->pin_info.pin[%d].antsel: %d",
				idx, pin_info.pin[idx].antsel);
		pr_info("total_info->pin_info.pin[%d].fem: %d",
				idx, pin_info.pin[idx].fem);
		pr_info("total_info->pin_info.pin[%d].polarity: %d",
				idx, pin_info.pin[idx].polarity);
	}
#endif

	/* Get wifi and bt flags */
	connfem_dt_parser_wifi(pdev, total_info);
	connfem_dt_parser_bt(pdev, total_info);

}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <dt-bindings/pinctrl/mt65xx.h>
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
#define LAA_PINMUX_SIZE 2
#define GPIO_NODE_NAME "gpio-hwid"
#define EPA_ELNA_NAME "epa_elna"
#define EPA_ELNA_MTK_NAME "epa_elna_mtk"

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
bool __weak connfem_is_internal(void)
{
	return false;
}

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
	bool internal_load = false;
	bool device_tree_is_invalid = false;
	struct connfem_epaelna_fem_info *fem_info = &total_info->fem_info;
	int idx5;
	struct pinctrl_state *pin_state = NULL;
	struct pinctrl *pinctrl = NULL;
	int idx4;
	int idx2;
	int cnt;
	const char *str = NULL;
	char prop_name[16];
	struct connfem_epaelna_pin_info *pin_info = &total_info->pin_info;
	struct device_node *pinctrl_elem_np = NULL;
	int cnt2;
	int idx3;
	int err2, err3;
	unsigned int value2, value3;
	unsigned int gpio_value, gpio_value2, md_mode_value, wf_mode_value;
	int cnt3;
	struct connfem_epaelna_laa_pin_info *laa_p_info =
			&total_info->laa_pin_info;

	internal_load = connfem_is_internal();
	pr_info("connfem_is_internal(): %d", internal_load);

	if (internal_load) {
		epaelna_node = of_get_child_by_name(devnode, EPA_ELNA_MTK_NAME);
		pr_info("Using %s", EPA_ELNA_MTK_NAME);
	} else {
		epaelna_node = of_get_child_by_name(devnode, EPA_ELNA_NAME);
		pr_info("Using %s", EPA_ELNA_NAME);
	}

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
			port_num < CONNFEM_PORT_NUM &&
			device_tree_is_invalid == false) {
		vid = 0;
		pid = 0;

#if (CONNFEM_DBG == 1)
		pr_info("Found parts[%d] '%s':'%s'",
				idx, np->name, np->full_name);
#endif

		strncpy(fem_info->part_name[port_num],
				np->name, CONNFEM_PART_NAME_SIZE-1);
		fem_info->part_name[port_num][CONNFEM_PART_NAME_SIZE-1] = 0;

		err = of_property_read_u32(np, "vid", &vid);
		if (err < 0) {
			pr_info("[WARN] Fetch parts[%d].vid failed: %d",
					idx, err);
			device_tree_is_invalid = true;
		} else {
			pr_info("Fetch parts[%d].vid ok: %d",
					idx, vid);
			fem_info->part[port_num].vid = vid;
			switch (port_num) {
			case CONNFEM_PORT_WF0:
				fem_info->id |= VID_2G(vid);
				break;
			case CONNFEM_PORT_WF1:
				fem_info->id |= VID_5G(vid);
				break;
			default:
				pr_info("[WARN] unknown port_num: %d",
						port_num);
				device_tree_is_invalid = true;
				break;
			}
		}

		err = of_property_read_u32(np, "pid", &pid);
		if (err < 0) {
			pr_info("[WARN] Fetch parts[%d].pid failed: %d",
					idx, err);
			device_tree_is_invalid = true;
		} else {
			pr_info("Fetch parts[%d].pid ok: %d",
					idx, pid);
			fem_info->part[port_num].pid = pid;
			switch (port_num) {
			case CONNFEM_PORT_WF0:
				fem_info->id |= PID_2G(pid);
				break;
			case CONNFEM_PORT_WF1:
				fem_info->id |= PID_5G(pid);
				break;
			default:
				pr_info("[WARN] unknown port_num: %d",
						port_num);
				device_tree_is_invalid = true;
				break;
			}
		}

		of_node_put(np);

		idx++;
		port_num++;
	}

	pr_info("total_info->fem_info.id: 0x%08x", total_info->fem_info.id);
#if (CONNFEM_DBG == 1)
	for (idx = 0; idx < CONNFEM_PORT_NUM; idx++) {
		pr_info("total_info->fem_info.part[%d].vid: %d",
				idx,
				total_info->fem_info.part[idx].vid);
		pr_info("total_info->fem_info.part[%d].pid: %d",
				idx,
				total_info->fem_info.part[idx].pid);
		pr_info("total_info->fem_info.part_name[%d]: %s",
				idx,
				total_info->fem_info.part_name[idx]);
	}
#endif

	if (total_info->fem_info.id == 0 || device_tree_is_invalid == true) {
		pr_info("[WARN] Please make sure fem id is correct, 0x%08x, %d",
				total_info->fem_info.id,
				device_tree_is_invalid);
		return;
	}

	/* Compose pinctrl-names from parts */
	snprintf(composed_state_name, COMPOSED_STATE_NAME_SIZE-1, "%s_%s",
			 fem_info->part_name[CONNFEM_PORT_WF0],
			 fem_info->part_name[CONNFEM_PORT_WF1]);
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

			if (err < 0) {
				pr_info("failed to apply pinctrl: %d", err);
			}
			else
				pr_info("'%s' pinmux successfully applied",
						composed_state_name);
		}
		devm_pinctrl_put(pinctrl);
		pinctrl = NULL;
	}

	/* Pinctrl - FEM Function Mapping */
	idx2 = 0;
	idx4 = 0;
	idx5 = 0;

	/* Find all pinctrl-names */
	cnt = of_property_count_strings(devnode, "pinctrl-names");
	pr_info("number of elements in 'pinctrl-names': %d", cnt);

	for (idx = 0; idx < cnt; idx++) {
		/* List all pinctrl-names */
		err = of_property_read_string_index(devnode, "pinctrl-names",
								idx, &str);
		if (err < 0) {
			pr_info("failed to get pinctrl-names[%d]: %d",
					idx, err);
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
			/* Parse mapping node */
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
					pr_info("[%d]ANTSEL{er:%d,value:%d}, [%d]FemPIN{er:%d,value:0x%02x}, [%d]polarity{er:%d,value:%d}",
						idx3, err, value,
						idx3 + 1, err2, value2,
						idx3 + 2, err3, value3);
					pin_info->pin[idx4].antsel =
						value;
					pin_info->pin[idx4].fem =
						value2;
					pin_info->pin[idx4].polarity =
						value3;
					idx4++;
				}
				pin_info->count = idx4;
			}

			/* Parse laa-pinmux node */
			idx3 = 0;
			cnt3 = of_property_count_u32_elems(np, "laa-pinmux");
			pr_info("total num of elements in pins_cmd_dat.'laa-pinmux': %d",
					cnt3);
			if (cnt3 > 0 && (cnt3 % LAA_PINMUX_SIZE) != 0) {
				pr_info("total num of elements in 'mapping'(%d) is not a multiple of %d",
						cnt3,
						LAA_PINMUX_SIZE);
			} else {
				for (idx3 = 0; idx3 < cnt3;
					idx3 += LAA_PINMUX_SIZE) {
					value = value2 = 0;
					err = of_property_read_u32_index(
							np, "laa-pinmux",
							idx3, &value);
					err2 = of_property_read_u32_index(
							np, "laa-pinmux",
							idx3 + 1, &value2);
					gpio_value = MTK_GET_PIN_NO(value);
					gpio_value2 = MTK_GET_PIN_NO(value2);
					wf_mode_value = MTK_GET_PIN_FUNC(value);
					md_mode_value =
						MTK_GET_PIN_FUNC(value2);
					pr_info("[%d]gpio1{er:%d,value:%d}, gpio2{er:%d,value:%d}, wf_mode{er:%d,value:%d}, md_mode{er:%d,value:%d}",
						idx3, err, gpio_value,
						err2, gpio_value2,
						err, wf_mode_value,
						err2, md_mode_value);

					if (gpio_value == gpio_value2) {
						laa_p_info->pin[idx5].gpio =
							gpio_value;
						laa_p_info->pin[idx5].wf_mode =
							wf_mode_value;
						laa_p_info->pin[idx5].md_mode =
							md_mode_value;
						idx5++;
					} else {
						pr_info("[WARN] laa-pinmux get 2 different gpio: %d and %d",
								gpio_value,
								gpio_value2);
						break;
					}
				}
				laa_p_info->count = idx5;
			}

			of_node_put(np);
			np = NULL;

			of_node_put(pinctrl_elem_np);
			pinctrl_elem_np = NULL;

			idx2++;
		}
		break;
	}

	/* Get wifi and bt flags */
	connfem_dt_parser_wifi(epaelna_node, total_info, parts_selected);
	connfem_dt_parser_bt(epaelna_node, total_info, parts_selected);

	/* Set it to true if any pin mapping is existed */
	if (fem_info->id > 0 &&
		device_tree_is_invalid == false)
		total_info->connfem_available = true;

	/* Make sure all values are 0 if connfem_available is false */
	if (total_info->connfem_available == false)
		memset(total_info, 0,
			sizeof(struct connfem_epaelna_total_info));

#if (CONNFEM_DBG == 1)
	pr_info("total_info->pin_info.count: %d",
			total_info->pin_info.count);
	for (idx = 0; idx < total_info->pin_info.count; idx++) {
		pr_info("total_info->pin_info.pin[%d].antsel: %d",
				idx, total_info->pin_info.pin[idx].antsel);
		pr_info("total_info->pin_info.pin[%d].fem: 0x%02x",
				idx, total_info->pin_info.pin[idx].fem);
		pr_info("total_info->pin_info.pin[%d].polarity: %d",
				idx, total_info->pin_info.pin[idx].polarity);
	}

	pr_info("total_info->laa_pin_info.count: %d",
			total_info->laa_pin_info.count);
	for (idx = 0; idx < total_info->laa_pin_info.count; idx++) {
		pr_info("total_info->laa_pin_info.pin[%d].gpio: %d",
				idx, total_info->laa_pin_info.pin[idx].gpio);
		pr_info("total_info->laa_pin_info.pin[%d].md_mode: %d",
				idx, total_info->laa_pin_info.pin[idx].md_mode);
		pr_info("total_info->laa_pin_info.pin[%d].wf_mode: %d",
				idx, total_info->laa_pin_info.pin[idx].wf_mode);
	}
#endif
}

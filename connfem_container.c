// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include "connfem_internal.h"

/*******************************************************************************
 *                        C O M P I L E R   F L A G S
 ******************************************************************************/
 
/*******************************************************************************
 *                              M A C R O S
 ******************************************************************************/
 
 
/*******************************************************************************
 *                          D A T A   T Y P E S
 ******************************************************************************/

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
struct connfem_container* connfem_container_alloc(unsigned int cnt, unsigned int entry_sz)
{
	struct connfem_container *container;
	unsigned int sz;

	sz = sizeof(struct connfem_container) + (cnt * entry_sz);
	container = (struct connfem_container*)kmalloc(sz, GFP_KERNEL);
	if (!container) {
		pr_info("failed to allocate %d bytes", sz);
		return NULL;
	}
	memset(container, 0, sz);

	container->cnt = cnt;
	container->entry_sz = entry_sz;

	return container;
};

void* connfem_container_entry(struct connfem_container *container, unsigned int idx)
{
	if (container) {
		if (idx < container->cnt) {
			return &container->buffer[idx * container->entry_sz];
		}
		pr_info("ConnFem container index out-of-bound: %d >= %d", idx, container->cnt);
	}
	return NULL;
}

void** connfem_container_create_pointers_array(struct connfem_container *cont)
{
	unsigned int sz, idx;
	void **pa;

	if (!cont || cont->cnt == 0) {
		pr_info("container is empty");
		return NULL;
	}

	sz = cont->cnt * sizeof(void*);
	pa = (void**)kmalloc(sz, GFP_KERNEL);
	if (!pa) {
		pr_info("failed to allocate %d bytes", sz);
		return NULL;
	}
	memset(pa, 0, sz);

	for (idx = 0; idx < cont->cnt; idx++) {
		pa[idx] = connfem_container_entry(cont, idx);
	}

	return pa;
}

int connfem_dt_parser_flag_names(struct device_node *node, struct connfem_container **flags_out, char ***names_out)
{
	unsigned int cnt, len;
	struct connfem_container *flags = NULL;
	struct property *prop = NULL;
	char **names = NULL;

	/* Get number of flags in this node */
	cnt = 0;
	for_each_property_of_node(node, prop) {
		cnt++;
	}

	/* Prepare ConnFem container */
	flags = connfem_container_alloc(cnt, CONNFEM_FLAG_NAME_SIZE);
	if (!flags) {
		pr_info("failed to allocate flag names container (%d * %d bytes)", cnt, CONNFEM_FLAG_NAME_SIZE);
		return -ENOMEM;
	}

	/* Retrieves flags name from device tree */
	cnt = 0;
	for_each_property_of_node(node, prop) {
		/* Skip built-in property 'name' */
		if (strcmp(prop->name, "name") == 0)
			continue;

		/* Discard flag with name exceeding length limit */
		len = strlen(prop->name) + 1;
		if (len > CONNFEM_FLAG_NAME_SIZE) {
			pr_info("Discard node '%s' property '%s', exceeds maximum name length (%d > %d)", node->name, prop->name, len - 1, CONNFEM_FLAG_NAME_SIZE - 1);
			continue;
		}

		/* Double check if container is big enough to keep this flag */
		if (cnt + 1 <= flags->cnt) {
			memcpy(connfem_container_entry(flags, cnt), prop->name, len);
			cnt++;
		} else {
			pr_info("Unexpected: Flags count(%d) exceeds container size(%d)", cnt + 1, flags->cnt);
		}
	}

	/* Updates container size as the end result could be smaller due to removal of long name */
	flags->cnt = cnt;

	/* Prepare name pointers array */
	if (flags->cnt > 0) {
		names = (char**)connfem_container_create_pointers_array(flags);
		if (!names) {
			kfree(flags);
			return -ENOMEM;
		}
	}

	/* Update output parameters */
	*flags_out = flags;
	*names_out = names;
	return 0;
}
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
*/
#include <linux/regulator/consumer.h>
void vibr_Enable_HW(struct regulator *reg);
void vibr_Disable_HW(struct regulator *reg);
void vibr_power_set(void);
#ifdef CONFIG_SEPERATE_VOLT_FORLONGVIB
void vibr_power_set_long(void);
#endif
struct vibrator_hw *mt_get_cust_vibrator_hw(void);

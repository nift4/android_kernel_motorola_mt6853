// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include "connfem_internal.h"

/*******************************************************************************
 *								M A C R O S
 ******************************************************************************/
#define CONNFEM_DRIVER_NAME "connfem"
#define CONNFEM_DEV_NUM 1

/*******************************************************************************
 *							D A T A   T Y P E S
 ******************************************************************************/
struct connfem_plat_data {
	unsigned int id;
};

struct connfem_cdev_context {
	dev_t devId;
	struct class *class;
	struct device *chrdev;
	struct cdev cdev;
	struct platform_device *device;
};

/*******************************************************************************
 *				  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************/
static int connfem_plat_probe(struct platform_device *pdev);
static int connfem_plat_remove(struct platform_device *pdev);

static int connfem_dev_open(struct inode *inode, struct file *file);
static int connfem_dev_close(struct inode *inode, struct file *file);
static ssize_t connfem_dev_read(struct file *filp, char __user *buf,
						size_t count, loff_t *f_pos);
static ssize_t connfem_dev_write(struct file *filp, const char __user *buf,
						size_t count, loff_t *f_pos);
static long connfem_dev_unlocked_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg);
#ifdef CONFIG_COMPAT
static long connfem_dev_compat_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg);
#endif

/*******************************************************************************
 *						   P U B L I C   D A T A
 ******************************************************************************/

/*******************************************************************************
 *						  P R I V A T E   D A T A
 ******************************************************************************/
/* Platform Device */
#ifdef CONFIG_OF

struct connfem_plat_data connfem_plat_mt6893 = {
	.id = 0x6893
};
struct connfem_plat_data connfem_plat_mt6877 = {
	.id = 0x6877
};

static const struct of_device_id connfem_of_ids[] = {
	{
		.compatible = "mediatek,mt6893-connfem",
		.data = (void *)&connfem_plat_mt6893
	},
	{
		.compatible = "mediatek,mt6877-connfem",
		.data = (void *)&connfem_plat_mt6877
	},
	{}
};
#endif

static struct platform_driver connfem_plat_drv = {
	.probe = connfem_plat_probe,
	.remove = connfem_plat_remove,
	.driver = {
		   .name = "mtk_connfem",
		   .owner = THIS_MODULE,
#ifdef CONFIG_OF
		   .of_match_table = connfem_of_ids,
#endif
		   },
};

/* Char Device */
static int connfem_major;
static struct connfem_cdev_context connfem_cdev_ctx;

static const struct file_operations connfem_dev_fops = {
	.open = connfem_dev_open,
	.release = connfem_dev_close,
	.read = connfem_dev_read,
	.write = connfem_dev_write,
	.unlocked_ioctl = connfem_dev_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = connfem_dev_compat_ioctl,
#endif
};

/*******************************************************************************
 *							 F U N C T I O N S
 ******************************************************************************/
static int connfem_dev_open(struct inode *inode, struct file *file)
{
	pr_info("%s", __func__);
	return 0;
}

static int connfem_dev_close(struct inode *inode, struct file *file)
{
	pr_info("%s", __func__);
	return 0;
}

static ssize_t connfem_dev_read(struct file *filp, char __user *buf,
						size_t count, loff_t *f_pos)
{
	pr_info("%s", __func__);
	return 0;
}

static ssize_t connfem_dev_write(struct file *filp, const char __user *buf,
						size_t count, loff_t *f_pos)
{
	pr_info("%s", __func__);
	return -EOPNOTSUPP;
}

static long connfem_dev_unlocked_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg)
{
	int ret = 0;
	struct connfem_ior_is_available_para para;

	switch (cmd) {
	case CONNFEM_IOR_IS_AVAILABLE:
		pr_info("CONNFEM_IOR_IS_AVAILABLE fem_type: %d, subsys: %d",
				para.fem_type, para.subsys);
		if (copy_from_user(&para,
				(struct connfem_ior_is_available_para *)arg,
				sizeof(struct connfem_ior_is_available_para))) {
			pr_info("[WARN] copy_from_user fail");
			ret = -EFAULT;
			break;
		}

		para.is_available = connfem_is_available(para.fem_type);
		if (copy_to_user((void *)arg, &para,
			sizeof(struct connfem_ior_is_available_para))) {
			ret = -EFAULT;
		}
		break;
	case CONNFEM_IOR_GET_INFO:
		break;
	default:
		pr_info("%s, unknown cmd: %d", __func__, cmd);
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long connfem_dev_compat_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg)
{
	pr_info("cmd:0x%x, arg:0x%lx, redirecting to connfem_dev_unlocked_ioctl...",
			cmd, arg);
	return connfem_dev_unlocked_ioctl(filp, cmd, arg);
}
#endif

static int connfem_plat_probe(struct platform_device *pdev)
{
	struct device_node *node;

	pr_info("%s", __func__);

	connfem_cdev_ctx.device = pdev;
	node = connfem_cdev_ctx.device->dev.of_node;

	connfem_epaelna_init(pdev);

	return 0;
}

static int connfem_plat_remove(struct platform_device *pdev)
{
	pr_info("%s", __func__);
	return 0;
}

static int __init connfem_mod_init(void)
{
	int ret = 0;

	/* Init global context */
	memset(&connfem_cdev_ctx, 0, sizeof(struct connfem_cdev_context));

	/* Platform device */
	ret = platform_driver_register(&connfem_plat_drv);
	if (ret < 0) {
		pr_info("[WARN] ConnFem platform driver registration failed: %d",
				ret);
		goto mod_init_err_skip_free;
	}

	/* Char Device: Dynamic allocate Major number */
	connfem_cdev_ctx.devId = MKDEV(connfem_major, 0);
	ret = alloc_chrdev_region(&connfem_cdev_ctx.devId, 0, CONNFEM_DEV_NUM,
							  CONNFEM_DRIVER_NAME);
	if (ret < 0) {
		pr_info("[WARN] ConnFem alloc chrdev region failed: %d", ret);
		ret = -20;
		goto mod_init_err_skip_free;
	}
	connfem_major = MAJOR(connfem_cdev_ctx.devId);
	pr_info("ConnFem DevID major %d", connfem_major);

	/* Char Device: Create class */
	connfem_cdev_ctx.class = class_create(THIS_MODULE, CONNFEM_DRIVER_NAME);
	if (IS_ERR(connfem_cdev_ctx.class)) {
		pr_info("[WARN] ConnFem create class failed");
		ret = -30;
		goto mod_init_err;
	}

	/* Char Device: Create device */
	connfem_cdev_ctx.chrdev = device_create(connfem_cdev_ctx.class, NULL,
						connfem_cdev_ctx.devId, NULL,
						CONNFEM_DRIVER_NAME);
	if (!connfem_cdev_ctx.chrdev) {
		pr_info("[WARN] ConnFem create device failed");
		ret = -40;
		goto mod_init_err;
	}

	/* Char Device: Init device */
	cdev_init(&connfem_cdev_ctx.cdev, &connfem_dev_fops);
	connfem_cdev_ctx.cdev.owner = THIS_MODULE;

	/* Char Device: Add device, visible from file system hereafter */
	ret = cdev_add(&connfem_cdev_ctx.cdev, connfem_cdev_ctx.devId,
				   CONNFEM_DEV_NUM);
	if (ret < 0) {
		pr_info("[WARN] ConnFem add device failed");
		ret = -50;
		goto mod_init_err;
	}

	pr_info("%s, ret: %d", __func__, ret);
	return ret;

mod_init_err:
	if (connfem_cdev_ctx.chrdev) {
		device_destroy(connfem_cdev_ctx.class, connfem_cdev_ctx.devId);
		connfem_cdev_ctx.chrdev = NULL;
	}

	if (connfem_cdev_ctx.class) {
		class_destroy(connfem_cdev_ctx.class);
		connfem_cdev_ctx.class = NULL;
	}

	unregister_chrdev_region(connfem_cdev_ctx.devId, CONNFEM_DEV_NUM);

mod_init_err_skip_free:
	pr_info("%s, failed: %d", __func__, ret);
	return ret;
}

static void __exit connfem_mod_exit(void)
{
	pr_info("%s", __func__);

	cdev_del(&connfem_cdev_ctx.cdev);

	if (connfem_cdev_ctx.chrdev) {
		device_destroy(connfem_cdev_ctx.class, connfem_cdev_ctx.devId);
		connfem_cdev_ctx.chrdev = NULL;
	}

	if (connfem_cdev_ctx.class) {
		class_destroy(connfem_cdev_ctx.class);
		connfem_cdev_ctx.class = NULL;
	}

	unregister_chrdev_region(connfem_cdev_ctx.devId, CONNFEM_DEV_NUM);

	platform_driver_unregister(&connfem_plat_drv);
}

module_init(connfem_mod_init);
module_exit(connfem_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Connsys FEM (Front-End-Module) Driver");
MODULE_AUTHOR("Dennis Lin <dennis.lin@mediatek.com>");
MODULE_AUTHOR("Brad Chou <brad.chou@mediatek.com>");
module_param(connfem_major, uint, 0644);

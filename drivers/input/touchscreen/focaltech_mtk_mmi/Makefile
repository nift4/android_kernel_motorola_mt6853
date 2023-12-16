ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
obj-$(CONFIG_FOCALTECH_MTK_MMI) := focaltech_mtk_mmi.o
focaltech_mtk_mmi-objs += focaltech_core.o
focaltech_mtk_mmi-objs += focaltech_ex_fun.o
focaltech_mtk_mmi-objs += focaltech_ex_mode.o
focaltech_mtk_mmi-objs += focaltech_gesture.o
focaltech_mtk_mmi-objs += focaltech_esdcheck.o
focaltech_mtk_mmi-objs += focaltech_point_report_check.o
focaltech_mtk_mmi-objs += focaltech_flash.o
focaltech_mtk_mmi-objs += focaltech_flash/focaltech_upgrade_ft3518u.o
focaltech_mtk_mmi-objs += focaltech_i2c.o

ifneq ($(filter m y,$(CONFIG_INPUT_TOUCHSCREEN_MMI)),)
       ccflags-y += -DCONFIG_INPUT_TOUCHSCREEN_MMI
       focaltech_mtk_mmi-objs += focaltech_ts_mmi.o
endif
ccflags-y += -DFOCALTECH_SENSOR_EN
#ifneq ($(filter m y,$(CONFIG_INPUT_FOCALTECH_MMI_ENABLE_PALM)),)
#   ccflags-y += -DFOCALTECH_PALM_SENSOR_EN
#endif

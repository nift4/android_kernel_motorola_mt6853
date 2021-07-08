###############################################################################
# Necessary Check

ifneq ($(KERNEL_OUT),)
    ccflags-y += -imacros $(KERNEL_OUT)/include/generated/autoconf.h
endif

# Force build fail on modpost warning
KBUILD_MODPOST_FAIL_ON_WARNINGS := y

###############################################################################
ccflags-y += -Wall
ccflags-y += -Werror

###############################################################################
MODULE_NAME := connfem
obj-m += $(MODULE_NAME).o

###############################################################################
# ConnFem Core
###############################################################################
$(MODULE_NAME)-objs += connfem_module.o
$(MODULE_NAME)-objs += connfem_api.o
$(MODULE_NAME)-objs += connfem_epaelna.o
$(MODULE_NAME)-objs += connfem_dt_parser.o
$(MODULE_NAME)-objs += connfem_dt_parser_wifi.o
$(MODULE_NAME)-objs += connfem_dt_parser_bt.o

###############################################################################
# Common_main
###############################################################################
ccflags-y += -I$(src)/include

ifneq ($(TARGET_BUILD_VARIANT), user)
    ccflags-y += -D CONNFEM_DBG=1
else
    ccflags-y += -D CONNFEM_DBG=0
endif
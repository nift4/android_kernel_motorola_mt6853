ifneq ($(filter m y,$(CONFIG_MMI_RELAY)),)
	EXTRA_CFLAGS += -DMMI_RELAY_MODULE
endif
goodix_mtk_tee-y += gf_spi_tee.o gf_spi_factory.o
obj-$(CONFIG_INPUT_GOODIX_FPS) += goodix_mtk_tee.o

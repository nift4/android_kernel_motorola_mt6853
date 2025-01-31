/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 MediaTek Inc.
 */
/*! \file   pcie_msi.c
*/

#include "gl_os.h"

#include "hif_pdma.h"

#include "precomp.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include <linux/irq.h>

irqreturn_t mtk_pci_isr_tx_data0_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(
		dev_instance, irq, PCIE_MSI_TX_DATA_BAND0);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_TX_DATA_BAND0, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_tx_data1_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(
		dev_instance, irq, PCIE_MSI_TX_DATA_BAND1);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_TX_DATA_BAND1, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_tx_free_done_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(
		dev_instance, irq, PCIE_MSI_TX_FREE_DONE);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_TX_FREE_DONE, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_rx_data0_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(
		dev_instance, irq, PCIE_MSI_RX_DATA_BAND0);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_RX_DATA_BAND0, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_rx_data1_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(
		dev_instance, irq, PCIE_MSI_RX_DATA_BAND1);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_RX_DATA_BAND1, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_rx_event_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(dev_instance, irq, PCIE_MSI_EVENT);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_EVENT, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_tx_cmd_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(dev_instance, irq, PCIE_MSI_CMD);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_CMD, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

irqreturn_t mtk_pci_isr_lump_thread(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;

	prGlueInfo = get_glue_info_isr(dev_instance, irq, PCIE_MSI_LUMP);
	if (!prGlueInfo)
		return IRQ_NONE;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	KAL_SET_BIT(PCIE_MSI_LUMP, prMsiInfo->ulEnBits);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

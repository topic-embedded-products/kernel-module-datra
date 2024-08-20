/*
 * datra-pcie.c
 *
 * Datra loadable kernel module.
 *
 * (C) Copyright 2013,2014 Topic Embedded Products B.V. (http://www.topic.nl).
 * All rights reserved.
 *
 * This file is part of kernel-module-datra.
 * kernel-module-datra is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * kernel-module-datra is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <product name>.  If not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA or see <http://www.gnu.org/licenses/>.
 *
 * You can contact Topic by electronic mail via info@topic.nl or via
 * paper mail at the following address: Postbus 440, 5680 AK Best, The Netherlands.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
# include <linux/pci-aspm.h>
#endif
#include "datra-core.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Topic Embedded Products <www.topic.nl>");
MODULE_DESCRIPTION("Driver for Topic Datra PCIe device");

#define PCI_DEVICE_ID_TOPIC_BOARD		0x7024

#ifndef PCI_VENDOR_ID_ALTERA
#	define PCI_VENDOR_ID_ALTERA		0x1172
#endif

#define DATRA_CONTROL_BAR 0
#define DATRA_PCIE_BAR 1

#define AXIBAR2PCIEBAR_0U	0x208
#define AXIBAR2PCIEBAR_0L	0x20C

static const struct pci_device_id datra_pci_ids[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_TOPIC_BOARD)},
	{PCI_DEVICE(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_TOPIC_BOARD)},
	{ /* End: all zeroes */ }
};

static const char datra_pci_name[] = "datra-pci";

static void datra_pci_write_bar_reg(void __iomem *base, unsigned int reg, u32 data)
{
	iowrite32(data, ((__iomem u8*)base) + reg);
}

static u32 datra_pci_read_bar_reg(void __iomem *base, unsigned int reg)
{
	return ioread32(((__iomem u8*)base) + reg);
}

static void datra_pci_bar_initialize(struct device *device, void __iomem *regs)
{
	u32 reg = datra_pci_read_bar_reg(regs, 0x144);

	/* Output some diagnostic link information */
	dev_info(device, "Link %s x%u %s\n",
		(reg & 1) ? "5GT/s" : "2.5GT/s", /* BIT0 = link speed */
		1 << ((reg >> 1) & 0x03),	/* BIT1..2 = number of lanes (1, 2, 4, 8) */
		(reg & BIT(11)) ? "UP" : "DOWN"); /* Uh, I don't really expect to see "down" here */

	/* We use a very simple translation: All 32-bits to address 0 */
	datra_pci_write_bar_reg(regs, AXIBAR2PCIEBAR_0U, 0);
	datra_pci_write_bar_reg(regs, AXIBAR2PCIEBAR_0L, 0);
}

static int datra_pci_probe(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{
	struct device *device = &pdev->dev;
	struct datra_dev *dev;
	int rc;

	dev_dbg(device, "%s\n", __func__);

	dev = devm_kzalloc(device, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;
	pci_set_drvdata(pdev, dev);

	rc = pcim_enable_device(pdev);
	if (rc) {
		dev_err(&pdev->dev,
			"pcim_enable_device() failed. Aborting.\n");
		return rc;
	}

	/* resource configuration */

	if (!(pci_resource_flags(pdev, DATRA_CONTROL_BAR) & IORESOURCE_MEM)) {
		dev_err(device,
			"Incorrect BAR configuration. Aborting.\n");
		return -ENODEV;
	}

	rc = pcim_iomap_regions(pdev,
		BIT(DATRA_CONTROL_BAR) | BIT(DATRA_PCIE_BAR), datra_pci_name);
	if (rc) {
		dev_err(device,
			"pcim_iomap_regions() failed. Aborting.\n");
		return rc;
	}
	dev->base = pcim_iomap_table(pdev)[DATRA_CONTROL_BAR];
	dev->mem = devm_kzalloc(device, sizeof(*dev->mem), GFP_KERNEL);
	if (!dev->mem)
		return -ENOMEM;
	dev->mem->start = pci_resource_start(pdev, DATRA_CONTROL_BAR);
	dev->mem->end = pci_resource_end(pdev, DATRA_CONTROL_BAR);
	dev->mem->flags = IORESOURCE_MEM;

	datra_pci_bar_initialize(device, pcim_iomap_table(pdev)[DATRA_PCIE_BAR]);

	pci_set_master(pdev);

	/* Set up a single MSI interrupt */
	if (pci_enable_msi(pdev)) {
		dev_err(device,
			"Failed to enable MSI interrupts. Aborting.\n");
		return -ENODEV;
	}
	dev->irq = pdev->irq;

	/* pci_set_dma_mask removed in 5.17 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
	rc = dma_set_mask_and_coherent(device, DMA_BIT_MASK(32));
	if (rc) {
		dev_err(device, "Failed to set DMA mask. Aborting.\n");
		return rc;
	}
#else
	if (pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		dev_err(device, "Failed to set DMA mask. Aborting.\n");
		return -ENODEV;
	}
#endif

	return datra_core_probe(device, dev);
}

static void datra_pci_remove(struct pci_dev *pdev)
{
	struct device *device = &pdev->dev;
	struct datra_dev *dev = pci_get_drvdata(pdev);

	datra_core_remove(device, dev);
}

MODULE_DEVICE_TABLE(pci, datra_pci_ids);

static struct pci_driver datra_pci_driver = {
	.name = datra_pci_name,
	.id_table = datra_pci_ids,
	.probe = datra_pci_probe,
	.remove = datra_pci_remove,
};

module_pci_driver(datra_pci_driver);

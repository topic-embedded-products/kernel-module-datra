/*
 * datra-of.c
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

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/nvmem-consumer.h>
#include <linux/slab.h>
#include "datra-core.h"
#include "datra.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Topic Embedded Products <www.topic.nl>");

static int datra_of_nvmem_license(struct datra_dev *dev, struct device_node *np)
{
	struct nvmem_cell *cell;
	const void *data;
	size_t len;
	int ret;

	cell = of_nvmem_cell_get(np, "license");
	if (IS_ERR(cell))
		return PTR_ERR(cell);

	data = nvmem_cell_read(cell, &len);

	nvmem_cell_put(cell);

	if (IS_ERR(data))
		return PTR_ERR(data);

	if (len < 8)
		ret = -EINVAL;
	else
		datra_core_apply_license(dev, data);

	kfree(data);

	return ret;
}

static int datra_probe(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;
	struct datra_dev *dev;

	dev = devm_kzalloc(device, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;
	dev_set_drvdata(device, dev);

	/* resource configuration */
	dev->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev->base = devm_ioremap_resource(device, dev->mem);
	if (IS_ERR(dev->base)) {
		dev_err(device, "Failed to map device memory\n");
		return PTR_ERR(dev->base);
	}
	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0) {
		dev_err(device, "IRQ resource missing\n");
		return -ENOENT;
	}

	datra_of_nvmem_license(dev, device->of_node);

	return datra_core_probe(device, dev);
}

static int datra_remove(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;
	struct datra_dev *dev;
	
	dev = dev_get_drvdata(device);
	if (!dev)
		return -ENODEV;

	return datra_core_remove(device, dev);
}


static const struct of_device_id datra_ids[] = {
	{ .compatible = "topic,datra-1.00.a" },
	{ },
};
MODULE_DEVICE_TABLE(of, datra_ids);

static struct platform_driver datra_driver = {
	.driver = {
		.name = "datra",
		.owner = THIS_MODULE,
		.of_match_table = datra_ids,
	},
	.probe = datra_probe,
	.remove = datra_remove,
};
module_platform_driver(datra_driver);

/*
 * datra.h
 *
 * Datra loadable kernel module.
 *
 * (C) Copyright 2013-2015 Topic Embedded Products B.V. (http://www.topic.nl).
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
 * paper mail at the following address:
 * Postbus 440, 5680 AK Best, The Netherlands.
 */


/* ioctl values for datractl device, set and get routing tables */
struct datra_route_item_t {
	unsigned char dstFifo; /* LSB */
	unsigned char dstNode;
	unsigned char srcFifo;
	unsigned char srcNode; /* MSB */
};

struct datra_route_t  {
	unsigned int n_routes;
	struct datra_route_item_t* proutes;
};

struct datra_buffer_block_alloc_req {
	__u32 size;	/* Size of each buffer (will be page aligned) */
	__u32 count;	/* Number of buffers */
};

struct datra_buffer_block {
	__u32 id;	/* 0-based index of the buffer */
	__u32 offset;	/* Location of data in memory map */
	__u32 size;	/* Size of buffer */
	__u32 bytes_used; /* How much actually is in use */
	__u16 user_signal; /* User signals (framing) either way */
	__u16 state; /* Who's owner of the buffer */
};

/* This STANDALONE mode is not supported anymore */
#define DATRA_DMA_MODE_STANDALONE 0
/* (default) Copies data from userspace into a kernel buffer and
 * vice versa. */
#define DATRA_DMA_MODE_RINGBUFFER_BOUNCE	1
/* Blockwise data transfers, using coherent memory. This will result in
 * slow non-cached memory being used when hardware coherency is not
 * available, but it is the fastest mode. */
#define DATRA_DMA_MODE_BLOCK_COHERENT	2
/* Blockwise data transfers, using  streaming DMA into cachable memory.
 * Managing the cache may cost more than actually copying the data. */
#define DATRA_DMA_MODE_BLOCK_STREAMING	3

struct datra_dma_configuration_req {
	__u32 mode;	/* One of DATRA_DMA_MODE.. */
	__u32 size;	/* Size of each buffer (will be page aligned) */
	__u32 count;	/* Number of buffers */
};

#define DATRA_IOC_MAGIC	'd'
#define DATRA_IOC_ROUTE_CLEAR	0x00
#define DATRA_IOC_ROUTE_SET	0x01
#define DATRA_IOC_ROUTE_GET	0x02
#define DATRA_IOC_ROUTE_TELL	0x03
#define DATRA_IOC_ROUTE_DELETE	0x04
#define DATRA_IOC_ROUTE_TELL_TO_LOGIC	0x05
#define DATRA_IOC_ROUTE_TELL_FROM_LOGIC	0x06
#define DATRA_IOC_ROUTE_QUERY_ID	0x07

#define DATRA_IOC_BACKPLANE_STATUS	0x08
#define DATRA_IOC_BACKPLANE_DISABLE	0x09
#define DATRA_IOC_BACKPLANE_ENABLE	0x0A

#define DATRA_IOC_ICAP_INDEX_QUERY	0x0B

#define DATRA_IOC_RESET_FIFO_WRITE	0x0C
#define DATRA_IOC_RESET_FIFO_READ	0x0D

#define DATRA_IOC_TRESHOLD_QUERY	0x10
#define DATRA_IOC_TRESHOLD_TELL	0x11

#define DATRA_IOC_USERSIGNAL_QUERY	0x12
#define DATRA_IOC_USERSIGNAL_TELL	0x13

#define DATRA_IOC_DMA_RECONFIGURE	0x1F
#define DATRA_IOC_DMABLOCK_ALLOC	0x20
#define DATRA_IOC_DMABLOCK_FREE 	0x21
#define DATRA_IOC_DMABLOCK_QUERY	0x22
#define DATRA_IOC_DMABLOCK_ENQUEUE	0x23
#define DATRA_IOC_DMABLOCK_DEQUEUE	0x24

#define DATRA_IOC_LICENSE_KEY	0x30
#define DATRA_IOC_STATIC_ID	0x31

#define DATRA_IOC_ROUTE_SINGLE_DELETE 0x32

#define DATRA_IOC_DEVICE_ID	0x33
#define DATRA_IOC_LICENSE_INFO	0x34

/* S means "Set" through a ptr,
 * T means "Tell", sets directly
 * G means "Get" through a ptr
 * Q means "Query", return value */

/* Delete all existing routes */
#define DATRA_IOCROUTE_CLEAR	_IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_CLEAR)
/* Define a set of routes, to be added to the currently active set */
#define DATRA_IOCSROUTE   _IOW(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_SET, struct datra_route_t)
/* Get the currently active routes. Returns number of entries. */
#define DATRA_IOCGROUTE   _IOR(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_GET, struct datra_route_t)
/* Add a single route. Argument is a datra_route_item_t cast to integer */
#define DATRA_IOCTROUTE   _IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_TELL)
/* Remove routes to a node. Argument is a integer node number. */
#define DATRA_IOCTROUTE_DELETE   _IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_DELETE)
/* Remove single route. Argument is a datra_route_item_t cast to integer */
#define DATRA_IOCTROUTE_SINGLE_DELETE   _IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_SINGLE_DELETE)

/* Add a route from "this" dma or cpu node to another node. The argument
 * is an integer of destination node | fifo << 8 */
#define DATRA_IOCTROUTE_TELL_TO_LOGIC	_IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_TELL_TO_LOGIC)
/* Add a route from another node into "this" dma or cpu node. Argument
 * is an integer of source node | fifo << 8 */
#define DATRA_IOCTROUTE_TELL_FROM_LOGIC	_IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_TELL_FROM_LOGIC)
/* Get the node number and fifo (if applicable) for this cpu or dma
 * node. Returns an integer of node | fifo << 8 */
#define DATRA_IOCQROUTE_QUERY_ID	_IO(DATRA_IOC_MAGIC, DATRA_IOC_ROUTE_QUERY_ID)

/* Get backplane status. When called on control node, returns a bit mask where 0=CPU and
 * 1=first HDL node and so on. When called on config node, returns the status for only
 * that node, 0=disabled, non-zero is enabled */
#define DATRA_IOCQBACKPLANE_STATUS   _IO(DATRA_IOC_MAGIC, DATRA_IOC_BACKPLANE_STATUS)
/* Enable or disable backplane status. Disable is required when the logic is active and
 * you want to replace a node using partial configuration. Operations are atomic. */
#define DATRA_IOCTBACKPLANE_ENABLE   _IO(DATRA_IOC_MAGIC, DATRA_IOC_BACKPLANE_ENABLE)
#define DATRA_IOCTBACKPLANE_DISABLE  _IO(DATRA_IOC_MAGIC, DATRA_IOC_BACKPLANE_DISABLE)
/* Get ICAP index. Returns negative ENODEV if no ICAP available */
#define DATRA_IOCQICAP_INDEX	_IO(DATRA_IOC_MAGIC, DATRA_IOC_ICAP_INDEX_QUERY)
/* Set the thresholds for "writeable" or "readable" on a CPU node fifo. Allows
 * tuning for low latency or reduced interrupt rate. */
#define DATRA_IOCQTRESHOLD   _IO(DATRA_IOC_MAGIC, DATRA_IOC_TRESHOLD_QUERY)
#define DATRA_IOCTTRESHOLD   _IO(DATRA_IOC_MAGIC, DATRA_IOC_TRESHOLD_TELL)
/* Reset FIFO data (i.e. throw it away). Can be applied to config
 * nodes to reset its incoming fifos (argument is bitmask for queues to
 * reset), or to a CPU read/write fifo (argument ignored). */
#define DATRA_IOCRESET_FIFO_WRITE	_IO(DATRA_IOC_MAGIC, DATRA_IOC_RESET_FIFO_WRITE)
#define DATRA_IOCRESET_FIFO_READ	_IO(DATRA_IOC_MAGIC, DATRA_IOC_RESET_FIFO_READ)
/* Set or get user signal bits. These are the upper 4 bits of Datra data
 * that aren't part of the actual data, but control the flow. */
#define DATRA_IOCQUSERSIGNAL   _IO(DATRA_IOC_MAGIC, DATRA_IOC_USERSIGNAL_QUERY)
#define DATRA_IOCTUSERSIGNAL   _IO(DATRA_IOC_MAGIC, DATRA_IOC_USERSIGNAL_TELL)

/* DMA configuration */
#define DATRA_IOCDMA_RECONFIGURE _IOWR(DATRA_IOC_MAGIC, DATRA_IOC_DMA_RECONFIGURE, struct datra_dma_configuration_req)

/* Datra's IIO-alike DMA block interface */
#define DATRA_IOCDMABLOCK_ALLOC	_IOWR(DATRA_IOC_MAGIC, DATRA_IOC_DMABLOCK_ALLOC, struct datra_buffer_block_alloc_req)
#define DATRA_IOCDMABLOCK_FREE 	_IO(DATRA_IOC_MAGIC, DATRA_IOC_DMABLOCK_FREE)
#define DATRA_IOCDMABLOCK_QUERY	_IOWR(DATRA_IOC_MAGIC, DATRA_IOC_DMABLOCK_QUERY, struct datra_buffer_block)
#define DATRA_IOCDMABLOCK_ENQUEUE	_IOWR(DATRA_IOC_MAGIC, DATRA_IOC_DMABLOCK_ENQUEUE, struct datra_buffer_block)
#define DATRA_IOCDMABLOCK_DEQUEUE	_IOWR(DATRA_IOC_MAGIC, DATRA_IOC_DMABLOCK_DEQUEUE, struct datra_buffer_block)

/* Read or write a 64-bit license key */
#define DATRA_IOCSLICENSE_KEY   _IOW(DATRA_IOC_MAGIC, DATRA_IOC_LICENSE_KEY, unsigned long long)
#define DATRA_IOCGLICENSE_KEY   _IOR(DATRA_IOC_MAGIC, DATRA_IOC_LICENSE_KEY, unsigned long long)

/* Retrieve the DEVICE_ID for requesting a license key for a device */
#define DATRA_IOCGDEVICE_ID     _IOR(DATRA_IOC_MAGIC, DATRA_IOC_DEVICE_ID, unsigned long long)

/* Retrieve license info from logic: BIT(0) indicates "license invalid" */
#define DATRA_IOCQLICENSE_INFO  _IO(DATRA_IOC_MAGIC, DATRA_IOC_LICENSE_INFO)

/* Retrieve static ID (to match against partials) */
#define DATRA_IOCGSTATIC_ID   _IOR(DATRA_IOC_MAGIC, DATRA_IOC_STATIC_ID, unsigned int)

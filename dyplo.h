/* Memory range for a processing block is 64k */
#define DYPLO_CONFIG_SIZE	(64*1024)
/* Each FIFO occupies 256 words address range */
#define DYPLO_FIFO_MEMORY_SIZE (4*256)

/* memory map offsets */
#define DYPLO_REG_ID	0x00

#define DYPLO_REG_ID_MASK_VENDOR	0xFF000000
#define DYPLO_REG_ID_MASK_PRODUCT	0x00FF0000
#define DYPLO_REG_ID_MASK_VENDOR_PRODUCT	(DYPLO_REG_ID_MASK_VENDOR|DYPLO_REG_ID_MASK_PRODUCT)

#define DYPLO_REG_ID_VENDOR_TOPIC	0x01000000
#define DYPLO_REG_ID_PRODUCT_TOPIC_CONTROL	(DYPLO_REG_ID_VENDOR_TOPIC | 0x00010000)
#define DYPLO_REG_ID_PRODUCT_TOPIC_CPU	(DYPLO_REG_ID_VENDOR_TOPIC | 0x00020000)

/* Size of the global configuration map for each node */
#define DYPLO_NODE_REG_SIZE	0x800
#define DYPLO_REG_NODE_ID	0x00

#define DYPLO_REG_BACKPLANE_ENABLE_STATUS	0x10
#define DYPLO_REG_BACKPLANE_ENABLE_SET	0x14
#define DYPLO_REG_BACKPLANE_ENABLE_CLR	0x18

/* Specific layout of the CPU/PL communication node */
#define DYPLO_REG_FIFO_WRITE_IRQ_MASK	0x20
#define DYPLO_REG_FIFO_WRITE_IRQ_STATUS	0x24
#define DYPLO_REG_FIFO_WRITE_IRQ_SET	0x28
#define DYPLO_REG_FIFO_WRITE_IRQ_CLR	0x2C
#define DYPLO_REG_FIFO_READ_IRQ_MASK	0x30
#define DYPLO_REG_FIFO_READ_IRQ_STATUS	0x34
#define DYPLO_REG_FIFO_READ_IRQ_SET	0x38
#define DYPLO_REG_FIFO_READ_IRQ_CLR	0x3C

/* Read level threshold */
#define DYPLO_REG_FIFO_READ_THD_BASE	0x100
/* Actual fill level */
#define DYPLO_REG_FIFO_READ_LEVEL_BASE	0x180
/* Base address of the source registers */
#define DYPLO_REG_FIFO_WRITE_SOURCE_BASE	0x200
/* Write level threshold */
#define DYPLO_REG_FIFO_WRITE_THD_BASE	0x280
/* Actual fill level */
#define DYPLO_REG_FIFO_WRITE_LEVEL_BASE	0x300

/* Queue sizes in words */
#define DYPLO_FIFO_WRITE_SIZE	255
#define DYPLO_FIFO_READ_SIZE	1023

#define DYPLO_NUMBER_OF_CPU_NODE_FIFOS	32
#define DYPLO_NUMBER_OF_OTHER_NODE_FIFOS	4

/* Hack: Write with burst doesn't work, limit to <32 bytes per call */
#define DYPLO_FIFO_WRITE_MAX_BURST_SIZE	DYPLO_FIFO_MEMORY_SIZE
/* Reading does not suffer from this problem it appears */
#define DYPLO_FIFO_READ_MAX_BURST_SIZE DYPLO_FIFO_MEMORY_SIZE

/* ioctl values for dyploctl device, set and get routing tables */
struct dyplo_route_item_t {
	unsigned char dstFifo; /* LSB */
	unsigned char dstNode;
	unsigned char srcFifo;
	unsigned char srcNode; /* MSB */
};

struct dyplo_route_t  {
	unsigned int n_routes;
	struct dyplo_route_item_t* proutes;
};


#define DYPLO_IOC_MAGIC	'd'
#define DYPLO_IOC_ROUTE_CLEAR	0x00
#define DYPLO_IOC_ROUTE_SET	0x01
#define DYPLO_IOC_ROUTE_GET	0x02
#define DYPLO_IOC_ROUTE_TELL	0x03
#define DYPLO_IOC_ROUTE_DELETE	0x04

#define DYPLO_IOC_BACKPLANE_STATUS	0x08
#define DYPLO_IOC_BACKPLANE_DISABLE	0x09
#define DYPLO_IOC_BACKPLANE_ENABLE	0x0A

/* S means "Set" through a ptr,
 * T means "Tell", sets directly
 * G means "Get" through a ptr
 * Q means "Query", return value */

/* Delete all existing routes */
#define DYPLO_IOCROUTE_CLEAR	_IO(DYPLO_IOC_MAGIC, DYPLO_IOC_ROUTE_CLEAR)
/* Define a set of routes, to be added to the currently active set */
#define DYPLO_IOCSROUTE   _IOW(DYPLO_IOC_MAGIC, DYPLO_IOC_ROUTE_SET, struct dyplo_route_t)
/* Get the currently active routes. Returns number of entries. */
#define DYPLO_IOCGROUTE   _IOR(DYPLO_IOC_MAGIC, DYPLO_IOC_ROUTE_GET, struct dyplo_route_t)
/* Add a single route. Argument is a dyplo_route_item_t cast to integer */
#define DYPLO_IOCTROUTE   _IO(DYPLO_IOC_MAGIC, DYPLO_IOC_ROUTE_TELL)
/* Remove routes to a node. Argument is a integer node number. */
#define DYPLO_IOCTROUTE_DELETE   _IO(DYPLO_IOC_MAGIC, DYPLO_IOC_ROUTE_DELETE)
/* Get backplane status. When called on control node, returns a bit mask where 0=CPU and
 * 1=first HDL node and so on. When called on config node, returns the status for only
 * that node, 0=disabled, non-zero is enabled */
#define DYPLO_IOCQBACKPLANE_STATUS   _IO(DYPLO_IOC_MAGIC, DYPLO_IOC_BACKPLANE_STATUS)
/* Enable or disable backplane status. Disable is required when the logic is active and
 * you want to replace a node using partial configuration. Operations are atomic. */
#define DYPLO_IOCTBACKPLANE_ENABLE   _IO(DYPLO_IOC_MAGIC, DYPLO_IOC_BACKPLANE_ENABLE)
#define DYPLO_IOCTBACKPLANE_DISABLE  _IO(DYPLO_IOC_MAGIC, DYPLO_IOC_BACKPLANE_DISABLE)

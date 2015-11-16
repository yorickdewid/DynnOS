#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kbase/ios.h>
#include <kmod/io/io.h>

static struct io_device ioDevice;

static unsigned int io_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	char *b = (char *)buffer;

	unsigned int i = 0;

	for(; count; count--, i++)
	{
		b[i] = io_inb(offset);
	}

	return i;
}

static unsigned int io_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	char *b = (char *)buffer;

	unsigned int i = 0;

	for(; count; count--, i++)
	{
		io_outb(offset, b[i]);
	}

	return i;
}

void io_init()
{
	strcpy(ioDevice.base.node.name, "io");
	ioDevice.base.node.length = 0;
	ioDevice.base.node.read = io_node_read;
	ioDevice.base.node.write = io_node_write;

	struct vfs_node *devNode = vfs_find_root("/dev");
	devNode->write(devNode, devNode->length, 1, &ioDevice.base.node);

	modules_register_device(MODULES_DEVICE_TYPE_IO, &ioDevice.base);
}


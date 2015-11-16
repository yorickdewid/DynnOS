#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kbase/ios.h>
#include <kmod/vga/vga.h>

static struct vga_device vgaDevice;
static unsigned char vgaFbColor;

static unsigned int vga_read_framebuffer(char *buffer, unsigned int count, unsigned int offset)
{
	unsigned int i;
	unsigned int j = 0;

	for(i = offset; i < offset + count; i++, j++)
	{
		if(i == VGA_FB_SIZE)
		{
			return j;
		}

		memcpy(buffer + j, (void *)(VGA_FB_ADDRESS + i * 2), 1);
	}

	return j;
}

static unsigned int vga_fb_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	return vgaDevice.read_framebuffer(buffer, count, offset);
}

static unsigned int vga_write_framebuffer(char *buffer, unsigned int count, unsigned int offset)
{
	unsigned int i;
	unsigned int j = 0;

	for(i = offset; i < offset + count; i++, j++)
	{
		if(i == VGA_FB_SIZE)
		{
			return j;
		}

		memcpy((void *)(VGA_FB_ADDRESS + i * 2), buffer + j, 1);
		memset((void *)(VGA_FB_ADDRESS + i * 2 + 1), vgaFbColor, 1);
	}

	return j;
}

static unsigned int vga_fb_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	return vgaDevice.write_framebuffer(buffer, count, offset);
}

static unsigned int vga_fb_color_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	if(count != 1)
	{
		return 0;
	}

	((char *)buffer)[0] = vgaFbColor;

	return 1;
}

static unsigned int vga_fb_color_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	if(count != 1)
	{
		return 0;
	}

	vgaFbColor = ((char *)buffer)[0];

	return 1;
}

static void vga_set_cursor_offset(unsigned short offset)
{
	io_outb(0x3D4, 14);
	io_outb(0x3D5, offset >> 8);
	io_outb(0x3D4, 15);
	io_outb(0x3D5, offset);
}

static unsigned int vga_fb_cursor_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	if(count != 1)
	{
		return 0;
	}

	short position = ((short *)buffer)[0];

	vgaDevice.set_cursor_offset(position);

	return 1;
}

void vga_init()
{
	vgaDevice.read_framebuffer = vga_read_framebuffer;
	vgaDevice.write_framebuffer = vga_write_framebuffer;
	vgaDevice.set_cursor_offset = vga_set_cursor_offset;

	strcpy(vgaDevice.nodeFramebuffer.name, "vga_fb");
	vgaDevice.nodeFramebuffer.length = VGA_FB_SIZE;
	vgaDevice.nodeFramebuffer.read = vga_fb_node_read;
	vgaDevice.nodeFramebuffer.write = vga_fb_node_write;

	strcpy(vgaDevice.nodeCursorColor.name, "vga_fb_color");
	vgaDevice.nodeCursorColor.length = 1;
	vgaDevice.nodeCursorColor.read = vga_fb_color_node_read;
	vgaDevice.nodeCursorColor.write = vga_fb_color_node_write;

	strcpy(vgaDevice.nodeCursorOffset.name, "vga_fb_cursor");
	vgaDevice.nodeCursorOffset.length = 1;
	vgaDevice.nodeCursorOffset.write = vga_fb_cursor_node_write;

	struct vfs_node *devNode = vfs_find_root("/dev");
	devNode->write(devNode, devNode->length, 1, &vgaDevice.nodeFramebuffer);
	devNode->write(devNode, devNode->length, 1, &vgaDevice.nodeCursorColor);
	devNode->write(devNode, devNode->length, 1, &vgaDevice.nodeCursorOffset);

	modules_register_device(MODULES_DEVICE_TYPE_VGA, &vgaDevice.base);
}


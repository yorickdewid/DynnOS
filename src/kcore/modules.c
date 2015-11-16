/*
*  DynnOS Operating System
*  Project DynnOS, Quenza
*  Copyright (c) 2010-2012
*
*  All files, published and unpublished, are property of
*  the DynnOS poject. Source is reserved for official
*  developers only. Project developers are named in the
*  next section.
*
*  Dinux, Team17, team1717
*
*  Description:	-
*  Date:		17-01-2012
*  Timestamp:		18-01-2012
*/

#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/file.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>

#include <kmod/elf/elf.h>
#include <kmod/tty/tty.h>
#include <kmod/io/io.h>
#include <kmod/ata/ata.h>
#include <kmod/kbd/kbd.h>
#include <kmod/pit/pit.h>
#include <kmod/rtc/rtc.h>
#include <kmod/serial/serial.h>
#include <kmod/vga/vga.h>

static struct vfs_filesystem modulesFilesystem;

static struct modules_bus *modulesBusses[32];

static struct vfs_node *modulesEntries[32];
static struct vfs_node modulesRoot;

void modules_register_bus(unsigned int type, struct modules_bus *bus)
{
	unsigned int i;

	for(i = 0; i < 32; i++)
	{
		if(!modulesBusses[i])
		{
			modulesBusses[i] = bus;
			return;
		}
	}
}

void modules_register_device(unsigned int type, struct modules_device *device)
{
}

void modules_register_driver(unsigned int type, struct modules_driver *driver)
{
}

static struct vfs_node *modules_node_walk(struct vfs_node *node, unsigned int index)
{
	return (index < node->length) ? modulesEntries[index] : 0;
}

static unsigned int modules_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	modulesEntries[offset] = (struct vfs_node *)buffer;
	node->length++;

	return count;
}

static unsigned int modules_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	unsigned int i;

	memset(buffer, 0, 1);

	for(i = 0; i < node->length; i++)
	{
		strcat(buffer, modulesEntries[i]->name);
		strcat(buffer, "\n");
	}

	return strlen(buffer);
}

struct vfs_node *modules_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{
	unsigned int i;

	for(i = 0; modulesEntries[i]; i++)
	{
		if(!memcmp(path, modulesEntries[i]->name, strlen(modulesEntries[i]->name)))
		{
			return modulesEntries[i];
		}
	}
	return 0;
}

static void modules_init_devices()
{
	io_init();
	vga_init();
	pit_init();
	kbd_init();
	rtc_init();
	ata_init();
	serial_init();
	tty_init();
	elf_init();
}

void modules_init()
{
	strcpy(modulesRoot.name, "dev");
	modulesRoot.length = 0;
	modulesRoot.walk = modules_node_walk;
	modulesRoot.write = modules_node_write;
	modulesRoot.read = modules_node_read;

	struct vfs_node *rootNode = vfs_get_root();
	rootNode->write(rootNode, rootNode->length, 1, &modulesRoot);

	strcpy(modulesFilesystem.name, "sysfs");
	modulesFilesystem.root = &modulesRoot;
	modulesFilesystem.lookup = modules_filesystem_lookup;
	vfs_register_filesystem(&modulesFilesystem);

	modules_init_devices();
}


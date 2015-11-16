#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcommon/tar.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kcore/initrd.h>

static struct initrd_filesystem initrdFilesystem;
static struct vfs_node initrdRoot;

static unsigned int initrd_file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	struct tar_header *header = initrdFilesystem.headers[node->id];

	if(offset > node->length)
	{
		return 0;
	}

	if(offset + count > node->length)
	{
		count = node->length - offset;
	}

	unsigned int address = (unsigned int)header + TAR_BLOCK_SIZE;

	memcpy(buffer, (unsigned char *)(address + offset), count);

	return count;
}

static unsigned int initrd_file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	struct tar_header *header = initrdFilesystem.headers[node->id];

	if(offset > node->length)
	{
		return 0;
	}

	if(offset + count > node->length)
	{
		count = node->length - offset;
	}

	unsigned int address = (unsigned int)header + TAR_BLOCK_SIZE;

	memcpy((unsigned char *)(address + offset), buffer, count);

	return count;
}

static unsigned int initrd_get_file_size(const char *in)
{
	unsigned int size = 0;
	unsigned int j;
	unsigned int count = 1;

	for(j = 11; j > 0; j--, count *= 8)
	{
		size += ((in[j - 1] - '0') * count);
	}

	return size;
}

static unsigned int initrd_parse(unsigned int address)
{
	unsigned int i;

	for(i = 0; ; i++)
	{
		struct tar_header *header = (struct tar_header *)address;

		if(header->name[0] == '\0')
		{
			break;
		}

		unsigned int size = initrd_get_file_size(header->size);

		initrdFilesystem.headers[i] = header;

		address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

		if(size % TAR_BLOCK_SIZE)
		{
			address += TAR_BLOCK_SIZE;
		}
	}

	return i;
}

static unsigned int initrd_root_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	memset(buffer, 0, 1);
	unsigned int i;

	for(i = 0; i < node->length; i++)
	{
		strcat(buffer, initrdFilesystem.nodes[i].name);
		strcat(buffer, "\n");
	}

	return strlen(buffer);
}

static struct vfs_node *initrd_root_walk(struct vfs_node *node, unsigned int index)
{
	return (index < node->length) ? &initrdFilesystem.nodes[index] : 0;
}

static void initrd_create_nodes(unsigned int numEntries)
{
	unsigned int i;

	for(i = 0; i < numEntries; i++)
	{
		struct tar_header *header = initrdFilesystem.headers[i];

		unsigned int size = initrd_get_file_size(header->size);
		unsigned int start = strnidx(header->name, '/', (header->typeflag[0] == TAR_FILETYPE_DIR) ? 1 : 0) + 1;

		struct vfs_node *initrdFileNode = &initrdFilesystem.nodes[i];
		strcpy(initrdFileNode->name, header->name + start);
		initrdFileNode->id = i;
		initrdFileNode->length = size;

		if(header->typeflag[0] == TAR_FILETYPE_DIR)
		{
			memrplc(initrdFileNode->name, '/', '\0', strlen(initrdFileNode->name));
		}

		initrdFileNode->read = initrd_file_read;
		initrdFileNode->write = initrd_file_write;
	}
}

struct vfs_node *initrd_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{
	unsigned int i;

	for(i = 0; strlen(initrdFilesystem.nodes[i].name); i++)
	{
		if(!memcmp(path, initrdFilesystem.nodes[i].name, strlen(initrdFilesystem.nodes[i].name)))
		{
			return &initrdFilesystem.nodes[i];
		}
	}

	return 0;
}

void initrd_init(unsigned int *address)
{
	unsigned int numEntries = initrd_parse(*address);
	initrd_create_nodes(numEntries);

	strcpy(initrdRoot.name, "initrd");
	initrdRoot.length = numEntries;
	initrdRoot.read = initrd_root_read;
	initrdRoot.walk = initrd_root_walk;

	//TODO remove
	struct vfs_node *rootNode = vfs_get_root();
	rootNode->write(rootNode, rootNode->length, 1, &initrdRoot);

	strcpy(initrdFilesystem.base.name, "tarfs");
	initrdFilesystem.base.root = &initrdRoot;
	initrdFilesystem.base.lookup = initrd_filesystem_lookup;
	vfs_register_filesystem(&initrdFilesystem.base);
}


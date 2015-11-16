#include <kconf/config.h>
#include <kcommon/file.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>

static struct vfs_node vfsNodes[256];
static struct vfs_descriptor vfsOpenTable[256];
static unsigned int vfsNodesCount;

static struct vfs_node *vfsRootEntries[64];

static struct vfs_filesystem *vfsFilesystems[8];
static struct vfs_filesystem vfsFilesystem;
static struct vfs_node vfsRoot;

void vfs_register_filesystem(struct vfs_filesystem *filesystem)
{
	unsigned int i;

	for(i = 0; i < 8; i++)
	{
		if(!vfsFilesystems[i])
		{
			vfsFilesystems[i] = filesystem;

			return;
		}
	}
}

unsigned int vfs_open(char *name)
{
	struct vfs_node *node = vfs_find_root(name);

	if(!node)
	{
		return -1;
	}

	unsigned int i;

	for(i = 0; i < 256; i++)
	{
		if(!vfsOpenTable[i].node)
		{
			vfsOpenTable[i].node = node;
			vfsOpenTable[i].permissions = 0;

			return i;
		}
	}

	return -1;
}

void vfs_close(unsigned int index)
{
	memset((void *)&vfsOpenTable[index], 0, sizeof (struct vfs_descriptor));
}

struct vfs_node *vfs_find(struct vfs_node *node, char *path)
{
	unsigned int index = stridx(path, '/', 0);
	unsigned int length = strlen(path);

	if(!index)
	{
		return node;
	}

	struct vfs_node *current;
	unsigned int i;

	for(i = 0; (current = node->walk(node, i)); i++)
	{
		unsigned int count = strlen(current->name);

		if(index > count)
		{
			count = index;
		}

		if(!memcmp(path, current->name, count))
		{
			return (index == length) ? current : vfs_find(current, path + index + 1);
		}
	}

	return 0;
}

struct vfs_node *vfs_find_root(char *path)
{
	struct vfs_node *t = vfsFilesystem.lookup(&vfsFilesystem, path);

/*
    return t;

    if (t)
    {

        struct vfs_node *out = vfs_get(1);

        if (out)
        {

        out->write(out, 0, 7, "FOUND: ");
        out->write(out, 0, string_length(t->name), t->name);
        out->write(out, 0, 1, "\n");

        }

    }
*/

	return vfs_find(vfs_get_root(), path + 1);
}

struct vfs_node *vfs_get(unsigned int index)
{
	return vfsOpenTable[index].node;
}

struct vfs_node *vfs_get_root()
{
	return &vfsRoot;
}

static struct vfs_node *file_node_walk(struct vfs_node *node, unsigned int index)
{
	return (index < node->length) ? vfsRootEntries[index] : 0;
}

static unsigned int file_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	vfsRootEntries[node->length] = (struct vfs_node *)buffer;
	node->length++;

	return count;
}

static unsigned int file_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	memset(buffer, 0, 1);
	unsigned int i;

	for(i = 0; i < node->length; i++)
	{
		strcat(buffer, vfsRootEntries[i]->name);
		strcat(buffer, "\n");
	}

	return strlen(buffer);
}

struct vfs_node *vfs_add_node(char *name, unsigned int length)
{
	struct vfs_node *node = &vfsNodes[vfsNodesCount];
	memset(node, 0, sizeof (struct vfs_node));
	strcpy(node->name, name);
	node->length = length;

	vfsNodesCount++;

	return node;
}

struct vfs_node *vfs_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{
	if(strlen(path) == 1 && path[0] == '/')
	{
		return vfsFilesystem.root;
	}

	unsigned int i;

	for(i = 0; vfsFilesystems[i]; i++)
	{
		if(!memcmp(path + 1, vfsFilesystems[i]->root->name, strlen(vfsFilesystems[i]->root->name)))
		{
			if(path[strlen(vfsFilesystems[i]->root->name) + 1] == '/')
			{
				return vfsFilesystems[i]->lookup(vfsFilesystems[i], path + strlen(vfsFilesystems[i]->root->name) + 2);
			}else{
				return vfsFilesystems[i]->root;
			}
		}
	}

	return 0;
}

void vfs_init()
{
	strcpy(vfsRoot.name, "root");
	vfsRoot.length = 0;
	vfsRoot.walk = file_node_walk;
	vfsRoot.read = file_node_read;
	vfsRoot.write = file_node_write;

	strcpy(vfsFilesystem.name, "rootfs");
	vfsFilesystem.root = &vfsRoot;
	vfsFilesystem.lookup = vfs_filesystem_lookup;
	vfs_register_filesystem(&vfsFilesystem);
}


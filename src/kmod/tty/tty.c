#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kmod/tty/tty.h>

static struct tty_device ttyDevice;

static struct vfs_node *ttyVgaNode;
static struct vfs_node *ttyVgaColorNode;
static struct vfs_node *ttyVgaCursorNode;

static char ttyCwd[256];

static void tty_scroll()
{
	char buffer[TTY_CHARACTER_SIZE];

	ttyVgaNode->read(ttyVgaNode, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);
	memset(buffer + TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, ' ', TTY_CHARACTER_WIDTH);
	ttyVgaNode->write(ttyVgaNode, 0, TTY_CHARACTER_SIZE, buffer);

	ttyDevice.cursorOffset -= TTY_CHARACTER_WIDTH;
}

static void tty_putc(char c)
{
	if(c == '\b')
	{
		ttyDevice.cursorOffset--;
	}
	else if(c == '\t')
	{
		ttyDevice.cursorOffset = (ttyDevice.cursorOffset + 8) & ~(8 - 1);
	}
	else if(c == '\r')
	{
		ttyDevice.cursorOffset -= (ttyDevice.cursorOffset % TTY_CHARACTER_WIDTH);
	}
	else if(c == '\n')
	{
		ttyDevice.cursorOffset += TTY_CHARACTER_WIDTH - (ttyDevice.cursorOffset % TTY_CHARACTER_WIDTH);
	}
	else if (c >= ' ')
	{
		ttyVgaNode->write(ttyVgaNode, ttyDevice.cursorOffset, 1, &c);
		ttyDevice.cursorOffset++;
	}

	if(ttyDevice.cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
	{
		tty_scroll();
	}
}

static void tty_vga_clear()
{
	char c = ' ';
	int i;

	for(i = 0; i < TTY_CHARACTER_SIZE; i++)
	{
		ttyVgaNode->write(ttyVgaNode, i, 1, &c);
	}
}

static unsigned int tty_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	unsigned int i;
	unsigned int j = 0;

	for(i = offset; i < offset + count; i++, j++)
	{
		tty_putc(((char *)buffer)[j]);
	}

	ttyVgaCursorNode->write(ttyVgaCursorNode, 0, 1, &ttyDevice.cursorOffset);

	return count;
}

static unsigned int tty_cwd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	count = strlen(ttyCwd) - offset;

	strcpy(buffer, ttyCwd + offset);

	return count;
}

static unsigned int tty_cwd_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{
	count = strlen(ttyCwd) - offset;

	strcpy(ttyCwd + offset, buffer);

	return count;
}

static void tty_set_color(unsigned char fg, unsigned char bg)
{
	ttyDevice.cursorColor = (bg << 4) | (fg & 0x0F);
}

static void tty_init_vga()
{
	ttyDevice.cursorOffset = 0;
	ttyDevice.set_color = tty_set_color;
	ttyDevice.set_color(TTY_COLOR_GRAY2, TTY_COLOR_BLACK);

	ttyVgaNode = vfs_find_root("/dev/vga_fb");
	ttyVgaColorNode = vfs_find_root("/dev/vga_fb_color");
	ttyVgaCursorNode = vfs_find_root("/dev/vga_fb_cursor");

	ttyVgaColorNode->write(ttyVgaColorNode, 0, 1, &ttyDevice.cursorColor);

	//tty_vga_clear();
}

void tty_init()
{
	tty_init_vga();

	strcpy(ttyCwd, "/");

	struct vfs_node *ttyStdoutNode = vfs_add_node("tty", TTY_CHARACTER_SIZE);
	ttyStdoutNode->write = tty_write;

	struct vfs_node *ttyCwdNode = vfs_add_node("cwd", 256);
	ttyCwdNode->read = tty_cwd_read;
	ttyCwdNode->write = tty_cwd_write;

	struct vfs_node *devNode = vfs_find_root("/dev");
	devNode->write(devNode, devNode->length, 1, ttyStdoutNode);
	devNode->write(devNode, devNode->length, 1, ttyCwdNode);
}


#include <kcommon/call.h>
#include <kcommon/file.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>

void file_close(unsigned int fd)
{
	call_close(fd);
}

unsigned int file_info(char *name, struct file_info *info)
{
	return call_info(name, info);
}

int file_open(char *name)
{
	return call_open(name);
}

unsigned int file_read(unsigned int fd, unsigned int count, void *buffer)
{
//	return call_read(fd, buffer, count);
}

unsigned int file_read_byte(unsigned int fd, char c)
{
//	return file_read(fd, 1, &c);
}

unsigned int file_write(unsigned int fd, unsigned int count, void *buffer)
{
//	return call_write(fd, buffer, count);
}

unsigned int file_write_bcd(unsigned int fd, unsigned char num)
{
//	return file_write_dec(fd, num >> 4) + file_write_dec(fd, num & 0x0F);
}

unsigned int file_write_byte(unsigned int fd, char c)
{
//	return file_write(fd, 1, &c);
}

unsigned int file_write_dec(unsigned int fd, unsigned int num)
{
//	return file_write_num(fd, num, 10);
}

unsigned int file_write_hex(unsigned int fd, unsigned int num)
{
//	return file_write_num(fd, num, 16);
}

unsigned int file_write_num(unsigned int fd, unsigned int num, unsigned int base)
{
	if(!num)
	{
		return file_write_byte(fd, '0');
	}

	char buffer[32] = {0};
	int i;

	for(i = 30; num && i; --i, num /= base)
	{
		buffer[i] = "0123456789abcdef"[num % base];
	}

	return file_write_string(fd, buffer + i + 1);
}

unsigned int file_write_string(unsigned int fd, char *buffer)
{
	return file_write(fd, strlen(buffer), buffer);
}

unsigned int file_write_string_format(unsigned int fd, char *buffer, void **args)
{
	if(!args)
	{
		return file_write(fd, strlen(buffer), buffer);
	}

	unsigned int i;
	unsigned int length = strlen(buffer);
	unsigned int size = 0;

	for(i = 0; i < length; i++)
	{
		if(buffer[i] != '%')
		{
			size += file_write_byte(fd, buffer[i]);

			continue;
		}

		i++;

		switch(buffer[i])
		{
			case 'c':
				size += file_write_byte(fd, **(char **)args);
				break;
			case 'd':
				size += file_write_num(fd, **(int **)args, 10);
				break;
			case 's':
				size += file_write_string(fd, *(char **)args);
				break;
			case 'x':
				size += file_write_num(fd, **(int **)args, 16);
				break;
		}

		args++;

	}

	return size + i;
}



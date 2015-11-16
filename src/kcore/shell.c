#include <kconf/config.h>
#include <kcommon/call.h>
#include <kcommon/file.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kcore/shell.h>

static char shellBuffer[SHELL_BUFFER_SIZE];
static unsigned int shellBufferHead;

static void shell_stack_push(char c)
{
	if(shellBufferHead < SHELL_BUFFER_SIZE)
	{
		shellBuffer[shellBufferHead] = c;
		shellBufferHead++;
	}
}

static char shell_stack_pop()
{
	return (shellBufferHead > 0) ? shellBuffer[--shellBufferHead] : 0;
}

static void shell_stack_clear()
{
	shellBufferHead = 0;
}

static void shell_clear()
{
	file_write_string(FILE_STDOUT, "[kex:]> ");
	shell_stack_clear();
}

static void shell_call(int file, int argc, char *argv[])
{
	void *buffer = (void *)0x00300000;
	file_read(file, 0x100000, buffer);

	unsigned int address = call_map((unsigned int)buffer);
	file_write_string(FILE_STDOUT, "loading address 0x");
	file_write_hex(FILE_STDOUT, address);
	file_write_string(FILE_STDOUT, "\n");
	file_write_string(FILE_STDOUT, "argc address 0x");
	file_write_hex(FILE_STDOUT, (unsigned int)&argc);
	file_write_string(FILE_STDOUT, "\n");


	void (*func)(int argc, char **argv) = (void (*)(int argc, char **argv))address;

	func(argc, argv);
}

static void shell_interpret(char *command)
{
	char *argv[32];
	unsigned int argc = strsplt(argv, command, ' ');

	if(!argc)
	{
		return;
	}

	if(!strcmp("help", argv[0]))
	{
		file_write_string(FILE_STDOUT, "Help information not available\n");
		return;
	}

	char path[256];

	strcpy(path, "/initrd/");
	strcat(path, argv[0]);

	int file = file_open(path);

	if(file == -1)
	{
		file_write_string(FILE_STDOUT, argv[0]);
		file_write_string(FILE_STDOUT, ": illegal command\n");
		return;
	}else{
		shell_call(file, argc, argv);
	}

	file_close(file);
}

static void shell_handle_input(char c)
{
	switch(c)
	{
		case '\t':
			break;
		case '\b':
			if(shell_stack_pop())
			{
				file_write_byte(FILE_STDOUT, '\b');
				file_write_byte(FILE_STDOUT, ' ');
				file_write_byte(FILE_STDOUT, '\b');
			}
			break;
		case '\n':
			shell_stack_push('\0');
			file_write_byte(FILE_STDOUT, c);
			shell_interpret(shellBuffer);
			shell_clear();
			break;
		default:
			shell_stack_push(c);
			file_write_byte(FILE_STDOUT, c);
			break;
	}
}

static void shell_poll()
{
	char c;

	for(;;)
	{
		while (!file_read(FILE_STDIN, 1, &c));

		shell_handle_input(c);
	}
}

void shell_init()
{
	shellBufferHead = 0;

	int sin = file_open("/dev/kbd");
	int sout = file_open("/dev/tty");

	// dirty hack for now
	//{
	char c = ' ';
	unsigned int i;

	for(i = 2000; i; i--)
	{
		file_write_byte(FILE_STDOUT, c);
	}
	//}

	file_write_string(FILE_STDOUT, PROJECT_NAME);
	file_write_string(FILE_STDOUT, "\n");
	file_write_string(FILE_STDOUT, "Copyright (c) 2012\n");
	file_write_string(FILE_STDOUT, KERNEL_NAME);
	file_write_string(FILE_STDOUT, " ");
	file_write_string(FILE_STDOUT, KERNEL_VERSION);
	file_write_string(FILE_STDOUT, "\n");
	file_write_string(FILE_STDOUT, "BUILD: ");
	file_write_string(FILE_STDOUT, __DATE__);
	file_write_string(FILE_STDOUT, " ");
	file_write_string(FILE_STDOUT, __TIME__);
	file_write_string(FILE_STDOUT, "\n\n");
	file_write_string(FILE_STDOUT, "Call kernel shell...\n\n");

	shell_clear();
	shell_poll();

	file_close(sin);
	file_close(sout);
}

#include <kconf/config.h>
#include <kcommon/memory.h>
#include <kcommon/string.h>
#include <kdebug/debug.h>
#include <kfs/vfs.h>
#include <kcore/kernel.h>
#include <kcore/modules.h>
#include <kbase/mach.h>
#include <kmod/kbd/kbd.h>
#include <kmm/paging.h>
#include <ktask/task.h>
#include <kdebug/log.h>
#include <kcore/kshell.h>

char shell_buffer[256];
char current_core[32];
unsigned int shell_buffer_pos;
unsigned int kernel_shell = TRUE;

extern unsigned int code;
extern unsigned int data;
extern unsigned int bss;
extern unsigned int end;
extern int task_counter;
extern struct page_directory *current_directory;

extern void start();

static void shell_stack_push(unsigned char c)
{
	if(shell_buffer_pos < 256)
	{
		shell_buffer[shell_buffer_pos] = c;
		shell_buffer_pos++;
	}
}

static void usage()
{
	kprintf("<< Available commands:\n");
	kprintf("<< [SHOW]\tShow lists\n");
	kprintf("<< [SELECT]\tSelect object\n");
	kprintf("<< [UPDATE]\tChange object\n");
	kprintf("<< [USE]\tChange current list\n");
	kprintf("<< [ALTER]\tChange category\n");
	kprintf("<< For help [<command> HELP]\n");
}

static void show_usage()
{
	kprintf("<< Available SHOW commands:\n");
	kprintf("<< [ENV]\tPrint environment\n");
	kprintf("<< [CORES]\tList kernel cores\n");
	kprintf("<< [MODS]\tList kernel modules\n");
	kprintf("<< [HELP]\tThis usage\n");
}

static void use_usage()
{
	kprintf("<< Available USE commands:\n");
	kprintf("<< [<cores>]\tSet core to <cores>\n");
	kprintf("<< [<mods>]\tSet mod to <mods>\n");
	kprintf("<< [HELP]\tThis usage\n");
}

static void select_usage()
{
	kprintf("<< Available SELECT syntax:\n");
	kprintf("<< [HELP]\tThis usage\n");
}

void shell_parse()
{
	char *argv[32];
	unsigned int argc = strsplt(argv, shell_buffer, ' ');

	if((!strcmp("show", argv[0])) || (!strcmp("SHOW", argv[0])))
	{
		if((!strcmp("help", argv[1])) || (!strcmp("HELP", argv[1])))
		{
			show_usage();
		}
		else if((!strcmp("env", argv[1])) || (!strcmp("ENV", argv[1])))
		{
			kprintf("<< Environment variables:\n");

			if(current_core[0])
			{
				kprintf("<< Core: %s\n", current_core);
			}
		}
		else if((!strcmp("cores", argv[1])) || (!strcmp("CORES", argv[1])))
		{
			kprintf("<< Kernel cores:\n");
			kprintf("<< [KDEBUG]\tKernel debugger\n");
			kprintf("<< [KMM]\tKernel memory manager\n");
			kprintf("<< [KTASK]\tkernel tasker\n");
			kprintf("<< [KMODS]\tkernel modules\n");
		}
		else if((!strcmp("mods", argv[1])) || (!strcmp("MODS", argv[1])))
		{
			kprintf("<< Kernel mods:\n");
			kprintf("<<\t[AKBD]\n");
			kprintf("<<\t[ATA]\n");
			kprintf("<<\t[ELF]\n");
			kprintf("<<\t[IO]\n");
			kprintf("<<\t[KBD]\n");
			kprintf("<<\t[PCI]\n");
			kprintf("<<\t[PIT]\n");
			kprintf("<<\t[RTC]\n");
			kprintf("<<\t[SERIAL]\n");
			kprintf("<<\t[TTY]\n");
			kprintf("<<\t[VGA]\n");
		}
		else
		{
			kprintf("<< Syntax Error: [SHOW <object> | HELP]\n");
		}
	}
	else if((!strcmp("use", argv[0])) || (!strcmp("USE", argv[0])))
	{
		if((!strcmp("help", argv[1])) || (!strcmp("HELP", argv[1])))
		{
			use_usage();
		}
		else if((!strcmp("kdebug", argv[1])) || (!strcmp("KDEBUG", argv[1])))
		{
			strcpy(current_core, "-KDEBUG");
			kprintf("<< Using core KDEBUG\n");
		}
		else if((!strcmp("kmm", argv[1])) || (!strcmp("KMM", argv[1])))
		{
			strcpy(current_core, "-KMM");
			kprintf("<< Using core KMM\n");
		}
		else if((!strcmp("ktask", argv[1])) || (!strcmp("KTASK", argv[1])))
		{
			strcpy(current_core, "-KTASK");
			kprintf("<< Using core KTASK\n");
		}
		else if((!strcmp("kmods", argv[1])) || (!strcmp("KMODS", argv[1])))
		{
			strcpy(current_core, "-KMODS");
			kprintf("<< Using core KMODS\n");
		}
		else
		{
			kprintf("<< Syntax Error: [USE <object> | HELP]\n");
		}
	}
	else if((!strcmp("select", argv[0])) || (!strcmp("SELECT", argv[0])))
	{
		if((!strcmp("help", argv[1])) || (!strcmp("HELP", argv[1])))
		{
			select_usage();
		}
		else
		{
			kprintf("<< Syntax Error: [SELECT <object> | HELP]\n");
		}
	} //temp usage
	else if((!strcmp("memory", argv[0])) || (!strcmp("MEMORY", argv[0])))
	{
		if((!strcmp("map", argv[1])) || (!strcmp("MAP", argv[1])))
		{
			kprintf("<< Memory mapping sections:\n");
			kprintf("<<\t.text addr 0x%x\n", &code);
			kprintf("<<\t.data addr 0x%x\n", &data);
			kprintf("<<\t.bss addr 0x%x\n", &bss);
			kprintf("<<\t.end addr 0x%x\n", &end);
			kprintf("<< Memory call adresses:\n");
			kprintf("<<\t_start() addr 0x%x\n", &start);
			kprintf("<<\t_mach_init() addr 0x%x\n", &mach_init);
			kprintf("<<\t_kinit() addr 0x%x\n", &kinit);
		}
		else if((!strcmp("pd", argv[1])) || (!strcmp("PD", argv[1])))
		{
			info_page_directory(current_directory);
		}
		else if((!strcmp("status", argv[1])) || (!strcmp("STATUS", argv[1])))
		{
			physical_memory_use();
		}
		else
		{
			kprintf("<< Syntax Error: [SELECT <object> | HELP]\n");
		}
	}
	else if((!strcmp("task", argv[0])) || (!strcmp("TASK", argv[0])))
	{
		if((!strcmp("map", argv[1])) || (!strcmp("MAP", argv[1])))
		{
			kprintf("<< Task mapping:\n");
			kprintf("<<\tTask sequence %u\n", task_counter);
		}
		else
		{
			kprintf("<< Syntax Error: [SELECT <object> | HELP]\n");
		}
	}
	else if((!strcmp("sleep", argv[0])) || (!strcmp("SLEEP", argv[0])))
	{
		timer_wait(5);
	}
	else if((!strcmp("reboot", argv[0])) || (!strcmp("REBOOT", argv[0])))
	{
		mach_reboot();
	}
	else if((!strcmp("exit", argv[0])) || (!strcmp("EXIT", argv[0])) || (!strcmp("quit", argv[0])) || (!strcmp("QUIT", argv[0])))
	{
		kernel_shell = FALSE;
	}
}

static void handle_char(unsigned char a)
{
	unsigned char tmp_string[2];
	tmp_string[1] = 0;
	tmp_string[0] = a;

	switch(a)
	{
		case '\t':
			kprintf("\t");
			break;
		case '\b':
			if(shell_buffer_pos > 0)
			{
				shell_buffer[--shell_buffer_pos];
				kprintf("\b");
			}
			break;
		case '\n':
			shell_stack_push('\0');
			kprintf(tmp_string);
			shell_parse();
			shell_buffer_pos = 0;
			kprintf(">>");
			break;
		case '?':
			shell_stack_push(a);
			kprintf(tmp_string);
			usage();
			shell_buffer_pos = 0;
			kprintf(">>");
			break;
		default:
			shell_stack_push(a);
			kprintf(tmp_string);
			break;
	}
}

void kshell_init()
{
	unsigned char c = 0;

	if(!kernel_shell)
	{
		return;
	}

	kprintf("%s\nCopyright (c) 2012\n%s %s\nBUILD: %s %s\n", PROJECT_NAME, KERNEL_NAME, KERNEL_VERSION, __DATE__, __TIME__);
	kprintf(">>");

	while(kernel_shell)
	{
	 	c = kbd_buffer();
	 	if(c)
	 	{
	 		handle_char(c);
	 	}
	}
}

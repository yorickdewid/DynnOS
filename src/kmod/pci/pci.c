#include <kconf/config.h>
#include <kcommon/string.h>
#include <kfs/vfs.h>
#include <kcore/modules.h>
#include <kbase/ios.h>
#include <kmod/pci/pci.h>

static unsigned short pci_read(unsigned short bus, unsigned short slot, unsigned short func, unsigned short offset)
{
	unsigned int lbus = (unsigned int)bus;
	unsigned int lslot = (unsigned int)slot;
	unsigned int lfunc = (unsigned int)func;
	unsigned int tmp = 0;

	unsigned int address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

	io_outd(PCI_ADDRESS, address);

	tmp = (unsigned short)((io_ind(PCI_DATA) >> ((offset & 2) * 8)) & 0xFFFF);

	return tmp;
}

static unsigned short pci_check_vendor(unsigned short bus, unsigned short slot)
{
	unsigned short vendor = NULL;
	unsigned short device = NULL;

	if((vendor == pci_read(bus, slot, 0, 0)) != 0xFFFF)
    {
		device = pci_read(bus, slot, 0, 2);
	}

	return vendor;
}

void pci_init()
{
}

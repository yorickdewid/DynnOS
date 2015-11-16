#ifndef __ATA_H
#define ___ATA_H

#define ATA_CONTROL_STATUS				0x02

#define ATA_COMMAND_ID					0xEC
#define ATA_COMMAND_ATAPI				0xA0
#define ATA_COMMAND_ATAPI_EJECT			0x1B
#define ATA_COMMAND_ATAPI_ID			0xA1
#define ATA_COMMAND_ATAPI_READ			0xA8
#define ATA_COMMAND_DMA28_READ			0xC8
#define ATA_COMMAND_DMA28_WRITE			0xCA
#define ATA_COMMAND_DMA48_READ			0x25
#define ATA_COMMAND_DMA48_WRITE			0x35
#define ATA_COMMAND_PIO28_READ			0x20
#define ATA_COMMAND_PIO28_WRITE			0x30
#define ATA_COMMAND_PIO48_READ			0x24
#define ATA_COMMAND_PIO48_WRITE			0x34

#define ATA_DATA_COMMAND				0x07
#define ATA_DATA_COUNT0					0x02
#define ATA_DATA_COUNT1					0x08
#define ATA_DATA_FEATURE				0x01
#define ATA_DATA_LBA0					0x03
#define ATA_DATA_LBA1					0x04
#define ATA_DATA_LBA2					0x05
#define ATA_DATA_LBA3					0x09
#define ATA_DATA_LBA4					0x0A
#define ATA_DATA_LBA5					0x0B
#define ATA_DATA_SELECT					0x06

#define ATA_ID_TYPE						0x00
#define ATA_ID_SERIAL					0x0A
#define ATA_ID_MODEL					0x1B
#define ATA_ID_LBA28MAX					0x3C
#define ATA_ID_LBA48MAX					0x64

#define ATA_PRIMARY_MASTER_CONTROL		0x3F6
#define ATA_PRIMARY_MASTER_DATA			0x1F0
#define ATA_PRIMARY_SLAVE_CONTROL		0x376
#define ATA_PRIMARY_SLAVE_DATA			0x170

#define ATA_SECONDARY_MASTER_CONTROL	0x3E6
#define ATA_SECONDARY_MASTER_DATA		0x1E8
#define ATA_SECONDARY_SLAVE_CONTROL		0x366
#define ATA_SECONDARY_SLAVE_DATA		0x168

#define ATA_STATUS_FLAG_ERROR			1 << 0
#define ATA_STATUS_FLAG_DRQ				1 << 3
#define ATA_STATUS_FLAG_BUSY			1 << 7

#define ATA_DEVICE_TYPE_UNKNOWN			0x00
#define ATA_DEVICE_TYPE_ATA				0x01
#define ATA_DEVICE_TYPE_ATAPI			0x02
#define ATA_DEVICE_TYPE_SATA			0x03
#define ATA_DEVICE_TYPE_SATAPI			0x04

struct ata_device
{
	struct modules_device base;
	unsigned short control;
	unsigned short data;
	unsigned char secondary;
	unsigned char type;
};

extern void ata_init();

#endif


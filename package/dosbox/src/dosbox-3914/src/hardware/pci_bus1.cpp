/*
 *  Copyright (C) 2002-2015  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "dosbox.h"
#include "inout.h"
#include "mem.h"
#include "paging.h"
#include "pci_bus.h"
#include "setup.h"
#include "debug.h"
#include "callback.h"
#include "regs.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <error.h>
#include <errno.h>
static int pci_check_device_present_proc(unsigned char bus, unsigned char device,
					 unsigned char fn);

static void pci_write_proc (unsigned char bus, unsigned char device,
			    unsigned char fn, unsigned long reg, unsigned long val, int len);

static unsigned long pci_read_proc (unsigned char bus, unsigned char device,
				    unsigned char fn, unsigned long reg, int len);

static int
interpretCfgSpace(unsigned int *pciheader,unsigned int *pcibuses,int busidx,
		   unsigned char dev, unsigned char func);
static int pcibios_init(void);

#define PCI_VENDOR_ID			0x00
#define PCI_VENDOR_ID_INTEL		0x8086
#define PCI_VENDOR_ID_COMPAQ		0x0e11
#define PCI_BASE_ADDRESS_0		0x10
#define PCI_BASE_ADDRESS_5		0x24
#define PCI_BASE_ADDRESS_SPACE_IO	0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
#define PCI_BASE_ADDRESS_MEM_MASK	(~0x0fUL)
#define PCI_BASE_ADDRESS_IO_MASK	(~0x03UL)
#define PCI_ROM_ADDRESS			0x30		
#define PCI_ROM_ADDRESS_MASK		(~0x7ffUL)
#define PCI_CLASS_DISPLAY_VGA		0x0300
#define PCI_CLASS_BRIDGE_HOST		0x0600
#define PCI_CLASS_DEVICE		0x0a

#define PCI_VERSION 0x0210  /* version 2.10 */
#define PCI_CONFIG_1 0x01 
#define PCI_CONFIG_2 0x02 
#define MAX_DEV_PER_VENDOR_CFG1  32
#define MAX_PCI_DEVICES 64
#define PCI_MULTIFUNC_DEV  0x80
#define PCI_CLASS_MASK 0xff0000
#define PCI_SUBCLASS_MASK 0xff00
#define PCI_CLASS_BRIDGE 0x060000
#define PCI_SUBCLASS_BRIDGE_PCI 0x0400
#define PCI_SUBCLASS_BRIDGE_HOST 0x0000
#define PCI_BRIDGE_CLASS(classid) ((classid & PCI_CLASS_MASK) == PCI_CLASS_BRIDGE)
#define PCI_BRIDGE_PCI_CLASS(classid) ((classid & PCI_SUBCLASS_MASK) \
                                       == PCI_SUBCLASS_BRIDGE_PCI)
#define PCI_BRIDGE_HOST_CLASS(classid) ((classid & PCI_SUBCLASS_MASK) \
                                       == PCI_SUBCLASS_BRIDGE_HOST)
#define debug_printf(...)



typedef struct _pciRec {
    int enabled, ext_enabled;
    unsigned short bdf;
    unsigned short vendor;
    unsigned short device;
    unsigned long classid;
    struct {
        int type;
        unsigned long base;
        unsigned long size;
        unsigned long rawsize;
    } region[7];
    unsigned int header[16];
    struct _pciRec *next;
} pciRec, *pciPtr;

static unsigned int pcicfg_addr;
static unsigned int pcicfg_val;
static int lastBus = 0;
static pciPtr pciList = NULL;
static int proc_bus_pci_devices_get_sizes(pciPtr pci);

#if defined(PCI_FUNCTIONALITY_ENABLED)
void pcicfg_addr_Write(Bitu port, Bitu val, Bitu len) {
	switch(port) {
		case 0xcf8:
		case 0xcf9:
		case 0xcfa:
		case 0xcfb:
		 memcpy(((char *)&pcicfg_addr)+port-0xcf8,&val,len);
		break;
	}
}

Bitu pcicfg_addr_Read(Bitu port, Bitu len) {
Bitu val;
	
	val = 0;

	switch(port) {
		case 0xcf8:
		case 0xcf9:
		case 0xcfa:
		case 0xcfb:
		 memcpy((char *)&val,((char *)&pcicfg_addr)+port-0xcf8,len);
		break;
	}
	return val;
}

void pcicfg_value_Write(Bitu port, Bitu val, Bitu len) {
int bus, device, func, reg;
int fd, exist;
bus = (pcicfg_addr&~0x80000000)>>16;
device = (pcicfg_addr>>11)&0x1f;
func = (pcicfg_addr>>8)&0x7; 
reg = (pcicfg_addr&0xfc)+port-0xcfc;


	exist = pci_check_device_present_proc(bus, device, func);
	if(!exist) return;

	pci_write_proc(bus, device, func, reg, val, len);


}

Bitu pcicfg_value_Read(Bitu port, Bitu len) {
Bitu val;
int bus, device, func, reg;
int fd, exist;
bus = (pcicfg_addr&~0x80000000)>>16;
device = (pcicfg_addr>>11)&0x1f;
func = (pcicfg_addr>>8)&0x7; 
reg = (pcicfg_addr&0xfc)+port-0xcfc;
	
	exist = pci_check_device_present_proc(bus, device, func);
	if(!exist) return -1;

        val =pci_read_proc(bus, device, func, reg, len);

return val;
}

int __attribute__((noinline))  pci_read_probe(PhysPt addr, Bitu val, char *func)
{
debug_printf("pci memory %s addr:0x%x,val 0x%x, CS:IP %8x:%8x\n",func,(unsigned int)addr, (unsigned int)val,SegValue(cs),reg_eip);
}

static HostPt IoBase;
Bitu pci_io_Read(Bitu port, Bitu len)
{
  switch(len)
  {
   case 1:
	return *(volatile char *)(IoBase+port);
   case 2:
	return *(volatile short *)(IoBase+port);
   case 4:
	return *(volatile int *)(IoBase+port);
   default:
	printf("pci_io_Read len error port=0x%x, len=0x%x\n", port, len);
	return -1;
  }
}

void pci_io_Write(Bitu port, Bitu val, Bitu len) {
  switch(len)
  {
   case 1:
	*(volatile char *)(IoBase+port) = val;
	break;
   case 2:
	*(volatile short *)(IoBase+port) = val;
	break;
   case 4:
	*(volatile int *)(IoBase+port) = val;
	break;
   default:
	printf("pci_io_Write len error port=0x%x, len=0x%x\n", port, len);
	break;
  }

}


int __attribute__((noinline))  pci_write_probe(PhysPt addr,Bitu val, char *func)
{
debug_printf("pci memory %s addr:0x%x, val 0x%x, CS:IP %8x:%8x\n",func, (unsigned int)addr, (unsigned int)val,SegValue(cs),reg_eip);
}

class PCIPageHandler : public PageHandler {
public:
	HostPt MemBase;
	PCIPageHandler() {
		int fd;
		int i;
		fd=open("/dev/mem",O_RDWR|O_SYNC);
		if(fd==-1)error(-1,errno,"error open /dev/mem\n");
		MemBase = (HostPt)mmap(0,PCI_MMAP_END-PCI_MMAP_START,PROT_READ|PROT_WRITE,MAP_SHARED,fd,PCI_MMAP_START);
		if (MemBase == MAP_FAILED)
			error(-1,errno,"map mem failed");
		munmap(MemBase, PCI_MMAP_END-PCI_MMAP_START);
		
		for(i=0;i<PCI_MMAP_END-PCI_MMAP_START;i+=0x1000000)
		{
			mmap(MemBase+i,0x1000000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,PCI_MMAP_START+i);
		}

		close(fd);
	
		//flags=PFLAG_READABLE|PFLAG_WRITEABLE;
		flags=PFLAG_NOCODE;
	}
#if 0
	HostPt GetHostReadPt(Bitu phys_page) {
		return MemBase+(phys_page-PCI_MMAP_START/MEM_PAGESIZE)*MEM_PAGESIZE;
	}
	HostPt GetHostWritePt(Bitu phys_page) {
		return MemBase+(phys_page-PCI_MMAP_START/MEM_PAGESIZE)*MEM_PAGESIZE;
	}
#endif

	Bitu readb(PhysPt addr) {
	 Bitu val;
	 val = *(volatile unsigned char *)(MemBase+addr-PCI_MMAP_START);
	pci_read_probe(addr, val, "readb");
	return val;
	} 

	Bitu readw(PhysPt addr) {
	 Bitu val;
	 val = *(volatile unsigned short *)(MemBase+addr-PCI_MMAP_START);
	pci_read_probe(addr, val, "readw");
	return val;
	} 

	Bitu readd(PhysPt addr) {
	 Bitu val;
	 val = *(volatile unsigned int *)(MemBase+addr-PCI_MMAP_START);
	pci_read_probe(addr, val, "readd");
	return val;
	} 

	void writeb(PhysPt addr,Bitu val) {
	pci_write_probe(addr,val, "writeb");
	 *(volatile unsigned char *)(MemBase+addr-PCI_MMAP_START) = val;
	}

	void writew(PhysPt addr,Bitu val) {
	pci_write_probe(addr,val, "writew");
	 *(volatile unsigned short *)(MemBase+addr-PCI_MMAP_START) = val;
	}

	void writed(PhysPt addr,Bitu val) {
	pci_write_probe(addr,val,"writed");
	 *(volatile unsigned int *)(MemBase+addr-PCI_MMAP_START) = val;
	}

};
	
PCIPageHandler pcihander;


void PCI_Init(Section* sec) {
	int fd;
	fd=open("/dev/mem",O_RDWR|O_SYNC);
	if(fd==-1)error(-1,errno,"error open /dev/mem\n");
	IoBase = (HostPt)mmap(0,0x10000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x18000000);
	if (IoBase == MAP_FAILED)
		error(-1,errno,"map io failed");
	close(fd);

	IO_RegisterReadHandler(0xcf8,pcicfg_addr_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterReadHandler(0xcfc,pcicfg_value_Read,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xcf8,&pcicfg_addr_Write,IO_MB | IO_MW | IO_MD);
	IO_RegisterWriteHandler(0xcfc,&pcicfg_value_Write,IO_MB | IO_MW | IO_MD);
	pcibios_init();
	//pcihander = new PCIPageHandler();
}

static int pci_open_proc(unsigned char bus, unsigned char device,
			 unsigned char fn)
{
  static char proc_pci_name_buf[] = "/proc/bus/pci/00/00.0";
  int fd;

  //PRIV_SAVE_AREA
  sprintf(proc_pci_name_buf + 14, "%02x/%02x.%d", bus, device, fn);
//  debug_printf("PCI: opening %s\n", proc_pci_name_buf);
  //enter_priv_on();
  fd = open(proc_pci_name_buf, O_RDWR);
  if (fd == -1)
    fd = open(proc_pci_name_buf, O_RDONLY);
  //leave_priv_setting();
  return fd;
}

static unsigned long pci_read_proc (unsigned char bus, unsigned char device,
				    unsigned char fn, unsigned long reg, int len)
{
  unsigned long val;
  int fd = pci_open_proc(bus, device, fn);
  if (fd == -1)
    return 0;
  debug_printf("PCI: reading reg %ld\n", reg);
  pread(fd, &val, len, reg);
  close(fd);
  return val;
}

static void pci_write_proc (unsigned char bus, unsigned char device,
			    unsigned char fn, unsigned long reg, unsigned long val, int len)
{
  int fd = pci_open_proc(bus, device, fn);
  if (fd == -1)
    return;
  debug_printf("PCI: writing reg %ld\n", reg);
  pwrite(fd, &val, len, reg);
  close(fd);
}

/* only called from pci bios init */
static int pci_check_device_present_proc(unsigned char bus, unsigned char device,
					 unsigned char fn)
{
  int fd = pci_open_proc(bus, device, fn);
  close(fd);
  return (fd != -1);
}


static int pci_read_header_proc (unsigned char bus, unsigned char device,
	unsigned char fn, unsigned int *buf)
{
  int fd = pci_open_proc(bus, device, fn);
  if (fd == -1)
    return 0;

  /* Get only first 64 bytes: See src/linux/drivers/pci/proc.c for
     why. They are not joking. My NCR810 crashes the machine on read
     of register 0xd8 */
  
  read(fd, buf, 64);
  close(fd);
  return 0;
}


static int numbus = 0;
static int hostbridges = 0;

static int pcibios_init(void)
{
    unsigned int pcibuses[16];
    unsigned int pciheader[16];
    int busidx = 0;
    int idx = 0;
    int func = 0;
    int cardnum;
    int cardnum_lo = 0;
    int cardnum_hi = MAX_DEV_PER_VENDOR_CFG1;
    int func_hi = 8;
    static int ret = -1;

    if (ret != -1) return ret;
    ret = 0;

    pcibuses[0] = 0;

    debug_printf("PCI enabled\n");
    

    do {
	for (cardnum = cardnum_lo; cardnum < cardnum_hi; cardnum++) {
	    func = 0;
	    do {
	    	if (!pci_check_device_present_proc(pcibuses[busidx],
							 cardnum,func))
		    break;
		pci_read_header_proc(pcibuses[busidx],cardnum,func,
					   pciheader);
		func = interpretCfgSpace(pciheader,pcibuses,busidx,
					 cardnum,func);
		if (idx++ > MAX_PCI_DEVICES)
		    continue;
	    } while (func < func_hi);
	}
    } while (++busidx <= numbus);

    lastBus = numbus;
    ret = 1;
    return 1;
}


static int
interpretCfgSpace(unsigned int *pciheader,unsigned int *pcibuses,int busidx,
		   unsigned char dev, unsigned char func)
{
    static char *typestr[] = { "MEM", "IO", "ROM" };
    int tmp, i;

    pciPtr pciTmp = (pciPtr)malloc(sizeof(pciRec));
    pciTmp->next = pciList;
    pciList = pciTmp;
    pciTmp->enabled = 1;
    pciTmp->bdf = pcibuses[busidx] << 8 | dev << 3 | func;
    pciTmp->vendor = pciheader[0] & 0xffff;
    pciTmp->device = pciheader[0] >> 16;
    pciTmp->classid = pciheader[0x02] >> 8;
    if (PCI_BRIDGE_CLASS(pciTmp->classid))  {
	if (PCI_BRIDGE_PCI_CLASS(pciTmp->classid)) { /*PCI-PCI*/
	    debug_printf("PCI-PCI bridge:\n");
	    /* always enable for PCI emulation */
	    pciTmp->enabled = 1;
	    tmp = (pciheader[0x6] >> 8) & 0xff;
	    if (tmp > 0)
		pcibuses[++numbus] = tmp; /* secondary bus */
	} else if (PCI_BRIDGE_HOST_CLASS(pciTmp->classid)) {
	    debug_printf("PCI-HOST bridge:\n");
	    /* always enable for PCI emulation */
	    pciTmp->enabled = 1;
	    if (++hostbridges > 1) {/* HOST-PCI bridges*/
		numbus++;
		pcibuses[numbus] = numbus;
	    }
	}
    }
    memcpy(pciTmp->header, pciheader, sizeof(*pciheader) * 16);
    memset(&pciTmp->region, 0, sizeof(pciTmp->region));
    if ((pciheader[3] & 0x007f0000) == 0) {
      int got_sizes = proc_bus_pci_devices_get_sizes(pciTmp);
      for (i = 0; i < 7; i++) {
	unsigned long mask, base, size, pci_val, pci_val1;
	unsigned long reg = PCI_BASE_ADDRESS_0 + (i << 2);
	int type;

	if (i == 6) reg = PCI_ROM_ADDRESS;
	pci_val = pciheader[reg/4];
	if (pci_val == 0xffffffff || pci_val == 0)
	    continue;
	if (i == 6) {
	    mask = PCI_ROM_ADDRESS_MASK;
	    type = 2;
	    type = PCI_BASE_ADDRESS_SPACE_MEMORY;
	} else if (pci_val & PCI_BASE_ADDRESS_SPACE_IO) {
	    mask = PCI_BASE_ADDRESS_IO_MASK & 0xffff;
	    type = PCI_BASE_ADDRESS_SPACE_IO;
	} else { /* memory */
	    mask = PCI_BASE_ADDRESS_MEM_MASK;
	    type = PCI_BASE_ADDRESS_SPACE_MEMORY;
	}
	base = pci_val & mask;
	if (!base)
	    continue;
	pciTmp->region[i].base = base;
	pciTmp->region[i].type = type;
	if (!got_sizes) {
	    pci_write_proc(pcibuses[busidx], dev, func, reg, 0xffffffff, 4);
	    pci_val1 = pci_read_proc(pcibuses[busidx], dev, func, reg, 4);
	    pci_write_proc(pcibuses[busidx], dev, func, reg, pci_val, 4);
	    pciTmp->region[i].rawsize = (~(pci_val1 & mask))+1;
	}
	if (pciTmp->region[i].rawsize == 0) {
	    pciTmp->region[i].base = 0;
	    continue;
	}
	size = pciTmp->region[i].rawsize & mask;
	size = (size & ~(size - 1)) ;
	debug_printf("PCI: found %s region at %#lx [%#lx] (%lx,%lx)\n",
		 typestr[type], base, size, pci_val, pciTmp->region[i].rawsize);
	pciTmp->region[i].size = size;
	if (type == PCI_BASE_ADDRESS_SPACE_IO) 
	{
		unsigned int addr;
		for(addr=base;addr<base+size;addr++)
		{
			IO_RegisterReadHandler(addr,pci_io_Read,IO_MB | IO_MW | IO_MD);
			IO_RegisterWriteHandler(addr,pci_io_Write,IO_MB | IO_MW | IO_MD);
		}
	}
      }
    }

    debug_printf("bus:%i dev:%i func:%i vend:0x%x dev:0x%x"
	     " classid:0x%lx bdf:0x%x\n",pcibuses[busidx],
	     dev,func,pciTmp->vendor,pciTmp->device,
	     pciTmp->classid,pciTmp->bdf);
    if ((func == 0) 
	&& ((((pciheader[0x03] >> 16) & 0xff)
	     & PCI_MULTIFUNC_DEV)==0))
	func = 8;
    else
	func++;
    return func;
}


static int proc_bus_pci_devices_get_sizes(pciPtr pci)
{
    FILE *f;
    char buf[512];

    f = fopen("/proc/bus/pci/devices", "r");
    if (!f) {
	debug_printf("PCI: Cannot open /proc/bus/pci/devices\n");
	return 0;
    }
    while (fgets(buf, sizeof(buf)-1, f)) {
	unsigned int cnt, i, bdf, tmp;
	unsigned int lens[7];

#define F " %08x"
	cnt = sscanf(buf, "%x %x %x"  F F F F F F F  F F F F F F F,
		    &bdf, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp,
		    &lens[0], &lens[1], &lens[2], &lens[3], &lens[4], &lens[5],
		    &lens[6]);
#undef F
	if (cnt != 17) {
	    debug_printf("PCI: /proc: (read only %d items)", cnt);
	    fclose(f);
	    return 0;
	}
	if (pci->bdf == bdf)
	    for (i = 0; i < 7; i++)
		pci->region[i].rawsize = lens[i];
    }
    fclose(f);
    debug_printf("PCI: proc_bus_pci_get_sizes done\n");
    return 1;
}

bool PCI_IsInitialized() {
	return true;
}

unsigned short
findDevice(unsigned short device, unsigned short vendor, int num)
{
    pciPtr pci = pciList;

    debug_printf("PCIBIOS: find device %d id=%04x vend=%04x", num,device,vendor);
    while (pci) {
	if ((pci->vendor == vendor) && (pci->device == device) && pci->enabled) {
	    if (num-- == 0) {
		debug_printf(" at: %04x\n",pci->bdf);
		return pci->bdf;
	    }
	}
	pci=pci->next;
    }
    debug_printf(" not found\n");
    return 0xffff;
}

pciRec *pcibios_find_bdf(unsigned short bdf)
{
    pciPtr pci = pciList;

    debug_printf("pcibios find bdf %x ", bdf);
    while (pci) {
	if (pci->enabled && pci->bdf == bdf) {
	    debug_printf("class: %lx\n",pci->classid);
	    return pci;
	}
	pci=pci->next;
    }
    debug_printf(" not found\n");
    return NULL;
}

pciRec *pcibios_find_class(unsigned long classid,  int num)
{
    pciPtr pci = pciList;

    debug_printf("pcibios find device %d class %lx\n", num, classid);
    while (pci) {
	if ((pci->classid & 0xFFFFFF) == (classid & 0xFFFFFF)) {
	    if (num-- == 0) {
		debug_printf(" at: %04x\n",pci->bdf);
		return pci;
	    }
	}
	pci=pci->next;
    }
    debug_printf(" not found\n");
    return NULL;
}

unsigned short
findClass(unsigned long classid,  int num)
{
    pciPtr pci = pcibios_find_class(classid, num);
    return (pci && pci->enabled) ? pci->bdf : 0xffff;
}

#endif


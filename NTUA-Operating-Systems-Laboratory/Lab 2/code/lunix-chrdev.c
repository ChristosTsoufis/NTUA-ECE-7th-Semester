/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * < Your name here >
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include <asm/uaccess.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"
/* A constant indicating the default
 * position for the decimal point in
 * the formatted values */
#define DEFAULT_DOT_POS 3

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	/* ! */
	/* We declare the (boolean) return value */
	int ret;
	struct lunix_sensor_struct *sensor;
	
	WARN_ON ( !(sensor = state->sensor));
	/* ! */
	/* We simply compare the sensor timestamp with the most recent one stored
	   in the measurement state struct */
	ret = (state->buf_timestamp != sensor->msr_data[state->type]->last_update);

	/* The following return is bogus, just for the stub to compile */
	/* The return is not bogus anymore, but equal to the result of the
	   above comparison */
	return ret; /* ! */
}

/*
 * A poorly written function to write the value from the lookup table
 * into the state buffer (using snprintf), and if a non zero dot position
 * is given, we move every fractional digit a position to the right, adding
 * a decimal point before the first one. We include a new line character
 * and update the buf_lim to be equal to the number of bytes in the
 * formated value.
 */

static void format_value (
	/* The state struct to update */ struct lunix_chrdev_state_struct *state,
	/* Value to convert to string and format */ long int value,
	/* Dot position, starting from the most right digit */	int dot
	) {
	int i, s;
	/* Copy the value from the lookup table into the buffer */
	s = snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ, "%ld", value);
	/* Position the decimal point if needed */
	if (dot != 0) {
		for (i = 0; i < dot; i++)
			state->buf_data[s - i] = state->buf_data[s - i - 1];
		state->buf_data[s - i] = '.';
	}
	/* Add the new line character at the end */
	state->buf_data[s + (dot != 0)] = '\n';
	/* Update the buffer limit to be ready for copy_to_user() */
	state->buf_lim = s + 1 + (dot != 0);
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;
	/* ! */
	uint16_t value;
	long int temp = 0;
	unsigned long flags;

	debug("leaving\n");

	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	
	/* ! */
	WARN_ON ( !(sensor = state->sensor) );
	/* Acquire the sensor lock */
	/* NOTE: We use spin_lock_irqsave() beacuse it is possible that new data
	 * may be sent from the same sensor, causing an interrupt and putting this
	 * function on hold while we call the sequence: lunix_ldisc_receive() ->
	 * lunix_protocol_received_buf() -> lunix_protocol_update_sensors() ->
	 * lunix_sensor_update() and thus trying to lock again the sensor
	 * spinlock. In practice, this is not that possible, because the frequency
	 * with which new data is sent from the same sensor is low enough to not
	 * cause this sort of aformentioned trouble */
	spin_lock_irqsave(&sensor->lock, flags);
	
	/* Why use spinlocks? See LDD3, p. 119 */
	/* For every XMesh packet we update 3 measurements. If more than
	 * one file for the same measurement is open, one read process
	 * would go to sleep if a structure other than spinlock was used.
	 * This would be obviously time-consuming, and thus spinlock is
	 * more suitable for the sensor struct. */
	
	/*
	 * Any new data available?
	 */
	
	/* ! */
	/* We examine the result of lunix_chrdev_state_needs_refresh() */
	if (lunix_chrdev_state_needs_refresh(state))
		/* Grab the value for the specific sensor */
		value = sensor->msr_data[state->type]->values[0];
	else {	/* We must not forget to unlock the spinlock */
		spin_unlock_irqrestore(&sensor->lock, flags);
		/* We return the value indicated by lunix_chrdev_read()
		 * EAGAIN = "there is no data available right now, try
		 * again later" */
		return -EAGAIN;
	     }
	/* Restore the sensor lock */
	spin_unlock_irqrestore(&sensor->lock, flags);

	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */

	/* ! */
	/* Look at the lookup table in lunix-lookup.h, for the
	 * measurement specified by state->type */
	
	if (state->switch_raw) {
		temp = value;
		goto out;
	}
	
	switch (state->type) {
		case BATT  : temp = lookup_voltage[value]; 	break;
		case TEMP  : temp = lookup_temperature[value];	break;
		case LIGHT : temp = lookup_light[value];	break;
		case N_LUNIX_MSR : /*This case is unlikely to occur */ ;
	     /* default    : [IT MAY BE BETTER TO RETURN WITH AN ERROR] */
	}
	
out:	/* Format the value acquired from the lookup table. If it is a LIGHT
	 * value, no decimal point is required */
	format_value(state, temp, state->type == LIGHT || state->switch_raw ? 0 : DEFAULT_DOT_POS);
	/* Update the timestamp */
	state->buf_timestamp = sensor->msr_data[state->type]->last_update;
	
	debug("leaving\n");
	return 0;
}

/*===================================*\
 * Implementation of file operations *
 * for the Lunix character device    *
\*===================================*/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	/* Declarations */
	/* ! */
	struct lunix_chrdev_state_struct *state;
	int ret;

	debug("entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */
	
	/* Allocate a new Lunix character device private state structure */
	/* ! */
	/* We perform the allocation using kmalloc */
	state = kmalloc(sizeof(struct lunix_chrdev_state_struct), GFP_KERNEL);
	if (!state) {
		/* If the allocation failed, return the suitable errno value */
		ret = -ENOMEM;
		goto out;
	}
	
	/* TYPE: We acquire the type of measurement from the 
	 * last 3 bits of the minor number */
	state->type = iminor(inode) & 0b111;
	/* SENSOR: Associate the file with the corresponding
	 * sensor struct defined in lunix.h */
	state->sensor = &lunix_sensors[iminor(inode) >> 3];
	/* BUF_LIM: Initially no measurement is cahced.
	 * This is not really necessary */
	state->buf_lim = 0;
	/* LOCK: Initialize the semaphore with value equal
	 * to 1. This is the same as init_MUTEX */
	sema_init(&state->lock, 1);
	/* BUF_TIMESTAMP: It is important to be initialized
	 * to 0, because the sensor MSR DATA is initialized
	 * as a zeroed page */
	state->buf_timestamp = 0;
	state->switch_raw = 0;
	
	/* Assign the state structure to the private_data field
	 * of the given file pointer */
	filp->private_data = state;
out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	/* ! */
	/* Deallocate the memory used for the private_data structure */
	kfree(filp->private_data);
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	struct lunix_chrdev_state_struct *state;
	state = filp->private_data;
	
	switch(cmd) {
		case LUNIX_IOC_SWITCH:
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			if (down_interruptible(&state->lock))
				return -ERESTARTSYS;	
			state->switch_raw = !state->switch_raw;
			up(&state->lock);
			break;
		default:
			return -ENOTTY;
	}
	
	// return -EINVAL;
	return retval;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret;

	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	/* ! */
	/* Lock? */
	/* We try to acquire the semaphore (used in case many processes
	 * try to read the same sensor measurement) */
	if (down_interruptible(&state->lock))
		return -ERESTARTSYS;

	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* ! */
			/* "Release" the semaphore before sleeping */
			up(&state->lock);
			/* If non-blocking is requested by the process,
			 * we return a -EAGAIN errno value and avoid
			 * putting the process to sleep */
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;
			/* Add the process to the waiting queue while no new data is
			 * available */
			/* LINK: https://stackoverflow.com/questions/9254395/ */			
			if (wait_event_interruptible(sensor->wq, lunix_chrdev_state_needs_refresh(state)))
				return -ERESTARTSYS;
			
			/* Re-acquire the lock before continuing */
			if (down_interruptible(&state->lock))
				return -ERESTARTSYS;
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */
		}
		lunix_chrdev_state_update(state);
	}
	/* End of file */
	/* ! */
	/* We initially assume that zero bytes are going to be read */
	ret = 0;
	/* Unlikely: If the f_pos value exceeds the buf_lim (what we
	 * actually have to copy), reset it and return */
	if (*f_pos > state->buf_lim) {
		*f_pos = 0;
		goto out;
	}

	/* Determine the number of cached bytes to copy to userspace */
	/* ! */
	/* If more bytes than those available are requested, redefine
	 * the cnt value */
	if (*f_pos + cnt > state->buf_lim)
		cnt = state->buf_lim - *f_pos;

	/* cnt indicates how many bytes will be transfered to the user */
	ret = cnt;
	if (copy_to_user(usrbuf, state->buf_data + *f_pos, cnt)) {
		/* If copying fails, release the semaphore */
		up(&state->lock);
		/* Indicate that a bad address was given */
		ret = -EFAULT;
		goto out;
	}

	/* Increase the f_pos by cnt for it to be ready at the next call */
	*f_pos += cnt;
	
	/* Auto-rewind on EOF mode? */
	/* ! */
	/* If we reached at the end of the buffer, reset f_pos */
	if (*f_pos == state->buf_lim)
		*f_pos = 0;
out:
	/* ! */
	/* Unlock? */
	/* Release the seamphore to be used by the next process 
	 * probably waiting */
	up(&state->lock);
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{	
	struct lunix_chrdev_state_struct *state;
	unsigned long page;
	state = filp->private_data;

	if (vma->vm_pgoff)
		return -EINVAL;

	page = virt_to_phys((unsigned long *)state->sensor->msr_data[state->type]) >> PAGE_SHIFT;
		
	if (remap_pfn_range(vma, vma->vm_start, page,
			    vma->vm_end - vma->vm_start,
			    vma->vm_page_prot))
		return -EAGAIN;	
	
	return 0;
}

static struct file_operations lunix_chrdev_fops = 
{
        .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

int lunix_chrdev_init(void)
{
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	/* For every sensor we want at least 3 minor numbers,
	   the measurement info is contained at the 3 LSB
	   of the minor number, and the region must be
	   consecutive, so we end up with 16 << 3 minor numbers */
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
	
	debug("initializing character device\n");
	
	/* We initialize the global cdev structure, specifying
	   the file operations right above to be used */
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);
	lunix_chrdev_cdev.owner = THIS_MODULE;
	
	/* Produce a Device ID for the pair (Major = 60, Minor = 0) */
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	
	/* ! */
	/* register_chrdev_region? */
	/* We register the wanted range, starting from (Major = 60, Minor = 0), up
	   to (Major = 60, Minor = 16 << 3) */
	ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix");
	
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}
		
	/* ! */
	/* cdev_add? */
	/* After the above registration, we are ready to add the
	   char device for the corresponding cdev structure and
	   the defined range */
	ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);
	
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
		
	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}

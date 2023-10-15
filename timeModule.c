#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikulin Daniil");
MODULE_DESCRIPTION("My first module. Write time once a minute to /tmp/current_time file"); 

static struct timer_list timer;
static struct file* fptr;
static int pos = 0;

struct file* file_open(const char *path, int flags, int rights) 
{
    struct file* filp = NULL;
    int err = 0;

    filp = filp_open(path, flags, rights);
    if (IS_ERR(filp)) 
    {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) 
{
    filp_close(file, NULL);
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    int ret;
    ret = kernel_write(file, data, size, &offset);
    return ret;
}

void timer_callback(struct timer_list *timer) {
    struct timespec64 now;
    ktime_get_real_ts64(&now);
    struct tm tm_now;

	//not universal way to get properly time zone
	time64_to_tm(now.tv_sec, 3 * 60 * 60, &tm_now);

    fptr = file_open("/tmp/current_time", O_CREAT | O_WRONLY, 0644);
    if (fptr != NULL)
    {
		char buffer[10];
		sprintf(buffer, "%d:%d\n", tm_now.tm_hour, tm_now.tm_min);
		pos = pos + file_write(fptr, pos, buffer, strlen(buffer)); 
        file_close(fptr);
    }
    else
    {
        pr_err("cannot open file to write\n");
    }

    mod_timer(timer, jiffies + msecs_to_jiffies(60000));
}

static int init_function(void) {
    pr_info("init timeModule\n");

    timer_setup(&timer, timer_callback, 0);
    mod_timer(&timer, jiffies + msecs_to_jiffies(60000));

    return 0;
}

static void exit_function(void) {
    pr_info("exit from timeModule\n");
    del_timer(&timer);
    kfree(fptr);
}

module_init(init_function);
module_exit(exit_function);

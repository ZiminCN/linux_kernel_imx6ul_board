/* -----------Include----------- */
#include <linux/fb.h>
#include <linux/types.h> 
#include <linux/kernel.h> 
#include <linux/delay.h> 
#include <linux/ide.h> 
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/errno.h> 
#include <linux/gpio.h> 
#include <linux/cdev.h>
#include <linux/spi/spi.h>
#include <linux/ioctl.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>    
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>




/* 设备名字 */
#define DEVICE_NAME	"ipslcd_device" 	

/* 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏 */
#define USE_HORIZONTAL 0        

#if (USE_HORIZONTAL == 0) || (USE_HORIZONTAL == 1)
#define LCD_WIDE 240
#define LCD_HIGH 240
#else
#define LCD_WIDE 240
#define LCD_HIGH 240
#endif
#define RGB565_MEM_SIZE (16 / 8)


static struct fb_info *fb_tft;
static struct task_struct *fb_tft_thread;
static struct spi_device *fb_tft_spi;

struct device_node *lcd_node;
int lcd_res_pin;
int lcd_dc_pin;
int lcd_cs_pin;
int lcd_blk_pin;
uint32_t horizontal, lcd_w, lcd_h, lcd_pixel_bits;
uint32_t pseudo_palette[16];


static void LCD_WR_REG(uint8_t reg)
{
  gpio_set_value(lcd_cs_pin, 0);
  gpio_set_value(lcd_dc_pin, 0); // 低电平，命令
  spi_write(fb_tft_spi, &reg, 1);
  gpio_set_value(lcd_cs_pin, 1);
}

static void LCD_WR_DATA8(uint8_t data)
  {   
  gpio_set_value(lcd_cs_pin, 0);
  gpio_set_value(lcd_dc_pin, 1);  // 高电平，数据
  spi_write(fb_tft_spi, &data, 1); 
  gpio_set_value(lcd_cs_pin, 1);
}

static void LCD_WR_DATA(uint16_t data)
{   
  uint8_t buffer[2];
  buffer[0] = (uint8_t) (data >> 8);
  buffer[1] = (uint8_t) (data);

  gpio_set_value(lcd_cs_pin, 0);
  gpio_set_value(lcd_dc_pin, 1);  // 高电平，数据
  spi_write(fb_tft_spi, buffer, 2); 
  gpio_set_value(lcd_cs_pin, 1);
}

static void LCD_WR_BUFFER(uint8_t *buffer, uint32_t length)
{
  gpio_set_value(lcd_cs_pin, 0);
  gpio_set_value(lcd_dc_pin, 1);  // 高电平，数据
  spi_write(fb_tft_spi, buffer, length); 
  gpio_set_value(lcd_cs_pin, 1);
}


static void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
  if (horizontal == 0)
  {
    LCD_WR_REG(0x2a);	// 列地址设置
    LCD_WR_DATA(x1 + 0);
    LCD_WR_DATA(x2 + 0);
    LCD_WR_REG(0x2b); // 行地址设置
    LCD_WR_DATA(y1 + 0);
    LCD_WR_DATA(y2 + 0);
    LCD_WR_REG(0x2c); // 储存器写
  }
  else if (horizontal == 1)
  {
    LCD_WR_REG(0x2a); // 列地址设置
    LCD_WR_DATA(x1 + 0);
    LCD_WR_DATA(x2 + 0);
    LCD_WR_REG(0x2b); // 行地址设置
    LCD_WR_DATA(y1 + 0);
    LCD_WR_DATA(y2 + 0);
    LCD_WR_REG(0x2c); // 储存器写
  }
  else if (horizontal == 2)
  {
    LCD_WR_REG(0x2a); // 列地址设置
    LCD_WR_DATA(x1 + 0);
    LCD_WR_DATA(x2 + 0);
    LCD_WR_REG(0x2b); // 行地址设置
    LCD_WR_DATA(y1 + 0);
    LCD_WR_DATA(y2 + 0);
    LCD_WR_REG(0x2c); // 储存器写
  }
  else
  {
    LCD_WR_REG(0x2a); // 列地址设置
    LCD_WR_DATA(x1 + 0);
    LCD_WR_DATA(x2 + 0);
    LCD_WR_REG(0x2b); // 行地址设置
    LCD_WR_DATA(y1 + 0);
    LCD_WR_DATA(y2 + 0);
    LCD_WR_REG(0x2c); // 储存器写
  }
}


static void fb_tft_init(struct spi_device *spi)
{
  gpio_set_value(lcd_res_pin, 1); 
  gpio_set_value(lcd_dc_pin, 1); 
  gpio_set_value(lcd_cs_pin, 1); 
  msleep(50);
  gpio_set_value(lcd_res_pin, 0); //设低电平，开始复位
  msleep(50);
  gpio_set_value(lcd_res_pin, 1); //设高电平，复位完成
  msleep(50);
  gpio_set_value(lcd_blk_pin, 1); //设高电平，开背光

  /* 写寄存器，初始化 */
  LCD_WR_REG(0x36); 
  if (horizontal == 0)
  {
    LCD_WR_DATA8(0x00);
  }
  else if (horizontal == 1)
  {
    LCD_WR_DATA8(0xC0);
  }
  else if (horizontal == 2)
  {
    LCD_WR_DATA8(0x70);
  }
  else  
  {
    LCD_WR_DATA8(0xA0);
  }
	
  LCD_WR_REG(0x3A);
  LCD_WR_DATA8(0x05);

  LCD_WR_REG(0xB2);
  LCD_WR_DATA8(0x0C);
  LCD_WR_DATA8(0x0C);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x33);
  LCD_WR_DATA8(0x33); 

  LCD_WR_REG(0xB7); 
  LCD_WR_DATA8(0x35);  

  LCD_WR_REG(0xBB);
  LCD_WR_DATA8(0x19);

  LCD_WR_REG(0xC0);
  LCD_WR_DATA8(0x2C);

  LCD_WR_REG(0xC2);
  LCD_WR_DATA8(0x01);

  LCD_WR_REG(0xC3);
  LCD_WR_DATA8(0x12);   

  LCD_WR_REG(0xC4);
  LCD_WR_DATA8(0x20);  

  LCD_WR_REG(0xC6); 
  LCD_WR_DATA8(0x0F);    

  LCD_WR_REG(0xD0); 
  LCD_WR_DATA8(0xA4);
  LCD_WR_DATA8(0xA1);

  LCD_WR_REG(0xE0);
  LCD_WR_DATA8(0xD0);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x11);
  LCD_WR_DATA8(0x13);
  LCD_WR_DATA8(0x2B);
  LCD_WR_DATA8(0x3F);
  LCD_WR_DATA8(0x54);
  LCD_WR_DATA8(0x4C);
  LCD_WR_DATA8(0x18);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x0B);
  LCD_WR_DATA8(0x1F);
  LCD_WR_DATA8(0x23);

  LCD_WR_REG(0xE1);
  LCD_WR_DATA8(0xD0);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x0C);
  LCD_WR_DATA8(0x11);
  LCD_WR_DATA8(0x13);
  LCD_WR_DATA8(0x2C);
  LCD_WR_DATA8(0x3F);
  LCD_WR_DATA8(0x44);
  LCD_WR_DATA8(0x51);
  LCD_WR_DATA8(0x2F);
  LCD_WR_DATA8(0x1F);
  LCD_WR_DATA8(0x1F);
  LCD_WR_DATA8(0x20);
  LCD_WR_DATA8(0x23);

  LCD_WR_REG(0x21); 

  LCD_WR_REG(0x11); 
  msleep(150);
  LCD_WR_REG(0x29);
  msleep(50);
}

uint8_t mybuff[LCD_WIDE * LCD_HIGH * RGB565_MEM_SIZE];

void fb_refresh(struct fb_info *fbi, struct spi_device *spi)
{
  uint32_t i = 0;
  uint8_t temp_u8[2];

  uint16_t *address = (uint16_t *)(fbi->screen_base);

  LCD_Address_Set(0 , 0, lcd_w - 1, lcd_h -1);

  for (i = 0; i < LCD_WIDE * LCD_HIGH; i++)
  {
    temp_u8[0] = address[i] >> 8;
    temp_u8[1] = 0x00FF & address[i];

    mybuff[i * 2 + 0] = temp_u8[0];
    mybuff[i * 2 + 1] = temp_u8[1]; 
  }

  LCD_WR_BUFFER((uint8_t *)(mybuff), LCD_WIDE * LCD_HIGH * RGB565_MEM_SIZE);
}


int thread_func_fb(void *data)
{
  struct fb_info *fbi = (struct fb_info *)data;

  while (1)
  {
    if (kthread_should_stop())
	  {
      break;
	  }
    fb_refresh(fbi, fb_tft_spi);
  }
  return 0;
}


static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
  chan &= 0xffff;
  chan >>= 16 - bf->length;
  return chan << bf->offset;
}


static int tft_setcolreg(uint32_t regno, uint32_t red,uint32_t green, uint32_t blue,uint32_t transp, struct fb_info *info)
{
  unsigned int val;
  
  if (regno > 16)
  {
    return 1;
  }
  val  = chan_to_field(red, &info->var.red);
  val |= chan_to_field(green, &info->var.green);
  val |= chan_to_field(blue, &info->var.blue);
  pseudo_palette[regno] = val;
  
  return 0;
}



static struct fb_ops fb_tft_ops = 
{
  .owner          = THIS_MODULE,
  .fb_write       = fb_sys_write,
  .fb_setcolreg   = tft_setcolreg,  	  /* 设置颜色寄存器 */
  .fb_fillrect    = sys_fillrect,  			/* 用像素行填充矩形框，通用库函数 */
  .fb_copyarea    = sys_copyarea,  			/* 将屏幕的一个矩形区域复制到另一个区域，通用库函数 */
  .fb_imageblit   = sys_imageblit, 			/* 显示一副图像，通用库函数 */
};


static void fb_tft_update(struct fb_info *fbi, struct list_head *pagelist)
{
  
}


struct fb_deferred_io fb_tft_defio = 
{
  .delay        = HZ/60,
  .deferred_io  = &fb_tft_update,
};


struct fb_var_screeninfo fb_tft_var = 
{
  .rotate         = 0,
  .xres           = LCD_WIDE,
  .yres           = LCD_HIGH,
  .xres_virtual   = LCD_WIDE,
  .yres_virtual   = LCD_HIGH,
  .bits_per_pixel = 16,
  .nonstd         = 1,
  /* RGB565 */
  .red.offset     = 11,
  .red.length     = 5,
  .green.offset   = 5,
  .green.length   = 6,
  .blue.offset    = 0,
  .blue.length    = 5,
  .transp.offset  = 0,
  .transp.length  = 0,
  .activate       = FB_ACTIVATE_NOW,
  .vmode          = FB_VMODE_NONINTERLACED,
};

struct fb_fix_screeninfo fb_tft_fix = 
{
  .type           = FB_TYPE_PACKED_PIXELS,
  .visual         = FB_VISUAL_TRUECOLOR,
  .line_length    = LCD_WIDE * RGB565_MEM_SIZE,
  .accel          = FB_ACCEL_NONE,  // 没有使用硬件加速
  .id             = "fb_tft",
};



/**
  * @brief  驱动入口函数
  * @note   无
  * @param  spi: SPI设备结构体
  * @retval 0 成功；其他 失败
  */
static int device_probe(struct spi_device *spi)
{
  int ret;
  uint8_t *addr = NULL;
  uint32_t length;

  /* 查找设备树中的LCD节点 */
  lcd_node = of_find_compatible_node(NULL, NULL, "ipslcd-gpios");
  if (lcd_node == NULL) 
  {
    printk(KERN_ERR"IPSLCD node cant not found!\r\n");
    return -EINVAL;
  } 
  else 
  {
    printk(KERN_ERR"IPSLCD node has been found!\r\n");
  }

  /* RES脚 */
  lcd_res_pin = of_get_named_gpio(lcd_node, "res-gpio", 0);
  if (lcd_res_pin < 0) 
  {
    printk(KERN_ERR"can't get lcd_res_pin!\r\n");
    return -EINVAL;
  }
  ret = gpio_direction_output(lcd_res_pin, 1);
  if (ret < 0) 
  {
    printk(KERN_ERR"can't set lcd_res_pin!\r\n");
  return -EINVAL;
  }

  /* DC脚 */
  lcd_dc_pin = of_get_named_gpio(lcd_node, "dc-gpio", 0);
  if (lcd_dc_pin < 0) 
  {
    printk(KERN_ERR"can't get lcd_dc_pin!\r\n");
    return -EINVAL;
  }
  ret = gpio_direction_output(lcd_dc_pin, 1);
  if (ret < 0) 
  {
    printk(KERN_ERR"can't set lcd_dc_pin!\r\n");
  return -EINVAL;
  }

  /* CS脚 */
  lcd_cs_pin = of_get_named_gpio(lcd_node, "cs-gpio", 0);
  if (lcd_cs_pin < 0) 
  {
    printk(KERN_ERR"can't get lcd_cs_pin!\r\n");
    return -EINVAL;
  }
  ret = gpio_direction_output(lcd_cs_pin, 1);
  if (ret < 0) 
  {
    printk(KERN_ERR"can't set lcd_cs_pin!\r\n");
    return -EINVAL;
  }

  /* BLK脚 */
  lcd_blk_pin = of_get_named_gpio(lcd_node, "blk-gpio", 0);
  if (lcd_blk_pin < 0) 
  {
    printk(KERN_ERR"can't get lcd_blk_pin!\r\n");
    return -EINVAL;
  }
  ret = gpio_direction_output(lcd_blk_pin, 1);
  if (ret < 0) 
  {
    printk(KERN_ERR"can't set lcd_blk_pin!\r\n");
    return -EINVAL;
  }

  /* 从设备树读参数 */
  ret = of_property_read_u32(lcd_node, "horizontal", &horizontal);
  if (ret < 0)
  {
    printk(KERN_ERR"can't read horizontal!\r\n");
    return -EINVAL;
  }

  /* 读颜色格式 */
  ret = of_property_read_u32(lcd_node, "bits-per-pixel", &lcd_pixel_bits);
  if (ret < 0)
  {
    printk(KERN_ERR"can't read bits-per-pixel!\r\n");
    return -EINVAL;
  }
  ret = of_property_read_u32(lcd_node, "hactive", &lcd_w);
  if (ret < 0)
  {
    printk(KERN_ERR"can't read hactive!\r\n");
    return -EINVAL;
  }
  ret = of_property_read_u32(lcd_node, "vactive", &lcd_h);
  if (ret < 0)
  {
    printk(KERN_ERR"can't read vactive!\r\n");
    return -EINVAL;
  }

  /* 向内核申请fb_info结构体 */
  fb_tft = framebuffer_alloc(sizeof(struct fb_info), &spi->dev);
  if (fb_tft == NULL)
  {
    printk(KERN_ERR"Failed framebuffer_alloc!\r\n");
    return -EINVAL;
  }

  /* 激活SPI */
  spi_setup(spi);

  /* 保存SPI结构体的地址 */
  fb_tft_spi = spi;
  
  /* 底层操作 */
  fb_tft->fbops = &fb_tft_ops;

  /* 分配Framebuffer显存 */
  length = lcd_w * lcd_h * (lcd_pixel_bits / 8);
  addr = kmalloc(length, GFP_KERNEL);
  if (addr == NULL)
  {
    printk(KERN_ERR"Failed kmalloc!\r\n");
    return -EINVAL;
  }

  /* var信息 */
  fb_tft_var.bits_per_pixel = lcd_pixel_bits;
  fb_tft_var.xres = lcd_w;
  fb_tft_var.yres = lcd_h;
  fb_tft_var.xres_virtual = lcd_w;
  fb_tft_var.yres_virtual = lcd_h;

  /* fix信息 */
  fb_tft_fix.line_length = lcd_w * (lcd_pixel_bits / 8);
  fb_tft_fix.smem_start = (uint32_t)addr;
  fb_tft_fix.smem_len = length;

  fb_tft->pseudo_palette = pseudo_palette;

  fb_tft->var = fb_tft_var;
  fb_tft->fix = fb_tft_fix;

  fb_tft->screen_base = addr;
  fb_tft->screen_size = length;

  memset((uint16_t *)fb_tft->screen_base, 0x00, fb_tft->screen_size);

  fb_tft_init(fb_tft_spi);

  fb_tft->fbdefio = &fb_tft_defio;

  fb_deferred_io_init(fb_tft);

  /* 注册Framebuffer设备 */
  ret = register_framebuffer(fb_tft);		
  if (ret)
  {
    unregister_framebuffer(fb_tft);
    framebuffer_release(fb_tft);
    printk(KERN_ERR"Failed register_framebuffer!\r\n");
    return -EINVAL;
  }

  fb_tft_thread = kthread_run(thread_func_fb, fb_tft, fb_tft_spi->modalias);

  printk(KERN_ERR"IPSLCD device driver has been installed!\r\n");

  return 0;
}


/**
  * @brief  驱动出口函数
  * @note   无
  * @param  spi: SPI设备结构体
  * @retval 0 成功；其他 失败
  */
static int device_remove(struct spi_device *spi)
{
  memset((uint16_t *)fb_tft->fix.smem_start, 0x00, fb_tft->fix.smem_len);

  kthread_stop(fb_tft_thread);

  fb_deferred_io_cleanup(fb_tft);
  unregister_framebuffer(fb_tft);
  framebuffer_release(fb_tft);
  return 0;
}


/* 匹配列表 */
static const struct of_device_id device_imx_dt_ids[] = 
{
  { .compatible = "ipslcd-gpios" },
  {	},
};


/* platform驱动结构体 */
static struct spi_driver device_driver = 
{
  .driver = 
  {
    .owner          = THIS_MODULE,
    .name           = DEVICE_NAME,       /* 驱动名字，用于和设备匹配 */
    .of_match_table	= device_imx_dt_ids, /* 设备树匹配表 */
  },
  .probe  = device_probe,
  .remove = device_remove,
};

module_spi_driver(device_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dazen");
MODULE_DESCRIPTION("ipslcd_driver,st7789v");




struct gpio_regs {
	uint32_t out;         /* 0x60000300 */
	uint32_t out_w1ts;    /* 0x60000304 */
	uint32_t out_w1tc;    /* 0x60000308 */
	uint32_t enable;      /* 0x6000030C */
	uint32_t enable_w1ts; /* 0x60000310 */
	uint32_t enable_w1tc; /* 0x60000314 */
	uint32_t in;          /* 0x60000318 */
	uint32_t status;      /* 0x6000031C */
	uint32_t status_w1ts; /* 0x60000320 */
	uint32_t status_w1tc; /* 0x60000324 */
};

static struct gpio_regs* gpio_r = (struct gpio_regs*)(0x60000300);

/*
 * usb_serial.c — USB CDC-ACM (Virtual COM port) สำหรับ STM32F103 แบบ bare metal
 * ไม่ต้องแก้ไฟล์นี้ — ใช้งานผ่านฟังก์ชันใน usb_serial.h
 *
 * Endpoints: EP0 control 64 · EP1 IN bulk 64 · EP2 OUT bulk 64 · EP3 IN interrupt 8
 * Register map: RM0008 ch.23 (USB full-speed device), ch.7 (RCC)
 */
#include <stdint.h>
#include "usb_serial.h"

#define R32(a) (*(volatile uint32_t *)(a))

/* ---------- RCC / FLASH / GPIO / core ---------- */
#define RCC_CR       R32(0x40021000)
#define RCC_CFGR     R32(0x40021004)
#define RCC_APB2ENR  R32(0x40021018)
#define RCC_APB1ENR  R32(0x4002101C)
#define FLASH_ACR    R32(0x40022000)
#define GPIOA_CRH    R32(0x40010804)
#define GPIOA_BRR    R32(0x40010814)
#define SCB_VTOR     R32(0xE000ED08)
#define NVIC_ISER0   R32(0xE000E100)
#define NVIC_ICER0   R32(0xE000E180)
#define USB_IRQN     20u                       /* USB_LP_CAN1_RX0 */
#define UID_BASE     0x1FFFF7E8u

/* ---------- USB peripheral ---------- */
#define USB_BASE     0x40005C00u
#define EPR(n)       R32(USB_BASE + 4u * (n))
#define USB_CNTR     R32(USB_BASE + 0x40u)
#define USB_ISTR     R32(USB_BASE + 0x44u)
#define USB_DADDR    R32(USB_BASE + 0x4Cu)
#define USB_BTABLE   R32(USB_BASE + 0x50u)
#define PMA_BASE     0x40006000u
/* buffer table entry: field 0 ADDR_TX, 2 COUNT_TX, 4 ADDR_RX, 6 COUNT_RX */
#define BT(ep, f)    R32(PMA_BASE + ((ep) * 8u + (f)) * 2u)

#define CNTR_CTRM    0x8000u
#define CNTR_RESETM  0x0400u
#define ISTR_CTR     0x8000u
#define ISTR_RESET   0x0400u

#define EP_CTR_RX    0x8000u
#define EP_DTOG_RX   0x4000u
#define EP_STAT_RX   0x3000u
#define EP_SETUP     0x0800u
#define EP_CTR_TX    0x0080u
#define EP_DTOG_TX   0x0040u
#define EP_STAT_TX   0x0030u
#define EP_REG_MASK  0x8F8Fu                   /* CTR_RX|SETUP|TYPE|KIND|CTR_TX|EA */

#define T_BULK       0x0000u
#define T_CONTROL    0x0200u
#define T_INTERRUPT  0x0600u
#define RX_DIS 0x0000u
#define RX_STALL 0x1000u
#define RX_NAK 0x2000u
#define RX_VALID 0x3000u
#define TX_DIS 0x0000u
#define TX_STALL 0x0010u
#define TX_NAK 0x0020u
#define TX_VALID 0x0030u

/* packet memory layout (byte offsets in PMA) */
#define PMA_EP0_TX   0x040u
#define PMA_EP0_RX   0x080u
#define PMA_EP1_TX   0x0C0u
#define PMA_EP2_RX   0x100u
#define PMA_EP3_TX   0x140u
#define RX_CNT_64    0x8400u                   /* BL_SIZE=1, NUM_BLOCK=1 -> 64 bytes */

/* -nostdlib: the compiler may emit these; weak so main.c may define its own */
__attribute__((weak)) void *memset(void *d, int c, unsigned n) { uint8_t *p = d; while (n--) *p++ = (uint8_t)c; return d; }
__attribute__((weak)) void *memcpy(void *d, const void *s, unsigned n) { uint8_t *p = d; const uint8_t *q = s; while (n--) *p++ = *q++; return d; }

/* ======================= descriptors ======================= */
static const uint8_t dev_desc[18] = {
  18, 1, 0x00, 0x02, 0x02, 0x00, 0x00, 64,
  0x83, 0x04, 0x40, 0x57,                      /* VID 0x0483 / PID 0x5740 (ST Virtual COM Port) */
  0x00, 0x02, 1, 2, 3, 1
};
static const uint8_t cfg_desc[67] = {
  9, 2, 67, 0, 2, 1, 0, 0x80, 50,              /* config: 2 interfaces, bus powered, 100 mA */
  9, 4, 0, 0, 1, 0x02, 0x02, 0x01, 0,          /* IF0: CDC communication (ACM, AT commands) */
  5, 0x24, 0x00, 0x10, 0x01,                   /* header 1.10 */
  5, 0x24, 0x01, 0x00, 1,                      /* call management -> data IF1 */
  4, 0x24, 0x02, 0x02,                         /* ACM: line coding + control line state */
  5, 0x24, 0x06, 0, 1,                         /* union: master IF0, slave IF1 */
  7, 5, 0x83, 0x03, 8, 0, 255,                 /* EP3 IN interrupt */
  9, 4, 1, 0, 2, 0x0A, 0x00, 0x00, 0,          /* IF1: CDC data */
  7, 5, 0x02, 0x02, 64, 0, 0,                  /* EP2 OUT bulk */
  7, 5, 0x81, 0x02, 64, 0, 0                   /* EP1 IN bulk */
};
static const char str_manuf[] = "UnityMbed";
static const char str_prod[]  = "UnityMbed Blue Pill Serial";

/* ======================= state ======================= */
enum { ST_IDLE, ST_DATA_IN, ST_DATA_OUT, ST_STATUS_IN, ST_STATUS_OUT };
static uint8_t ep0_buf[64];
static const uint8_t *ep0_ptr;
static uint16_t ep0_left;
static uint8_t ep0_zlp, ep0_state, pending_addr;
static uint8_t line_coding[7] = {0x00, 0xC2, 0x01, 0x00, 0, 0, 8};   /* 115200 8N1 */
static volatile uint8_t configured, dtr;

#define TXN 256u
#define RXN 128u
static volatile uint8_t txb[TXN], rxb[RXN];
static volatile uint16_t tx_head, tx_tail, rx_head, rx_tail;
static volatile uint8_t tx_busy, tx_last, tx_jam, rx_paused;

/* ======================= low level ======================= */
static void irq_off(void) { NVIC_ICER0 = 1u << USB_IRQN; __asm volatile("dsb\n isb" ::: "memory"); }
static void irq_on(void)  { NVIC_ISER0 = 1u << USB_IRQN; }

static void pma_write(uint16_t off, const uint8_t *b, uint16_t n) {
  volatile uint32_t *p = (volatile uint32_t *)(PMA_BASE + off * 2u);
  uint16_t i;
  for (i = 0; i < n; i += 2u) *p++ = (uint32_t)b[i] | ((i + 1u < n) ? (uint32_t)b[i + 1u] << 8 : 0u);
}
static void pma_read(uint16_t off, uint8_t *b, uint16_t n) {
  volatile uint32_t *p = (volatile uint32_t *)(PMA_BASE + off * 2u);
  uint16_t i;
  for (i = 0; i < n; i += 2u) {
    uint32_t w = *p++;
    b[i] = (uint8_t)w;
    if (i + 1u < n) b[i + 1u] = (uint8_t)(w >> 8);
  }
}
/* STAT and DTOG bits toggle when written 1; CTR bits clear when written 0 */
static void ep_tx_stat(uint8_t ep, uint16_t s) { uint32_t r = EPR(ep) & (EP_REG_MASK | EP_STAT_TX); EPR(ep) = (r ^ s) | EP_CTR_RX | EP_CTR_TX; }
static void ep_rx_stat(uint8_t ep, uint16_t s) { uint32_t r = EPR(ep) & (EP_REG_MASK | EP_STAT_RX); EPR(ep) = (r ^ s) | EP_CTR_RX | EP_CTR_TX; }
static void ep_clr_rx(uint8_t ep) { EPR(ep) = (EPR(ep) & EP_REG_MASK & ~EP_CTR_RX) | EP_CTR_TX; }
static void ep_clr_tx(uint8_t ep) { EPR(ep) = (EPR(ep) & EP_REG_MASK & ~EP_CTR_TX) | EP_CTR_RX; }
static void ep_init(uint8_t ep, uint16_t type, uint16_t rx, uint16_t tx) {
  uint32_t r = EPR(ep);
  EPR(ep) = type | ep | (r & (EP_DTOG_RX | EP_DTOG_TX)) | ((r & EP_STAT_RX) ^ rx) | ((r & EP_STAT_TX) ^ tx);
}

/* ======================= EP0 control ======================= */
static void ep0_chunk(void) {
  uint16_t n = ep0_left > 64u ? 64u : ep0_left;
  pma_write(PMA_EP0_TX, ep0_ptr, n);
  BT(0, 2) = n;
  ep0_ptr += n; ep0_left -= n;
  ep_tx_stat(0, TX_VALID);
}
static void ep0_send(const uint8_t *d, uint16_t len, uint16_t wlen) {
  if (len > wlen) len = wlen;
  ep0_ptr = d; ep0_left = len;
  ep0_zlp = (len < wlen) && ((len & 63u) == 0u) && len;
  ep0_state = ST_DATA_IN;
  ep0_chunk();
}
static void ep0_status_in(void) { ep0_left = 0; ep0_zlp = 0; ep0_state = ST_STATUS_IN; BT(0, 2) = 0; ep_tx_stat(0, TX_VALID); }
static void ep0_stall(void) { ep0_state = ST_IDLE; ep_tx_stat(0, TX_STALL); ep_rx_stat(0, RX_STALL); }

static uint16_t str_desc(const char *s) {          /* ASCII -> UTF-16LE string descriptor */
  uint16_t n = 2;
  while (*s && n < 62u) { ep0_buf[n++] = (uint8_t)*s++; ep0_buf[n++] = 0; }
  ep0_buf[0] = (uint8_t)n; ep0_buf[1] = 3;
  return n;
}
static uint16_t serial_desc(void) {                /* unique per chip -> stable COM number */
  static const char hex[] = "0123456789ABCDEF";
  uint16_t n = 2; uint32_t i;
  for (i = 0; i < 12u; i++) {
    uint8_t b = *(volatile uint8_t *)(UID_BASE + i);
    ep0_buf[n++] = (uint8_t)hex[b >> 4];   ep0_buf[n++] = 0;
    ep0_buf[n++] = (uint8_t)hex[b & 15u];  ep0_buf[n++] = 0;
  }
  ep0_buf[0] = (uint8_t)n; ep0_buf[1] = 3;
  return n;
}

static void configure_data_eps(void) {
  ep_init(1, T_BULK, RX_DIS, TX_NAK);
  ep_init(2, T_BULK, RX_VALID, TX_DIS);
  ep_init(3, T_INTERRUPT, RX_DIS, TX_NAK);
  tx_busy = 0; tx_last = 0; tx_jam = 0; rx_paused = 0;
  configured = 1;
}

static void handle_setup(void) {
  uint8_t s[8];
  uint16_t wValue, wLength;
  pma_read(PMA_EP0_RX, s, 8);
  wValue = (uint16_t)(s[2] | (s[3] << 8));
  wLength = (uint16_t)(s[6] | (s[7] << 8));
  ep0_state = ST_IDLE;

  if ((s[0] & 0x60u) == 0x00u) {                  /* standard request */
    switch (s[1]) {
    case 6:                                        /* GET_DESCRIPTOR */
      switch (wValue >> 8) {
      case 1: ep0_send(dev_desc, sizeof dev_desc, wLength); break;
      case 2: ep0_send(cfg_desc, sizeof cfg_desc, wLength); break;
      case 3:
        if ((wValue & 0xFFu) == 0) { ep0_buf[0] = 4; ep0_buf[1] = 3; ep0_buf[2] = 0x09; ep0_buf[3] = 0x04; ep0_send(ep0_buf, 4, wLength); }
        else if ((wValue & 0xFFu) == 1) ep0_send(ep0_buf, str_desc(str_manuf), wLength);
        else if ((wValue & 0xFFu) == 2) ep0_send(ep0_buf, str_desc(str_prod), wLength);
        else if ((wValue & 0xFFu) == 3) ep0_send(ep0_buf, serial_desc(), wLength);
        else { ep0_stall(); return; }
        break;
      default: ep0_stall(); return;                /* device qualifier etc. -> full-speed only */
      }
      break;
    case 5: pending_addr = (uint8_t)(wValue & 0x7Fu); ep0_status_in(); break;       /* SET_ADDRESS */
    case 9:                                                                         /* SET_CONFIGURATION */
      if ((wValue & 0xFFu) == 1u) configure_data_eps(); else configured = 0;
      ep0_status_in(); break;
    case 8: ep0_buf[0] = configured; ep0_send(ep0_buf, 1, wLength); break;         /* GET_CONFIGURATION */
    case 0: ep0_buf[0] = 0; ep0_buf[1] = 0; ep0_send(ep0_buf, 2, wLength); break;  /* GET_STATUS */
    case 10: ep0_buf[0] = 0; ep0_send(ep0_buf, 1, wLength); break;                 /* GET_INTERFACE */
    case 1: case 3: case 11: ep0_status_in(); break;          /* CLEAR/SET_FEATURE, SET_INTERFACE */
    default: ep0_stall(); return;
    }
  } else if ((s[0] & 0x60u) == 0x20u) {           /* CDC class request */
    switch (s[1]) {
    case 0x20: ep0_state = ST_DATA_OUT; break;                                      /* SET_LINE_CODING */
    case 0x21: ep0_send(line_coding, 7, wLength); break;                            /* GET_LINE_CODING */
    case 0x22: dtr = (uint8_t)(wValue & 1u); ep0_status_in(); break;               /* SET_CONTROL_LINE_STATE */
    case 0x23: ep0_status_in(); break;                                              /* SEND_BREAK */
    default: ep0_stall(); return;
    }
  } else { ep0_stall(); return; }
  ep_rx_stat(0, RX_VALID);
}

static void ep0_in_done(void) {
  if (pending_addr) { USB_DADDR = 0x80u | pending_addr; pending_addr = 0; }
  if (ep0_state == ST_DATA_IN) {
    if (ep0_left) ep0_chunk();
    else if (ep0_zlp) { ep0_zlp = 0; BT(0, 2) = 0; ep_tx_stat(0, TX_VALID); }
    else ep0_state = ST_STATUS_OUT;                /* host now sends a zero-length OUT */
  } else {
    ep0_state = ST_IDLE;
  }
}
static void ep0_out_done(void) {
  if (ep0_state == ST_DATA_OUT) {
    uint16_t n = (uint16_t)(BT(0, 6) & 0x3FFu);
    if (n > 7u) n = 7u;
    pma_read(PMA_EP0_RX, line_coding, n);
    ep0_status_in();
  } else {
    ep0_state = ST_IDLE;
  }
  ep_rx_stat(0, RX_VALID);
}

/* ======================= data endpoints ======================= */
static void tx_kick(void) {                       /* call with USB IRQ masked, or from the ISR */
  uint8_t pkt[64]; uint16_t n = 0;
  if (tx_busy || !configured) return;
  while (n < 64u && tx_tail != tx_head) { pkt[n++] = txb[tx_tail]; tx_tail = (uint16_t)((tx_tail + 1u) % TXN); }
  if (n == 0u) {
    if (tx_last == 64u) { BT(1, 2) = 0; tx_last = 0; tx_busy = 1; ep_tx_stat(1, TX_VALID); }  /* ZLP ends the transfer */
    return;
  }
  pma_write(PMA_EP1_TX, pkt, n);
  BT(1, 2) = n; tx_last = (uint8_t)n; tx_busy = 1;
  ep_tx_stat(1, TX_VALID);
}
static uint16_t rx_free(void) { return (uint16_t)(RXN - 1u - ((rx_head + RXN - rx_tail) % RXN)); }
static void ep2_out_done(void) {
  uint8_t pkt[64]; uint16_t n = (uint16_t)(BT(2, 6) & 0x3FFu), i;
  if (n > 64u) n = 64u;
  pma_read(PMA_EP2_RX, pkt, n);
  for (i = 0; i < n; i++) {
    uint16_t nx = (uint16_t)((rx_head + 1u) % RXN);
    if (nx == rx_tail) break;                    /* full: drop the rest */
    rxb[rx_head] = pkt[i]; rx_head = nx;
  }
  if (rx_free() >= 64u) ep_rx_stat(2, RX_VALID); else rx_paused = 1;
}

static void bus_reset(void) {
  USB_BTABLE = 0;
  BT(0, 0) = PMA_EP0_TX; BT(0, 2) = 0; BT(0, 4) = PMA_EP0_RX; BT(0, 6) = RX_CNT_64;
  BT(1, 0) = PMA_EP1_TX; BT(1, 2) = 0;
  BT(2, 4) = PMA_EP2_RX; BT(2, 6) = RX_CNT_64;
  BT(3, 0) = PMA_EP3_TX; BT(3, 2) = 0;
  ep_init(0, T_CONTROL, RX_VALID, TX_NAK);
  ep_init(1, T_BULK, RX_DIS, TX_DIS);
  ep_init(2, T_BULK, RX_DIS, TX_DIS);
  ep_init(3, T_INTERRUPT, RX_DIS, TX_DIS);
  USB_DADDR = 0x80u;                              /* enable, address 0 */
  configured = 0; dtr = 0; pending_addr = 0; ep0_state = ST_IDLE;
  tx_busy = 0; tx_last = 0; tx_jam = 0; rx_paused = 0;
}

static void usb_lp_isr(void) {
  uint32_t istr = USB_ISTR;
  if (istr & ISTR_RESET) { USB_ISTR = (uint16_t)~ISTR_RESET; bus_reset(); return; }
  while ((istr = USB_ISTR) & ISTR_CTR) {
    uint8_t ep = (uint8_t)(istr & 0x0Fu);
    uint32_t r = EPR(ep);
    if (r & EP_CTR_RX) {
      ep_clr_rx(ep);
      if (ep == 0u) { if (r & EP_SETUP) handle_setup(); else ep0_out_done(); }
      else if (ep == 2u) ep2_out_done();
    }
    if (r & EP_CTR_TX) {
      ep_clr_tx(ep);
      if (ep == 0u) ep0_in_done();
      else if (ep == 1u) { tx_busy = 0; tx_jam = 0; tx_kick(); }
    }
  }
}

/* ======================= vector table (moved here by usb_serial_init) =======================
 * main.c ของ Blank project มี vector table แค่ 4 ช่อง จึงย้ายมาใช้ตารางเต็มชุดนี้แทน
 * ชื่อ handler เป็น weak: ถ้า main.c เขียนฟังก์ชันชื่อเดียวกัน (เช่น SysTick_Handler) จะใช้ของ main.c */
static void usbs_default(void) { for (;;) { } }
#define WEAK __attribute__((weak, alias("usbs_default")))
void NMI_Handler(void) WEAK;          void HardFault_Handler(void) WEAK;   void MemManage_Handler(void) WEAK;
void BusFault_Handler(void) WEAK;     void UsageFault_Handler(void) WEAK;  void SVC_Handler(void) WEAK;
void DebugMon_Handler(void) WEAK;     void PendSV_Handler(void) WEAK;      void SysTick_Handler(void) WEAK;
void EXTI0_IRQHandler(void) WEAK;     void EXTI1_IRQHandler(void) WEAK;    void EXTI2_IRQHandler(void) WEAK;
void EXTI3_IRQHandler(void) WEAK;     void EXTI4_IRQHandler(void) WEAK;    void EXTI9_5_IRQHandler(void) WEAK;
void EXTI15_10_IRQHandler(void) WEAK; void ADC1_2_IRQHandler(void) WEAK;
void DMA1_Channel1_IRQHandler(void) WEAK; void DMA1_Channel2_IRQHandler(void) WEAK; void DMA1_Channel3_IRQHandler(void) WEAK;
void DMA1_Channel4_IRQHandler(void) WEAK; void DMA1_Channel5_IRQHandler(void) WEAK; void DMA1_Channel6_IRQHandler(void) WEAK;
void DMA1_Channel7_IRQHandler(void) WEAK;
void TIM1_UP_IRQHandler(void) WEAK;   void TIM1_CC_IRQHandler(void) WEAK;  void TIM2_IRQHandler(void) WEAK;
void TIM3_IRQHandler(void) WEAK;      void TIM4_IRQHandler(void) WEAK;
void I2C1_EV_IRQHandler(void) WEAK;   void I2C1_ER_IRQHandler(void) WEAK;  void SPI1_IRQHandler(void) WEAK;
void USART1_IRQHandler(void) WEAK;    void USART2_IRQHandler(void) WEAK;   void USART3_IRQHandler(void) WEAK;
void RTC_IRQHandler(void) WEAK;

typedef void (*vec_t)(void);
__attribute__((aligned(512))) static const vec_t usbs_vectors[16 + 43] = {
  0, 0, NMI_Handler, HardFault_Handler, MemManage_Handler, BusFault_Handler, UsageFault_Handler,
  0, 0, 0, 0, SVC_Handler, DebugMon_Handler, 0, PendSV_Handler, SysTick_Handler,
  /* IRQ 0.. */
  usbs_default, usbs_default, usbs_default, RTC_IRQHandler, usbs_default, usbs_default,      /* 0-5   WWDG PVD TAMPER RTC FLASH RCC */
  EXTI0_IRQHandler, EXTI1_IRQHandler, EXTI2_IRQHandler, EXTI3_IRQHandler, EXTI4_IRQHandler,   /* 6-10  */
  DMA1_Channel1_IRQHandler, DMA1_Channel2_IRQHandler, DMA1_Channel3_IRQHandler, DMA1_Channel4_IRQHandler,
  DMA1_Channel5_IRQHandler, DMA1_Channel6_IRQHandler, DMA1_Channel7_IRQHandler,             /* 11-17 */
  ADC1_2_IRQHandler,                                                                         /* 18 */
  usbs_default, usb_lp_isr, usbs_default, usbs_default,                                      /* 19-22 USB_HP USB_LP CAN */
  EXTI9_5_IRQHandler, usbs_default, TIM1_UP_IRQHandler, usbs_default, TIM1_CC_IRQHandler,   /* 23-27 */
  TIM2_IRQHandler, TIM3_IRQHandler, TIM4_IRQHandler,                                         /* 28-30 */
  I2C1_EV_IRQHandler, I2C1_ER_IRQHandler, usbs_default, usbs_default,                        /* 31-34 */
  SPI1_IRQHandler, usbs_default,                                                             /* 35-36 */
  USART1_IRQHandler, USART2_IRQHandler, USART3_IRQHandler,                                   /* 37-39 */
  EXTI15_10_IRQHandler, usbs_default, usbs_default                                           /* 40-42 */
};

/* ======================= public API ======================= */
static void spin(uint32_t n) { while (n--) __asm volatile("nop"); }

static int clock_72mhz(void) {
  uint32_t t;
  if ((RCC_CFGR & 0xCu) == 0x8u) return 1;       /* PLL already running the CPU */
  RCC_CR |= 1u << 16;                             /* HSEON (8 MHz crystal) */
  for (t = 0; !(RCC_CR & (1u << 17)); t++) if (t > 400000u) return 0;
  FLASH_ACR = (FLASH_ACR & ~7u) | 0x10u | 2u;     /* prefetch, 2 wait states */
  RCC_CFGR = (7u << 18) | (1u << 16) | (2u << 14) | (4u << 8);   /* PLL x9 from HSE, ADC /6, APB1 /2, USB /1.5 */
  RCC_CR |= 1u << 24;                             /* PLLON */
  while (!(RCC_CR & (1u << 25))) { }
  RCC_CFGR |= 2u;                                 /* SYSCLK = PLL */
  while ((RCC_CFGR & 0xCu) != 0x8u) { }
  return 1;
}

int usb_serial_init(void) {
  if (!clock_72mhz()) return 0;
  /* pull D+ (PA12) low ~10 ms so the PC re-detects the board after every reset/flash */
  RCC_APB2ENR |= 1u << 2;
  GPIOA_CRH = (GPIOA_CRH & ~(0xFu << 16)) | (0x2u << 16);
  GPIOA_BRR = 1u << 12;
  spin(240000u);
  GPIOA_CRH = (GPIOA_CRH & ~(0xFu << 16)) | (0x4u << 16);   /* back to input; USB takes the pin */

  RCC_APB1ENR |= 1u << 23;                        /* USB clock */
  USB_CNTR = 1u;                                  /* FRES=1, PDWN=0 */
  spin(100u);
  USB_CNTR = 0u;
  USB_ISTR = 0u;
  USB_BTABLE = 0u;
  USB_CNTR = CNTR_CTRM | CNTR_RESETM;
  SCB_VTOR = (uint32_t)usbs_vectors;
  __asm volatile("dsb\n isb" ::: "memory");
  irq_on();
  __asm volatile("cpsie i" ::: "memory");
  return 1;
}

int usb_connected(void) { return configured && dtr; }

void usb_write(const uint8_t *d, uint32_t n) {
  while (n--) {
    uint16_t nx = (uint16_t)((tx_head + 1u) % TXN);
    if (nx == tx_tail) {                          /* buffer full */
      uint32_t t = 0;
      if (!configured || tx_jam) return;          /* nobody reading: drop instead of hanging */
      irq_off(); tx_kick(); irq_on();
      while (nx == tx_tail) { if (++t > 300000u) { tx_jam = 1; return; } }
    }
    txb[tx_head] = *d++;
    tx_head = nx;
  }
  irq_off(); tx_kick(); irq_on();
}
void usb_putc(char c) { usb_write((const uint8_t *)&c, 1); }
void usb_print(const char *s) { uint32_t n = 0; while (s[n]) n++; usb_write((const uint8_t *)s, n); }
void usb_print_int(int32_t v) {
  char b[12]; int i = 11; uint32_t u = v < 0 ? (uint32_t)-v : (uint32_t)v;
  b[i] = 0;
  do { b[--i] = (char)('0' + u % 10u); u /= 10u; } while (u);
  if (v < 0) b[--i] = '-';
  usb_print(&b[i]);
}
int usb_read(void) {
  int c;
  if (rx_tail == rx_head) return -1;
  c = rxb[rx_tail];
  rx_tail = (uint16_t)((rx_tail + 1u) % RXN);
  if (rx_paused && rx_free() >= 64u) { irq_off(); rx_paused = 0; ep_rx_stat(2, RX_VALID); irq_on(); }
  return c;
}

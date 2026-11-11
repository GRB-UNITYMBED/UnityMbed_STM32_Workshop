/*
 * usb_serial.h — Serial ผ่าน Micro-USB ของ Blue Pill (USB CDC / Virtual COM)
 * STM32F103C8T6 · bare metal · ไม่ต้องใช้ USB-TTL · Windows 10/11 ไม่ต้องลงไดรเวอร์
 *
 * วิธีใช้: usb_serial.c อยู่ใน src/ · usb_serial.h อยู่ใน inc/ (Makefile ต้องมี -Iinc) แล้วใน main.c
 *     #include "usb_serial.h"
 *     usb_serial_init();          // เรียกบรรทัดแรกของ main()
 *     usb_print("Hello\r\n");
 *
 * สำคัญ: usb_serial_init() ตั้ง clock ใหม่ (USB ต้องใช้คริสตัล 8 MHz + PLL)
 *   SYSCLK = HCLK = 72 MHz · APB1 = 36 MHz (TIM2-4 = 72 MHz) · APB2 = 72 MHz · ADC = 12 MHz
 *   โค้ดที่ตั้งเวลา/baud ต้องคิดจาก 72 MHz เช่น SysTick 1 ms = 72000 - 1
 * Interrupt ใช้ได้ตามปกติ: แค่ตั้งชื่อฟังก์ชันมาตรฐาน เช่น SysTick_Handler, TIM2_IRQHandler
 */
#ifndef USB_SERIAL_H
#define USB_SERIAL_H
#include <stdint.h>

#define USB_SERIAL_CPU_HZ 72000000u

int  usb_serial_init(void);                         /* 1 = ok, 0 = คริสตัลไม่ทำงาน */
void usb_write(const uint8_t *data, uint32_t len);  /* ไม่ค้าง: ถ้ายังไม่มีใครเปิดพอร์ต ข้อมูลจะถูกทิ้ง */
void usb_putc(char c);
void usb_print(const char *s);
void usb_print_int(int32_t v);
int  usb_read(void);                                /* อ่าน 1 ไบต์ที่คอมส่งมา, -1 = ไม่มี */
int  usb_connected(void);                           /* 1 = มีโปรแกรมเปิดพอร์ตอยู่ (DTR) */

#endif

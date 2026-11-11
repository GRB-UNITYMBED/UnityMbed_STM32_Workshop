/*
 * startup.c — ตัวเริ่มระบบของ STM32F103C8T6 (ชุดเดียวกับ Blank project ของ UnityMbed)
 * ไม่ต้องแก้ไฟล์นี้ และไม่ต้องเขียน Reset_Handler / vector table ซ้ำใน main.c
 * (ฟังก์ชันเป็น weak: ถ้า AI เผลอเขียนซ้ำใน main.c ก็ยัง build ผ่านและบูตได้)
 *   Reset_Handler: ก๊อปค่าเริ่มต้นของตัวแปรลง RAM ล้าง .bss แล้วเรียก main()
 *   vector table ชุดเล็ก 4 ช่อง · ถ้าเรียก usb_serial_init() จะย้ายไปใช้ตารางเต็มชุดใน usb_serial.c
 */
#include <stdint.h>

int main(void);

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
__attribute__((weak)) void Reset_Handler(void) {
  uint32_t *src = &_sidata, *dst = &_sdata;
  while (dst < &_edata) *dst++ = *src++;
  for (dst = &_sbss; dst < &_ebss;) *dst++ = 0;
  main();
  for (;;) {
  }
}
__attribute__((weak)) void Default_Handler(void) {
  for (;;) {
  }
}
__attribute__((section(".vectors"), used)) static const uint32_t vectors[] = {
    (uint32_t)&_estack,
    (uint32_t)Reset_Handler,
    (uint32_t)Default_Handler,
    (uint32_t)Default_Handler,
};

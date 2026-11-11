# STM32_Lab_Starter — โปรเจกต์ตั้งต้นสำหรับทุก Lab

Blank project ของ UnityMbed (STM32F103C8T6 Blue Pill · bare metal · ไม่ใช้ HAL) จัดไฟล์แบบโปรเจกต์จริง: `.c` ใน `src/` · `.h` ใน `inc/`

## วิธีใช้
1. Copy โฟลเดอร์นี้ไปไว้ใน `C:\Users\<ชื่อ>\UnityMbed\` แล้วเปลี่ยนชื่อเป็นชื่อ Lab (เช่น `lab2`)
2. UnityMbed IDE → Open Project → เลือกโฟลเดอร์นั้น
3. วาง prompt ของ Lab ใน AI Chat → AI เขียนโค้ดลง `src/main.c` → Build → Flash

## มีอะไรบ้าง
```
src/
  main.c        ว่าง รอให้ AI เขียน  ← แก้ไฟล์นี้ไฟล์เดียว
  startup.c     Reset_Handler + vector table (ไม่ต้องแก้)
  usb_serial.c  Serial ผ่าน Micro-USB (ไม่ต้องแก้)
inc/
  usb_serial.h  ฟังก์ชันที่เรียกได้: usb_serial_init(), usb_print(), usb_print_int() ...
Makefile        เหมือน Blank project + เพิ่ม -Iinc ให้หา .h ใน inc/ เจอ
linker.ld · openocd.cfg · unitymbed.json   ชุดเดียวกับ Blank project
```

## ข้อควรรู้
- Lab ที่ไม่ใช้ Serial ไม่ต้องลบ `usb_serial.c` — ถ้าไม่ได้เรียก ระบบจะตัดทิ้งตอน build ให้เอง
- `usb_serial_init()` ตั้ง CPU เป็น 72 MHz · ถ้าไม่เรียก CPU อยู่ที่ 8 MHz (HSI)
- ดู Serial: เสียบ Micro-USB ของบอร์ด → IDE แท็บ Serial → กด List → เลือก COM ของบอร์ด
- ไม่มีไฟล์ header ของชิป (แบบ `n32g031.h`) โดยตั้งใจ — AI เขียนแอดเดรสรีจิสเตอร์เองด้วย `#define` ถ้ามี header ให้ด้วย AI มักเขียนชื่อซ้ำกับ header จน build ไม่ผ่าน

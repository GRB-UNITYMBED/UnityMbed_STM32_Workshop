# Challenge — แยก Final Project เป็นไฟล์ .c / .h

> **ไม่ใช่ Lab ใหม่:** ทำต่อจากโปรเจกต์ Final Project ของคุณที่ใช้ได้แล้ว (สไลด์ "ต่อยอด: แยกไฟล์ .c / .h") · prompt อยู่ใน `prompt.txt` · ตัวอย่างที่แยกเสร็จแล้ว = `FinalProject_SmartRoom_Modular` ในชุดเฉลย

เป้าหมาย: โค้ดทำงานเหมือน Final Project เดิมทุกอย่าง แต่จัดแบบโปรเจกต์จริง:
ไฟล์ `.c` อยู่ใน `src/` ไฟล์ `.h` อยู่ใน `inc/` แยกตามหน้าที่ — `main.c` เหลือแค่ตรรกะของงาน

```
src/                      inc/
  main.c        งานหลัก     
  board.c       clock 72 MHz, millis/delay, LED      board.h
  button.c      ปุ่ม PA0 + debounce                  button.h
  dht11.c       DHT11 PB12 (จับเวลาด้วย DWT)          dht11.h
  i2c.c         I2C1 PB6/PB7                         i2c.h
  oled.c        จอ SSD1306 + ฟอนต์                    oled.h
  anomaly.c     เรียนรู้ Mean/SD + คะแนนผิดปกติ        anomaly.h
  regression.c  Linear Regression (Q16)              regression.h
  usb_serial.c  Serial ผ่าน Micro-USB                  usb_serial.h
  startup.c     Reset_Handler + vector table
                                                     stm32f103.h  แอดเดรสรีจิสเตอร์
```

## ทำไมต้องแยกไฟล์
- อ่านง่าย: อยากแก้จอ เปิด `oled.c` ไฟล์เดียว
- ใช้ซ้ำได้: โปรเจกต์ใหม่ที่ใช้จอ OLED แค่ copy `oled.c` + `oled.h` + `i2c.c` + `i2c.h`
- แก้ส่วนหนึ่งไม่กระทบส่วนอื่น: `main.c` เรียกผ่านฟังก์ชันใน `.h` เท่านั้น
- `.h` = "สัญญา" ว่าโมดูลนี้มีฟังก์ชันอะไรให้เรียก · `.c` = วิธีทำงานข้างใน (ฟังก์ชัน `static` ใช้ได้แค่ในไฟล์นั้น)

## สำคัญ: Makefile ต้องรู้จักโฟลเดอร์ inc/
Makefile คอมไพล์ทุกไฟล์ `src/*.c` อยู่แล้ว แต่ต้องมี `-Iinc` ถึงจะหา `.h` ใน `inc/` เจอ
`STM32_Lab_Starter` ใส่ไว้ให้แล้ว (ถ้าเริ่มจาก Blank project ของ IDE ต้องเพิ่มเอง 1 จุด):

```make
CFLAGS  = -mcpu=$(CPU) -mthumb -Os -g -ffreestanding -Wall -Iinc $(CFLAGS_EXTRA)
                                                          ^^^^^ เพิ่มตรงนี้
```

## วิธีทำ: ให้ AI แยกทีละโมดูล
เริ่มจากโปรเจกต์ Final Project ของคุณที่ใช้ได้แล้ว (มาจาก Starter จึงมี `inc/` และ `-Iinc` อยู่แล้ว) แล้วสั่งทีละโมดูล:

> แยกโค้ดจอ OLED และ I2C ออกจาก main.c เป็น src/oled.c, src/i2c.c และ inc/oled.h, inc/i2c.h ให้ main.c include "oled.h" ฟังก์ชันที่ใช้เฉพาะในไฟล์ให้เป็น static ห้ามแก้การทำงาน ตอบทุกไฟล์ครบ ไม่ต้องอธิบาย

Build + Flash ให้ผ่าน แล้วค่อยแยกโมดูลถัดไป (dht11 → button → regression → anomaly)
ถ้าพัง: ย้อนไปไฟล์ของขั้นก่อนหน้า แล้วสั่งแยกโมดูลนั้นใหม่

# ดูค่า Serial ผ่าน Micro-USB (Lab 2 ขึ้นไป)

ไม่ต้องใช้ USB-TTL และ Windows 10/11 ไม่ต้องลงไดรเวอร์

1. ใช้โปรเจกต์ที่ copy จาก `STM32_Lab_Starter` (มี `usb_serial.c` ใน `src/` และ `usb_serial.h` ใน `inc/` แล้ว) แล้ว Build → Flash โค้ดที่เรียก `usb_serial_init()`
2. เสียบ **Micro-USB ของบอร์ด** เข้าคอม (เสียบคู่กับ ST-LINK ได้เลย)
3. Device Manager → **Ports (COM & LPT)** จะเห็น `USB Serial Device (COMx)` จดเลข COM ไว้
4. UnityMbed IDE → แท็บ **Serial** → กด **List** → เลือก COM ของบอร์ด → Connect (baud เลือกค่าไหนก็ได้)

## ปัญหาที่เจอบ่อย
- **"USB device not recognized" / Device Descriptor Request Failed** — บอร์ดยังรันโค้ดที่ไม่มี USB (เช่น Lab 1)
  ต้อง Flash โค้ดที่เรียก `usb_serial_init()` ก่อน แล้วถอด-เสียบ Micro-USB ใหม่ 1 ครั้ง
- **ไม่เห็น COM** — กด List ใหม่ หรือถอด-เสียบ Micro-USB
- **LED PC13 กะพริบถี่ค้าง** — คริสตัล 8 MHz บนบอร์ดไม่ทำงาน ลองเปลี่ยนบอร์ด
- **ไฟกะพริบเร็วผิดปกติ (โค้ดที่ AI เขียน)** — AI คำนวณเวลาจาก 8 MHz ให้บอก AI ว่า CPU เป็น 72 MHz หลัง `usb_serial_init()`

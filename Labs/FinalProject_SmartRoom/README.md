# Final Project — Smart Room Monitor

> **เริ่มยังไง:** โฟลเดอร์นี้เป็นโปรเจกต์เปล่าพร้อมใช้ (`src/main.c` ว่าง · มี `startup.c` `usb_serial` ให้แล้ว) → UnityMbed IDE → **Open Project** → เลือกโฟลเดอร์นี้ → วาง Prompt จาก `prompt.txt` ใน AI Chat → Build → Flash

**อุปกรณ์:** ใช้ของทั้งหมด (DHT11, OLED, ปุ่ม, LED)

## ต่อวงจร

| อุปกรณ์ | ขา Blue Pill |
|---|---|
| DHT11 DATA | PB12 |
| OLED SCL / SDA | PB6 / PB7 |
| ปุ่มกด | PA0 -> GND |
| LED | PC13 (บนบอร์ด) |

## ผลที่ควรเห็น
BOOT -> LEARNING x/15 -> NORMAL หรือ !! ALERT !! พร้อม Temp, Humi, Next (ค่าทำนาย)

## วิธีทำ: สร้างใหม่ทีละขั้น (ทวนการเขียน Prompt)
1. Open Project โฟลเดอร์ Lab นี้ใน IDE (โปรเจกต์เปล่าพร้อมใช้) แล้วเขียน Prompt เองด้วย C.L.E.A.R.
2. Prompt แรก: Boot 2 วิ + Temp Humi บนจอ → Build + Flash
3. Refine: เพิ่มค่าทำนาย Linear Regression → เทสต์
4. Refine: เรียนรู้ความชื้น 15 ค่า + ALERT → เทสต์
5. Refine: ปุ่ม PA0 เรียนใหม่ → เทสต์

ทำไมไม่สั่งทีเดียว: โค้ดยาวจน AI ตัดกลางคัน / ใส่ ``` ปนในไฟล์ — ทีละขั้นหาจุดพังง่ายกว่า

## Prompt
ดู `prompt.txt` — มี Prompt แรก (เหมือนในสไลด์) + Prompt Refine 3 ขั้น

## ถ้าไม่ทำงาน
Copy ข้อความ Error หรือ Log จาก Monitor ไปวางในแชท AI แล้วบอกอาการ เช่น:
> Build แล้วเจอ Error นี้ [วาง Error] ช่วยหาสาเหตุและแก้เฉพาะจุดที่ผิด อธิบายสั้น ๆ ห้ามแก้ส่วนอื่นที่ทำงานได้แล้ว

# UnityMbed × STM32 Workshop — ชุดผู้เรียน

คอร์ส UnityMbed พัฒนาบน STM32 · ThaiEasyElec × GRB · 12 พ.ย. 2569
บอร์ด: **STM32F103C8T6 (Blue Pill)** + ST-LINK V2 · โปรเจกต์ Blank ของ UnityMbed (bare metal, ไม่ใช้ HAL)

## วิธีเรียนแต่ละ Lab
1. Copy โฟลเดอร์ **`STM32_Lab_Starter`** ไปไว้ใน `C:\Users\<ชื่อ>\UnityMbed\` แล้วเปลี่ยนชื่อเป็นชื่อ Lab (เช่น `lab2`) → UnityMbed IDE → Open Project → เลือกโฟลเดอร์นั้น
   (Starter = `main.c` เปล่า · `startup.c` + `usb_serial.c` ใน `src/` · `usb_serial.h` ใน `inc/` ไม่ต้องสร้างโปรเจกต์ ไม่ต้อง Add files)
2. เปิด `Labs/<ชื่อ Lab>/prompt.txt` → Copy prompt ไปวางใน **AI Chat** → AI เขียน `main.c` ให้
3. กด **Build** → **Flash** → ดูผลบนบอร์ด (วิธีต่อวงจรและผลที่ควรเห็นอยู่ใน `Labs/<ชื่อ Lab>/README.md`)
4. ไม่ผ่าน? Copy Error ให้ AI แก้ได้ไม่เกิน 2 รอบ (ดูสไลด์ "แก้โค้ดด้วย AI")
5. ยังไม่ผ่าน → ยกมือ / ขอ **ชุดเฉลย** จากวิทยากร

| โฟลเดอร์ | เนื้อหา | อุปกรณ์เพิ่ม |
|---|---|---|
| `STM32_Lab_Starter` | โปรเจกต์ตั้งต้น — copy ไปใช้ทุก Lab | — |
| `Labs/Lab1_Blink` | ไฟกะพริบ PC13 + ให้ AI อธิบายโค้ด | — |
| `Labs/Lab2_Button_Serial` | ปุ่มกด นับครั้ง LED กะพริบตามจำนวน + ส่งขึ้น Serial | ปุ่ม, สาย Micro-USB |
| `Labs/Lab3_Pot_Servo` | วอลุ่มคุม Servo + ดูค่าดิบ vs ค่าที่กรองใน Serial | Potentiometer, Servo SG90 |
| `Labs/Lab4_DHT11_OLED` | อุณหภูมิ/ความชื้นขึ้นจอ OLED | DHT11, OLED I2C |
| `Labs/Lab5_TinyML_Anomaly` | ชิปเรียนรู้อุณหภูมิปกติ แล้วจับความผิดปกติ (เกิน 1 SD) | + ปุ่ม |
| `Labs/Lab6_TinyML_Regression` | Linear Regression ทำนายอุณหภูมิบนชิป | — |
| `Labs/FinalProject_SmartRoom` | Final Project: Smart Room Monitor — Prompt แรก + Refine ทีละขั้น | ทั้งหมด |
| `Labs/FinalProject_Challenge_SplitFiles` | Challenge (ไม่ใช่ Lab ใหม่): ต่อจาก Final Project ให้ AI แยกโค้ดเป็น .c (src) / .h (inc) | ทั้งหมด |

## ก่อนเริ่ม
- ใช้ UnityMbed **v0.1.27 ขึ้นไป** (Windows อัปเดตในแอปได้ · Linux ใช้ไฟล์ .deb)
- Windows: ลง ST-LINK driver **STSW-LINK009** จาก https://www.st.com/en/development-tools/stsw-link009.html
  (เว็บ ST ให้ล็อกอิน หรือกรอกชื่อ+อีเมลแล้วกดลิงก์ยืนยันในอีเมลก่อนโหลด — ควรลงมาก่อนวันอบรม)
  ถ้าลืมลง Flash จะไม่ผ่าน และแอปจะบอกวิธีแก้พร้อมลิงก์ไว้ใน Output
- Serial ผ่าน Micro-USB: ดู `SETUP_SERIAL.md` (ไม่ต้องใช้ USB-TTL และไม่ต้องลงไดรเวอร์)

## ถ้า Flash ไม่ผ่าน
- Windows ยังไม่ได้ลง STSW-LINK009 → ดูข้อความใน Output แล้วลงตามลิงก์
- ST-LINK V2 รุ่นเก่า (HLA) → เปิด terminal ในโฟลเดอร์ project รัน `unitymbed detect --apply` แล้ว Flash ซ้ำ
- เช็กสาย SWD: 3.3V · GND · SWDIO→DIO · SWCLK→DCLK

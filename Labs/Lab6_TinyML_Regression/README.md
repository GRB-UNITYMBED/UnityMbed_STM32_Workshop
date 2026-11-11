# Lab6 TinyML Regression — TinyML: ทำนายอุณหภูมิด้วย Linear Regression บนชิป

> **เริ่มยังไง:** โฟลเดอร์นี้เป็นโปรเจกต์เปล่าพร้อมใช้ (`src/main.c` ว่าง · มี `startup.c` `usb_serial` ให้แล้ว) → UnityMbed IDE → **Open Project** → เลือกโฟลเดอร์นี้ → วาง Prompt จาก `prompt.txt` ใน AI Chat → Build → Flash

**อุปกรณ์:** ใช้ของจาก Lab 4

## ต่อวงจร

| อุปกรณ์ | ขา Blue Pill |
|---|---|
| DHT11 DATA | PB12 |
| OLED SCL / SDA | PB6 / PB7 |

## ผลที่ควรเห็น
จอแสดง TinyML Regression / Act / Pred / Loss — Loss ลดลงเรื่อย ๆ / กำเซนเซอร์ไว้ในมือให้อุ่นขึ้น แล้วดูค่าทำนายวิ่งตามค่าจริงบนจอ

## Pred กับ Loss คืออะไร
- **Act** = อุณหภูมิจริงที่ DHT11 อ่านได้รอบนี้
- **Pred** = อุณหภูมิที่โมเดลทายว่ารอบนี้จะเป็นเท่าไร โดยใช้ค่าของ 2 วินาทีก่อน (x): Pred = w × x + b
- **Loss** = (Pred − Act)² = ทายผิดมากแค่ไหน
- ทุกรอบชิปใช้ Loss ปรับ w กับ b (Gradient Descent) · Loss ลดลงเข้าใกล้ 0 = ทายแม่นขึ้น

## วิธีทำ: เริ่มจากโปรเจกต์เปล่าในโฟลเดอร์นี้ แล้วสั่งทีละขั้น (2 ครั้ง)
1. Open Project โฟลเดอร์ Lab นี้ใน IDE (โปรเจกต์เปล่าพร้อมใช้ มี `usb_serial` ให้แล้ว)
2. Prompt 6.1: จอขึ้น TinyML Regression + Act: → Build + Flash
3. Prompt 6.2 (แชทเดิม): Linear Regression + Pred + Loss + ส่ง Serial → Build + Flash
4. เปิด Serial Plotter (เลือก COM ของบอร์ด) ดูกราฟ Act / Pred / Loss

Fix with AI ได้ไม่เกิน 2 รอบ ถ้ายังไม่ผ่านให้ย้อนไปไฟล์ของขั้นก่อนหน้า

## Prompt
ดู `prompt.txt` (เหมือนในสไลด์)

## ถ้าไม่ทำงาน
Copy ข้อความ Error หรือ Log จาก Monitor ไปวางในแชท AI แล้วบอกอาการ เช่น:
> Build แล้วเจอ Error นี้ [วาง Error] ช่วยหาสาเหตุและแก้เฉพาะจุดที่ผิด อธิบายสั้น ๆ ห้ามแก้ส่วนอื่นที่ทำงานได้แล้ว

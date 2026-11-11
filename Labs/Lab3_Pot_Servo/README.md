# Lab3 Pot Servo — หมุนวอลุ่มคุม Servo

> **เริ่มยังไง:** copy โฟลเดอร์ `STM32_Lab_Starter` ไปไว้ใน `C:\Users\<ชื่อ>\UnityMbed\` เปลี่ยนชื่อเป็นชื่อ Lab นี้ → UnityMbed IDE → Open Project → วาง Prompt จาก `prompt.txt` ใน AI Chat → Build → Flash

**อุปกรณ์:** Potentiometer 10k, Servo SG90

## ต่อวงจร

| อุปกรณ์ | ขา Blue Pill |
|---|---|
| Potentiometer ขากลาง | PA1 |
| Potentiometer ขาข้าง | 3.3V และ GND |
| Servo สายส้ม (สัญญาณ) | PA6 |
| Servo สายแดง | 5V (ขา 5V ของบอร์ด มีไฟเมื่อเสียบ Micro-USB) |
| Servo สายน้ำตาล | GND |

## ผลที่ควรเห็น
หมุนวอลุ่มแล้ว Servo หมุนตาม / Serial Monitor จะเห็นตัวเลข 3 ค่า: ค่าดิบ, ค่าที่กรอง, มุม (เอาไปพล็อตกราฟได้)

## Prompt
ดู `prompt.txt` (เหมือนในสไลด์)

## ถ้าไม่ทำงาน
Copy ข้อความ Error หรือ Log จาก Monitor ไปวางในแชท AI แล้วบอกอาการ เช่น:
> Build แล้วเจอ Error นี้ [วาง Error] ช่วยหาสาเหตุและแก้เฉพาะจุดที่ผิด อธิบายสั้น ๆ ห้ามแก้ส่วนอื่นที่ทำงานได้แล้ว

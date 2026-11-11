# Lab2 Button Serial — ปุ่มกด + Serial Monitor

> **เริ่มยังไง:** copy โฟลเดอร์ `STM32_Lab_Starter` ไปไว้ใน `C:\Users\<ชื่อ>\UnityMbed\` เปลี่ยนชื่อเป็นชื่อ Lab นี้ → UnityMbed IDE → Open Project → วาง Prompt จาก `prompt.txt` ใน AI Chat → Build → Flash

**อุปกรณ์:** ปุ่มกด, สาย Micro-USB (ใช้ดู Serial)

## ต่อวงจร

| อุปกรณ์ | ขา Blue Pill |
|---|---|
| ปุ่มกด ขาหนึ่ง | PA0 |
| ปุ่มกด อีกขา | GND |
| Micro-USB ของบอร์ด | เสียบเข้าคอม (คู่กับ ST-LINK) |

## ผลที่ควรเห็น
กดปุ่มแล้ว LED บนบอร์ด (PC13) กะพริบตามจำนวนครั้งที่กด: กดครั้งที่ 1 กะพริบ 1 ครั้ง, ครั้งที่ 2 กะพริบ 2 ครั้ง ... ครบ 5 แล้ววนกลับเป็น 1

Serial Monitor (เลือก COM ของบอร์ด) จะเห็น Count: 1, Count: 2 ... ด้วย

## Prompt
ดู `prompt.txt` (เหมือนในสไลด์)

## ถ้าไม่ทำงาน
Copy ข้อความ Error หรือ Log จาก Monitor ไปวางในแชท AI แล้วบอกอาการ เช่น:
> Build แล้วเจอ Error นี้ [วาง Error] ช่วยหาสาเหตุและแก้เฉพาะจุดที่ผิด อธิบายสั้น ๆ ห้ามแก้ส่วนอื่นที่ทำงานได้แล้ว

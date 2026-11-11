# Lab4 DHT11 OLED — DHT11 + จอ OLED

> **เริ่มยังไง:** copy โฟลเดอร์ `STM32_Lab_Starter` ไปไว้ใน `C:\Users\<ชื่อ>\UnityMbed\` เปลี่ยนชื่อเป็นชื่อ Lab นี้ → UnityMbed IDE → Open Project → วาง Prompt จาก `prompt.txt` ใน AI Chat → Build → Flash

**อุปกรณ์:** DHT11, OLED SSD1306 0.96" I2C

## ต่อวงจร

| อุปกรณ์ | ขา Blue Pill |
|---|---|
| DHT11 VCC | 3.3V |
| DHT11 DATA | PB12 |
| DHT11 GND | GND |
| OLED VCC / GND | 3.3V / GND |
| OLED SCL | PB6 |
| OLED SDA | PB7 |

## ผลที่ควรเห็น
จอแสดง Temp และ Humi เป็นตัวเลขใหญ่ / ลองเป่าลมหายใจใส่ DHT11 ค่าความชื้นจะพุ่งขึ้น / ถ้าจอไม่ติด LED จะกระพริบเร็ว = เช็กสาย SCL/SDA

## Prompt
ดู `prompt.txt` (เหมือนในสไลด์)

## ถ้าไม่ทำงาน
Copy ข้อความ Error หรือ Log จาก Monitor ไปวางในแชท AI แล้วบอกอาการ เช่น:
> Build แล้วเจอ Error นี้ [วาง Error] ช่วยหาสาเหตุและแก้เฉพาะจุดที่ผิด อธิบายสั้น ๆ ห้ามแก้ส่วนอื่นที่ทำงานได้แล้ว

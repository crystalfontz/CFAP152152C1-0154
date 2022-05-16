# CFAP152152C1-0154

This is the demonstration code for the Crystalfontz [CFAP152152C1-0154](https://www.crystalfontz.com/product/cfap152152c10154) black/white epaper display running on a 3.3V Seeeduino. The display controller used for this epaper is the [UltraChip US8151D](https://www.crystalfontz.com/controllers/UltraChip/UC8151d/)

  
Kits for this product can be found here:  
[CFAP152152C1-0154-E2](https://www.crystalfontz.com/product/CFAP152152C1-0154-E2)\
[CFAP152152C1-0154-E2-2](https://www.crystalfontz.com/product/CFAP152152C1-0154-E2-2)


## Connection Details
#### To [CFA10084 Adapter Board](https://www.crystalfontz.com/product/cfa10084) (See kits above)
|Seeeduino  |10084  |Function            |
|-----------|-------|--------------------|
|D3         |8      |Busy Line           |
|D4         |7      |Reset Line          |
|D5         |6      |Data/Command Line   |
|D10        |5      |Chip Select Line    |
|D11        |4      |MOSI                |
|D13        |3      |Clock               |
|3.3V       |1      |Power               |
|GND        |2      |Ground              |


#### To Optional [uSD Adapter Board](https://www.crystalfontz.com/product/cfa10112-sd-card-reader)
| microSD Pin | Seeeduino Pin| Connection Description |
|-------------|--------------|------------------------|
| 1 (CS)      | D10          | SD CS                  |
| 2 (DI)      | D11          | SD MOSI                |
| 3 (DO)      | D12          | SD MISO                |
| 4 (VDD)     | 3v3          | +3.3V Power            |
| 5 (SCLK)    | D13          | SD SCLK                |
| 6 (VSS)     | GND          | Ground                 |

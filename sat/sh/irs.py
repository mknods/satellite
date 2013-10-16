#! /usr/bin/python

import smbus
i2c = smbus.SMBus(1)

#i2c.write_byte(0x0a,0x4c)
buf = i2c.read_i2c_block_data(0x0a,0x4c)
#print buf

row1 = [0,0,0,0]
row2 = [0,0,0,0]
row3 = [0,0,0,0]
row4 = [0,0,0,0]
row1[3] = buf[3]*256 + buf[2]
row1[2] = buf[5]*256 + buf[4]
row1[1] = buf[7]*256 + buf[6]
row1[0] = buf[9]*256 + buf[8]
row2[3] = buf[11]*256 + buf[10]
row2[2] = buf[13]*256 + buf[12]
row2[1] = buf[15]*256 + buf[14]
row2[0] = buf[17]*256 + buf[16]
row3[3] = buf[19]*256 + buf[18]
row3[2] = buf[21]*256 + buf[20]
row3[1] = buf[23]*256 + buf[22]
row3[0] = buf[25]*256 + buf[24]
row4[3] = buf[27]*256 + buf[26]
row4[2] = buf[29]*256 + buf[28]
row4[1] = buf[31]*256 + buf[30]
row4[0] = buf[1]*256 + buf[0]

print row4
print row3
print row2
print row1

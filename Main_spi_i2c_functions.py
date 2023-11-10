'''
Reading and calibration of homemade orifice plate flow sensor using Sensirion SFM3300.

Authors: Allison Marten and Andrés Sanchez
Institution: Harvey Mudd College, CowBell Labs Clinic Team
Created: 2023-11-05
Last modified: 2023-11-09
Last modified by: Allison Marten


Sources:
 1. Open Vent Bristol "How to Make an Accurate Air Flow Rate Sensor With Arduino for Under £20 COVID-19 Ventilator"
    https://www.instructables.com/How-to-Make-an-Air-Flow-Rate-Sensor-With-Arduino-a/
 
 2. Simple example of reading the MCP3008 analog input channels and printing them all out.
    Author: Tony DiCola
    License: Public Domain

 3. I2C code from StackOverflow user Patryk Dzierżawski https://stackoverflow.com/users/5763087/patryk-dzier%c5%bcawski
    https://stackoverflow.com/questions/60741196/difference-i2c-sensor-reading-raspberry-pi-and-arduino#comment107542180_60774615
'''


import time
from datetime import datetime as dtime
import csv

# Import SPI library (for hardware SPI) and MCP3008 library.
import Adafruit_GPIO.SPI as SPI
import Adafruit_MCP3008

# Import libraries for I2C communication
from fcntl import ioctl
from struct import unpack
from smbus import SMBus

# I2C device address (SFM3300) = 0x40 (64)
i2c_address = 0x40

# Pick I2C bus 1, default for Raspberry Pi 3B
bus = SMBus(1)

# Software SPI configuration:
CLK  = 18
MISO = 23
MOSI = 24
CS   = 25
mcp = Adafruit_MCP3008.MCP3008(clk=CLK, cs=CS, miso=MISO, mosi=MOSI)

# Hardware SPI configuration:
SPI_PORT   = 0
SPI_DEVICE = 0
mcp = Adafruit_MCP3008.MCP3008(spi=SPI.SpiDev(SPI_PORT, SPI_DEVICE))
  

# print('Reading MCP3008 values, press Ctrl-C to quit...')
# Print nice channel column headers.
# print('| {0:>4} | {1:>4} | {2:>4} | {3:>4} | {4:>4} | {5:>4} | {6:>4} | {7:>4} | {8:>6} |'.format(*range(9)))
# print('-' * 66)
# Main program loop.
# Headers for .csv
# print("time_ms , temp_0 , temp_1 , temp_0-1 , diff_p , abs_p , sfm3300")
headers = ["time_ms" , "temp_0" , "temp_1" , "temp_0-1" , "diff_p" , "abs_p" , "sfm3300"]
print(headers)
start_time = time.time()

def sfm3300_I2C():
    ''' Communicate with and read outputs from the SFM3300 Volumetric Flow Sensor over I2C protocol.
	Return flow rate string.
    '''
    # Contact I2C device (SFM3300) and read values
    # Send start continuous measurement command (0x1000) to SFM3300
    bus.write_byte_data(i2c_address,0x10,0x00)
    # Open the I2C device file for reading
    i2c = open("/dev/i2c-1", "rb", buffering=0)
    # ioctl = input/output control. 0x0703 sets the SFM3300 device address to its default address 0x40 (based on the datasheet)
    ioctl(i2c,0x0703,i2c_address)
    # Read 3 bytes of data from SFM3300
    flow_data = i2c.read(3)
    # Unpack flow data into three variables
    # d0 and d1 are bits 15:8 and 7:0 of the flow rate, respectively
    # c is the checksum
    d0,d1,crc = unpack('BBB', flow_data)
    # print(d0, d1)
    # Shift d0 8 bits to the left and combine with d1 to get the whole flow reading
    d = d0 << 8 | d1
    # Offset and scale the flow reading using values from the datasheet
    sfm3300_flow_rate = (float(d)-32768.)/120
    # Round output to 4 decimal places because floating value -0.0333 is very long and interferes with formatting
    sfm3300_flow_rate = round(sfm3300_flow_rate, 4)
    # checkvalue(d0, d1, crc)
    return sfm3300_flow_rate

    
def mcp3008_SPI():
    ''' Read output from the MCP3008 Analog to Digital Converter (ADC).
	Return outputs in a list.
    '''
    # Read all the ADC channel values in a list. Add extra element (*9 instead of *8) so sfm3300 flow rate can be added later.
    values = [0]*9
    for i in range(8):
        # The read_adc function will get the value of the specified channel (0-7).
        values[i] = mcp.read_adc(i)
    return values

def checkvalue(d0, d1, crc):
    ''' Used to check the flow meter checksum. Possibly broken or incorrect.
    '''
    mycrc = 0xFF
    mycrc = crc8(d0, mycrc)
    mycrc = crc8(d1, mycrc)
    if (mycrc != crc):
        print("Error: CRC Issue")


def crc8(data, crc):
    ''' Used to check the flow meter checksum with crc8 algorithm. Possibly broken or incorrect.
    '''
    crc ^= data
    for i in range(8, 0, -1):
        if crc & 0x80:
            crc = (crc << 1) ^ 0x31
        else:
            crc = (crc << 1)
    return crc

def main(n, file_name, samp_rate):
    ''' Takes in number of samples to take n, the name of the file to store the results in, and the sample rate samp_rate.
	Creates file_name.csv
	Loops n times, storing data in file_name.csv
	Prints outputs
    '''
    # Initiate sample numbering
    j = 0
    
    # Create writable file_name.csv and add header
    with open('Results/'+file_name, 'w', newline='') as file:
	    writer = csv.writer(file)
	    field = headers
	    writer.writerow(field)
	    
	    # Take n measurements
	    while(j < n):
		    # Record elapsed time as the difference between starting and current epoch time, *1000 for milliseconds
		    time_elapsed_ms = round((time.time()-start_time)*1000,2)

		    # Get current flow rate from SFM3300 through I2C
		    sfm3300_flow_rate = sfm3300_I2C()

		    # Get pressure and temperature sensor values from MCP3008 ADC
		    values = mcp3008_SPI()

		    # Add flow rate measurement to values for printing
		    values[8] = sfm3300_flow_rate

		    # Print the ADC values.
		    # print('| {0:>4} | {1:>4} | {2:>4} | {3:>4} | {4:>4} | {5:>4} | {6:>4} | {7:>4} |{8:>7} |'.format(*values))
		    # Print for .csv
		    
		    # Create row to add to file_name.csv
		    output_csv = [time_elapsed_ms, values[0], values[1], values[0]-values[1], values[2], values[3], sfm3300_flow_rate]
		    # print(time_elapsed_ms, ",", values[0], ",", values[1], ",",  values[0]-values[1], ",", values[2], ",",  values[3], ",",  sfm3300_flow_rate)
		    print(j, output_csv)
		    writer.writerow(output_csv)
		    j = j+1
		    time.sleep(1/samp_rate)
    
    
def make_timestamp_csv(name_note, samp_rate, n):
    ''' Creates a string with the format for use as the .csv file name
	'file_timestamp'__'samp_rate'_"hz"__'n'_"n"__'name_note'.csv
	Single quotes mark variables and douuble quotes mark strings.
	If name_note is empty, the file name is
	'file_timestamp'__'samp_rate'_"hz"__'n'_"n".csv
	Returns the file name as a string.
    '''
    # Gets current date and time
    now = dtime.now()
    # Formats current date and time as YYYY_mm_dd__HH_MM_SS
    file_timestamp = now.strftime("%Y_%m_%d__%H_%M_%S")
    # Deals with empty name_note case, so there isn't a trailing __ at the end of the file name
    if name_note == "": name_note2 = ""
    else: name_note2 = "__" + name_note
    # Concatenates variables and converts ints to str
    file_name = file_timestamp + "__" + str(samp_rate) + "_" + "hz" + "__" + str(n) + "_" + "n" + name_note2 + '.csv'
    # print(file_name)
    return file_name



if __name__ == "__main__":
    ''' Runs first. Input parameters for run and file name
    '''
    # Set number of measurements to take
    n = 1000
    # Set sample rate
    samp_rate = 100
    # Add notes to file name
    name_note = 'cal'
    # Create file and output file name for main()
    file_name = make_timestamp_csv(name_note, samp_rate, n)

    # Take measurements and add to file
    main(n, file_name, samp_rate)
    

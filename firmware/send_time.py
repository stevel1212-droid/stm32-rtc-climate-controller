#!/usr/bin/env python3
"""
Send current PC time to STM32 microcontroller via Serial.
Format: HH:MM:SS DD:MM:YYYY
"""

import serial
import sys
from datetime import datetime

# Configuration
PORT = 'COM3'  # Change this to your COM port
BAUD_RATE = 115200

def send_time(port=PORT, baud=BAUD_RATE):
    """Send current PC time to device"""
    try:
        ser = serial.Serial(port, baud, timeout=2)
        
        # Get current time from PC
        now = datetime.now()
        time_str = now.strftime("%H:%M:%S %d:%m:%Y")
        
        print(f"Sending time to {port}: {time_str}")
        
        # Send with newline
        ser.write((time_str + "\n").encode())
        
        # Wait for response
        response = ser.readline().decode().strip()
        if response:
            print(f"Device response: {response}")
        
        ser.close()
        print("Done!")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        send_time(sys.argv[1])
    else:
        send_time()

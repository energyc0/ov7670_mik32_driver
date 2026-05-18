#!/usr/bin/env python3
"""
A script for receiving data from the OV7670 camera via UART and saving it in PPM format.

Usage:
    python3 camera_ppm_receiver.py /dev/ttyUSB0 115200
"""

import serial
import sys
import struct
import time
from datetime import datetime

CAMERA_WIDTH = 40
CAMERA_HEIGHT = 30
EXPECTED_PIXELS = CAMERA_WIDTH * CAMERA_HEIGHT
FRAME_SIZE = EXPECTED_PIXELS * 2
TOTAL_EXPECTED = FRAME_SIZE + 5


def bytes_to_rgb888_pixel(data):
    pixel = (data[1] << 8) | data[0]  # little-endian
    # extract data
    r = (pixel >> 11) & 0x1F
    g = (pixel >> 5) & 0x3F
    b = pixel & 0x1F
    # convert from rgb565 to rgb888
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    
    return (r, g, b)


def save_ppm(filename, width, height, pixels):
    with open(filename, 'wb') as f:
        # Header
        f.write(f"P6\n{width} {height}\n255\n".encode())
        
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[y * width + x]
                f.write(bytes([r, g, b]))
    
    print(f"Saved: {filename}")


def receive_frame(ser, timeout=3):
    start_time = time.time()
    
    # "RDY\r\n" is a start of the frame
    buffer = b''
    while b'RDY\r\n' not in buffer:
        if time.time() - start_time > timeout:
            print("Timeout waiting for frame marker")
            print("Have read: ", buffer)
            return None
        
        if ser.in_waiting:
            while ser.in_waiting:
                buffer += ser.read(1)
        else:
            time.sleep(0.001)
    
    marker_pos = buffer.find(b'RDY\r\n')
    if marker_pos != -1:
        leftover = buffer[marker_pos + 5:]
    else:
        leftover = b''
    
    # Read the remaining data
    while len(leftover) < FRAME_SIZE:
        if time.time() - start_time > timeout:
            print(f"Timeout: received {len(leftover)}/{FRAME_SIZE} bytes")
            return None
        
        if ser.in_waiting:
            leftover += ser.read(ser.in_waiting)
        else:
            time.sleep(0.001)
    
    frame_data = leftover[:FRAME_SIZE]
    
    if len(frame_data) != FRAME_SIZE:
        print(f"Invalid frame size: {len(frame_data)}")
        return None
    
    return frame_data


def decode_rgb565_frame(frame_data, width, height):
    pixels = []
    for i in range(0, len(frame_data), 2):
        if i + 1 < len(frame_data):
            r, g, b = bytes_to_rgb888_pixel(frame_data[i:i+2])
            pixels.append((r, g, b))
    
    if len(pixels) != width * height:
        print(f"Warning: expected {width*height} pixels, got {len(pixels)}")
    
    return pixels


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 camera_ppm_receiver.py <serial_port> [baudrate]")
        print("Example: python3 camera_ppm_receiver.py /dev/ttyUSB0 115200")
        sys.exit(1)
    
    port = sys.argv[1]
    baudrate = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
    
    print(f"Opening {port} at {baudrate} baud...")
    
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        sys.exit(1)
    
    print("Waiting for data from MIK32...")
    print(f"Expected frame size: {FRAME_SIZE} bytes data + 5 bytes header")
    print("Press Ctrl+C to stop\n")
    
    frame_count = 0
    
    try:
        while True:
            # Receive frame
            frame_data = receive_frame(ser)
            
            if frame_data is None:
                print("Failed to receive frame, retrying...")
                continue
            
            # Decode pixels
            pixels = decode_rgb565_frame(frame_data, CAMERA_WIDTH, CAMERA_HEIGHT)
            
            if len(pixels) != EXPECTED_PIXELS:
                print(f"Skipping frame with {len(pixels)} pixels")
                continue
            
            # Save to PPM
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"frame_{frame_count:04d}_{timestamp}.ppm"
            save_ppm(filename, CAMERA_WIDTH, CAMERA_HEIGHT, pixels)
            
            frame_count += 1
            
    except KeyboardInterrupt:
        print(f"\nStopped. Saved {frame_count} frames.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
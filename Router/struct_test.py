import struct

values = [100, 200, 300, 400, 500, 600, 700]

for i in range(0, len(values), 4):
    chunk = values[i:i+4]
    
    print(chunk)
    # Pack the chunk into 2 bytes per value in big-endian format
    packed_data = struct.pack(f">{len(chunk)}H", *chunk)

    print(type(packed_data))
    
     # Print each byte in hex format for clarity
    print(' '.join(f'{byte:02X}' for byte in packed_data))

    
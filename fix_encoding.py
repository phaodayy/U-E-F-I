import sys

filepath = r'UnifiedTelemetryNode\telemetry\sdk\Utils\Translation.h'

with open(filepath, 'rb') as f:
    raw_data = f.read()

# Try decoding using utf-8 first, if it fails, try cp1258 (Vietnamese ANSI) or utf-8 without bom
try:
    if raw_data.startswith(b'\xef\xbb\xbf'):
        text = raw_data.decode('utf-8-sig')
        print("File is already UTF-8 with BOM.")
    else:
        try:
            text = raw_data.decode('utf-8')
            print("Decoded as UTF-8.")
        except UnicodeDecodeError:
            text = raw_data.decode('cp1258') # Windows Vietnamese code page
            print("Decoded as Windows-1258.")

    with open(filepath, 'w', encoding='utf-8-sig') as f:
        f.write(text)
    print("Saved as UTF-8 with BOM.")
except Exception as e:
    print(f"Error: {e}")

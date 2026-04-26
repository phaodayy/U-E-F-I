import sys

filepath = r'UnifiedTelemetryNode\telemetry\overlay\translation.hpp'
with open(filepath, 'rb') as f:
    content = f.read()

# Replace any CR or CRLF or LF with CRLF
content_str = content.decode('utf-8-sig', errors='ignore')
content_str = content_str.replace('\r\n', '\n').replace('\r', '\n')
content_str = content_str.replace('\n', '\r\n')

with open(filepath, 'w', encoding='utf-8-sig', newline='') as f:
    f.write(content_str)
print("Fixed line endings for translation.hpp")

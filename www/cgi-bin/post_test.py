#!/usr/bin/env python3
import sys

data = sys.stdin.read()

print("Content-Type: text/plain")
print()
print("POST isteği alındı:\n")
print(data)

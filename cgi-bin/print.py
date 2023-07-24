#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()

request_method = os.environ['REQUEST_METHOD']

print(f"request_method: {request_method}")

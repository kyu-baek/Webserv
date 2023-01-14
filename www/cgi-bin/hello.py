#!/usr/bin/python3

import sys, cgi, os

form = cgi.FieldStorage()

script = str(os.environ.get("PATH_TRANSLATED"))

print(script)

# line = sys.stdin.readline()
# print(line)

content = sys.stdin.read()
print(content)

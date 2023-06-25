#!/usr/bin/env python3

import cgi
import tempfile
import os
import sys

# form = cgi.FieldStorage()

# fileitem = form["fileToUpload"]

logfile = open("log.txt", "w")

print(os.environ["CONTENT_TYPE"], file=logfile)

# if fileitem.filename:
# fn = os.path.basename(file item.filename)
fn = "test.txt"

# print(fn, file=logfile)
# print(fileitem.file.read(), file=logfile)
# print(type(fileitem.file.read()), file=logfile)
# print(fileitem.file.read().decode('utf-8'), file=logfile)
# print(type(fileitem.file.read().decode('utf-8')), file=logfile)
tmpfile = open("/tmp/" + fn, "wb")
# tmpfile.write(fileitem.file.read())
print(input(), file=logfile)
print("---", file=logfile)

message = 'The file "' + fn + '" was uploaded successfully'


logfile.close()

print(
    """\
Content-Type: text/html\n
<html>
<body>
<p>%s</p>
</body>
</html>
"""
    % (message,)
)

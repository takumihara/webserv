#!/usr/bin/env python3

import cgi
import os

logfile = open("log.txt", "w")

# print(os.environ["CONTENT_TYPE"], file=logfile)

form = cgi.FieldStorage()

fileitem = form["fileToUpload"]


if fileitem.filename:
    fn = os.path.basename(fileitem.filename)

    # print(fileitem.file.read(), file=logfile)
    # print(type(fileitem.file.read()), file=logfile)
    # print(fileitem.file.read().decode('utf-8'), file=logfile)
    # print(type(fileitem.file.read().decode('utf-8')), file=logfile)

    # todo(thara): fix path once working directory for cgi is fixed
    tmpfile = open("./tmp/" + fn, "wb")
    tmpfile.write(fileitem.file.read())
    tmpfile.close()

    message = 'The file "' + fn + '" was uploaded successfully'

else:
    message = "No file was uploaded"

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

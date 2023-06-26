#!/usr/bin/env python3

import cgi
import os

logfile = open("log.txt", "w")

# print(os.environ["CONTENT_TYPE"], file=logfile)

form = cgi.FieldStorage()

fileitem = form["fileToUpload"]

try:
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

        status = "201 Created"
        message = 'The file "' + fn + '" was uploaded successfully'

    else:
        status = "400 Bad Request"
        message = "No file was uploaded"
except Exception as e:
    status = "500 Internal Server Error"
    message = "An error occurred while uploading the file: " + str(e)

logfile.close()

print("Content-Type:text/html")
print(f"Status:{status}\n")

print(
    f"""<html>
<body>
<p>{message}</p>
</body>
</html>
"""
)

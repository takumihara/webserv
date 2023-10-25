#!/usr/bin/env python3

import cgi
import os

# print(os.environ["CONTENT_TYPE"], file=logfile)

WHERE_TO_UPLOAD = "./"

form = cgi.FieldStorage()

print("Content-Type:text/html")
request_method = os.environ['REQUEST_METHOD']
if request_method != "POST":
    print(f"Status:400 Bad Request\n")
    exit()

try:
    logfile = open("log.txt", "w")
    fileitem = form["fileToUpload"]
    if fileitem.filename:
        fn = os.path.basename(fileitem.filename)

        # print(fileitem.file.read(), file=logfile)
        # print(type(fileitem.file.read()), file=logfile)
        # print(fileitem.file.read().decode('utf-8'), file=logfile)
        # print(type(fileitem.file.read().decode('utf-8')), file=logfile)

        tmpfile = open(WHERE_TO_UPLOAD + fn, "wb")
        tmpfile.write(fileitem.file.read())
        tmpfile.close()

        status = "201 Created"
        message = 'The file "' + fn + '" was uploaded successfully'

    else:
        status = "400 Bad Request"
        message = "No file was uploaded"

    logfile.close()

except Exception as e:
    status = "500 Internal Server Error"
    message = "An error occurred while uploading the file: " + str(e)


print(f"Status:{status}\n")

print(
    f"""<html>
<body>
<p>{message}</p>
</body>
</html>
"""
)

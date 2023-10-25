#!/usr/bin/env python3

import cgi
import os

# print(os.environ["CONTENT_TYPE"], file=logfile)

WHERE_TO_UPLOAD = "./"

form = cgi.FieldStorage()

print("Content-Type:text/html")
request_method = os.environ['REQUEST_METHOD']
path_info = os.environ['PATH_INFO']
if request_method != "DELETE":
    print(f"Status:400 Bad Request\n")
    exit()

try:
    os.remove("." + path_info)
    status="200 OK"
    message = f"File {path_info} was deleted: "

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

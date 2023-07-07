#!/usr/bin/env python3

import cgi
import cgitb
import sqlite3
import os
from http import cookies

cgitb.enable()

print("Content-Type: text/html")  # HTML is following
print()  # blank line required, end of headers

form = cgi.FieldStorage()

database_name = "test.db"  # change this to your database name
table_name = "test"  # change this to your table name
column_name = "column1"  # change this to your column name


def handle_get():
    conn = sqlite3.connect(database_name)
    cursor = conn.cursor()

    cursor.execute(f"SELECT * FROM {table_name}")

    rows = cursor.fetchall()

    for row in rows:
        print(row)


def handle_post():
    value = form.getvalue(column_name)
    conn = sqlite3.connect(database_name)
    cursor = conn.cursor()

    cursor.execute(f"INSERT INTO {table_name} ({column_name}) VALUES (?)", (value,))

    conn.commit()

    print(f"{value} has been added to the database.")


def handle_delete():
    value = form.getvalue(column_name)
    conn = sqlite3.connect(database_name)
    cursor = conn.cursor()

    cursor.execute(f"DELETE FROM {table_name} WHERE {column_name}=?", (value,))

    conn.commit()

    print(f"{value} has been deleted from the database.")


request_method = os.environ["REQUEST_METHOD"]

if request_method == "GET":
    handle_get()
elif request_method == "POST":
    handle_post()
elif request_method == "DELETE":
    handle_delete()
else:
    print(f"HTTP {request_method} is not supported.")

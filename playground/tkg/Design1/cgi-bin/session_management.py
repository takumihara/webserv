#!/usr/bin/env python3

import cgi
import cgitb
import sqlite3
import os
from http import cookies
import secrets


def new_session():
    session_id = secrets.token_hex(16)
    visit_count = 1

    cursor = conn.cursor()
    cursor.execute(f"INSERT INTO {table_name} VALUES (?, ?)", (session_id, visit_count))
    conn.commit()
    return session_id


cgitb.enable()

print("Content-Type: text/html")

database_name = "test.db"
table_name = "session"
conn = sqlite3.connect(database_name)

incoming_cookie = cookies.SimpleCookie(os.getenv("HTTP_COOKIE"))

if incoming_cookie.get("session") is None:
    session_id = new_session()
    visit_count = 1
else:
    session_id = incoming_cookie.get("session").value
    cursor = conn.cursor()
    res = cursor.execute(
        f"SELECT * FROM {table_name} WHERE id=?", (session_id,)
    ).fetchone()

    if res is None:
        session_id = new_session()
        visit_count = 1
    else:
        _, visit_count = res
        visit_count += 1
        cursor.execute(
            f"UPDATE {table_name} SET visit_count=? WHERE id=?",
            (visit_count, session_id),
        )
        conn.commit()

print("Set-Cookie: session=" + session_id)
print()
print(f"Session ID: {session_id} | Visits: {visit_count}")

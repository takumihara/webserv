#!/bin/bash

# requirement: sqlite3

sqlite3 test.db < script/db.sql && chmod 777 test.db

FROM python:3.9-slim-buster

WORKDIR /app

RUN apt-get update && apt-get install -y \
  sqlite3 \
  && rm -rf /var/lib/apt/lists/*

# RUN sqlite3 /app/test.db < /app/db.sql && \
#   rm /app/db.sql && \
#   chmod 777 /app/test.db && \
#   chmod 777 /app

# RUN chmod +x /app/cgi-bin/*

EXPOSE 8000

ENV NAME World

CMD ["python3", "-m", "http.server", "--cgi", "8000"]

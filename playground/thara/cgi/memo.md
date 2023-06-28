### instructions

```
docker build -t my-python-app .
docker build --no-cache -t my-python-app .
docker run -p 4000:8000  -v /absolute-path/...:/app my-python-app
```

```
curl 'http://localhost:4000/cgi-bin/main.py'
curl -XPOST -d 'column1=value' 'http://localhost:4000/cgi-bin/main.py'
curl -XPOST -d 'column1=value' 'http://localhost/cgi-bin/print.py'
```

python cgi seems not to support DELETE

```
curl -XDELETE -d 'column1=value' 'http://localhost:4000/cgi-bin/main.py'
```

for our server

```
curl -XPOST -d 'column1=value' 'http://localhost/cgi-bin/db_app.py'
curl -XDELETE -d 'column1=value' 'http://localhost/cgi-bin/db_app.py'
```

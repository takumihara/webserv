### instructions

```
docker build -t my-python-app .
docker run -p 4000:8000  --name my-python-app my-python-app
```

```
curl 'http://localhost:4000/cgi-bin/main.py'
curl -XPOST -d 'column1=value' 'http://localhost:4000/cgi-bin/main.py'
```

python cgi seems not to support DELETE

```
curl -XDELETE -d 'column1=value' 'http://localhost:4000/cgi-bin/main.py'
```

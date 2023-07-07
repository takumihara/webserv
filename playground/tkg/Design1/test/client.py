#!/usr/bin/env python3

import http.client
import sys

def send_http_request(host, port, method, path, body, headers):
    # URLを解析してホスト名とパスを取得
    conn = http.client.HTTPConnection(host, port)
    conn.request(method, path, body, headers)
    
    # レスポンスを取得
    response = conn.getresponse()
    
    # レスポンスのステータスコードとヘッダーを表示
    print(f"HTTP/{response.version/10} {response.status} {response.reason}")
    for header, value in response.getheaders():
        print(header + ":", value)
    
    # レスポンスのボディを受け取る
    body = response.read()
    # レスポンスのボディを表示
    print(body.decode())

    # 接続を閉じる
    conn.close()

# メインの実行部分
if __name__ == "__main__":
    argv = sys.argv
    host = argv[1]
    port = argv[2]
    method = argv[3]
    path = argv[4]
    body = argv[5]
    headers = dict()
    for h in argv[6:]:
        name = h[:h.find(":")]
        value = h[h.find(":")+1:]
        headers[name] = value
    send_http_request(host, port, method, path, body, headers)
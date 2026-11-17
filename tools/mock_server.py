"""Server giả lập chạy trên máy tính để thử OBC trước khi dùng dịch vụ miễn phí thật.
Chạy: python tools/mock_server.py   (cổng 8000). Đặt URL trong secrets.h trỏ tới IP của máy tính (mạng công khai cần tunnel).
Nhận: POST /api/telemetry (JSON) và POST /api/image (nhị phân, có img, idx, total); lưu vào thư mục received/."""
import json, os, time
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, parse_qs
os.makedirs("received", exist_ok=True)

class H(BaseHTTPRequestHandler):
    def do_POST(self):
        u = urlparse(self.path); q = parse_qs(u.query)
        body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        if u.path.endswith("/telemetry"):
            with open("received/telemetry.jsonl", "ab") as f: f.write(body + b"\n")
            print(time.strftime("%H:%M:%S"), "telemetry", body[:120])
        elif u.path.endswith("/image"):
            img, idx = q.get("img", ["0"])[0], q.get("idx", ["0"])[0]
            with open(f"received/img_{img}.part{int(idx):04d}", "wb") as f: f.write(body)
            print("image", img, "chunk", idx, "of", q.get("total", ["?"])[0], len(body), "B")
        self.send_response(200); self.end_headers(); self.wfile.write(b"OK")
    def do_GET(self):
        self.send_response(200); self.end_headers(); self.wfile.write(b"NONE")

HTTPServer(("0.0.0.0", 8000), H).serve_forever()

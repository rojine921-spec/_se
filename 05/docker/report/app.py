"""獨立 Python 報表微服務：透過 HTTP 呼叫 Node.js API，不直接連資料庫。"""
import json
import os
import urllib.error
import urllib.request
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

API_BASE_URL = os.environ.get("API_BASE_URL", "http://api:3000").rstrip("/")
PORT = int(os.environ.get("PORT", "8000"))


class Handler(BaseHTTPRequestHandler):
    def send_json(self, code: int, data: dict) -> None:
        payload = json.dumps(data, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self) -> None:
        if self.path == "/health":
            self.send_json(200, {"status": "ok", "service": "python-report"})
            return
        if self.path != "/report":
            self.send_json(404, {"error": "API 路徑不存在"})
            return
        try:
            with urllib.request.urlopen(f"{API_BASE_URL}/summary", timeout=3) as response:
                summary = json.load(response)
            total, done, pending = (int(summary[k]) for k in ("total", "done", "pending"))
            rate = round(done / total * 100, 1) if total else 0.0
            self.send_json(200, {
                "service": "python-report",
                "total": total,
                "done": done,
                "pending": pending,
                "completion_rate": rate,
                "message": "任務都完成了！" if total and pending == 0 else "持續完成你的 Docker 練習任務！",
                "generated_at": datetime.now(timezone.utc).isoformat(),
            })
        except (urllib.error.URLError, json.JSONDecodeError, ValueError, KeyError, TypeError) as error:
            print(f"Report error: {error}", flush=True)
            self.send_json(503, {"error": "無法取得 Node API 的統計資料"})


if __name__ == "__main__":
    server = ThreadingHTTPServer(("0.0.0.0", PORT), Handler)
    print(f"Python report service listening on {PORT}", flush=True)
    server.serve_forever()

#!/usr/bin/env python3
"""Docker 啟動後執行端到端 CRUD/API 健康檢查，不需額外 Python 套件。"""
import argparse
import json
import sys
from urllib.request import Request, urlopen


def call(base_url, path, method="GET", data=None):
    headers = {}
    body = None
    if data is not None:
        body = json.dumps(data, ensure_ascii=False).encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = Request(base_url + path, data=body, headers=headers, method=method)
    with urlopen(req, timeout=10) as response:
        return response.status, json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", default="http://localhost:8080", help="Nginx 網址")
    args = parser.parse_args()
    base = args.url.rstrip("/")
    task_id = None
    try:
        _, health = call(base, "/api/health")
        assert health["status"] == "ok" and health["database"] == "connected"
        print("[OK] Node.js + PostgreSQL 健康檢查")

        _, report = call(base, "/api/report")
        assert report["service"] == "python-report"
        print("[OK] Python 報表微服務")

        code, result = call(base, "/api/tasks", "POST", {"title": "自動化 Docker 驗收測試"})
        assert code == 201
        task_id = result["task"]["id"]
        print(f"[OK] POST 新增任務 ID={task_id}")

        _, listing = call(base, "/api/tasks")
        assert any(item["id"] == task_id for item in listing["tasks"])
        print("[OK] GET 讀取任務")

        _, updated = call(base, f"/api/tasks/{task_id}", "PATCH", {"completed": True})
        assert updated["task"]["completed"] is True
        print("[OK] PATCH 更新任務")

        _, report = call(base, "/api/report")
        assert report["done"] >= 1
        print("[OK] 報表可取得更新後的統計")

        print("[PASS] 所有測試通過。")
    except Exception as error:
        print(f"[FAIL] {type(error).__name__}: {error}", file=sys.stderr)
        return 1
    finally:
        if task_id is not None:
            try:
                call(base, f"/api/tasks/{task_id}", "DELETE")
                print("[OK] DELETE 測試資料已清除")
            except Exception as error:
                print(f"[WARN] 無法刪除測試資料 ID={task_id}: {error}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

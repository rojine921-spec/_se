# 模擬大學校務系統（習題 2）

使用 **Python + Flask + SQLite + HTML/CSS** 自行製作的課堂練習，沒有爬蟲，也不會連線到任何學校的校務平台。所有帳號、人名、課程均為虛構資料。

## 功能

- 學生／管理員登入、登出（密碼雜湊儲存）
- 校務儀表板（學分／選課數／管理數據）
- 開課列表（老師、學分、時間、名額）
- 學生選課、額滿限制與退選
- 學生查詢成績與已評分科目的學分加權平均
- 管理員查詢選課與輸入 0–100 分成績
- SQLite 自動建立資料庫與測試資料

## Windows 11 安裝與執行

1. 安裝 Python 3.10 或更新版本。
2. 使用 VS Code 開啟 `campus_portal` 資料夾，開啟終端機（PowerShell）。
3. 在終端機執行：

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
python app.py
```

> 若 PowerShell 禁止啟用虛擬環境，改用 `cmd` 執行 `.venv\Scripts\activate.bat`，或直接使用 `.venv\Scripts\python.exe -m pip install -r requirements.txt` 與 `.venv\Scripts\python.exe app.py`。

4. 瀏覽器打開 **http://127.0.0.1:5000**。

### 測試登入帳號

| 角色 | 帳號 | 密碼 |
| --- | --- | --- |
| 學生 | `s001` | `student123` |
| 學生 | `s002` | `student123` |
| 管理員 | `admin` | `admin123` |

### 建議展示操作

1. 學生 s001 登入、查詢課程、選修 CS101 與 CS202。
2. 到「我的選課」確認、到「成績查詢」看尚未公布。
3. 登出、用管理員登入，到「成績管理」將兩科輸入 85、90。
4. 登出、學生再次登入，確認成績及加權平均。
5. 嘗試退選已評分課程，系統會阻擋。

## 架構

```text
campus_portal/
├── app.py                  # 網站路由、選課邏輯、SQLite 建表
├── requirements.txt        # Python 套件
├── templates/              # HTML 頁面 (Jinja2)
│   ├── base.html
│   ├── login.html
│   ├── dashboard.html
│   ├── courses.html
│   ├── my_courses.html
│   ├── grades.html
│   ├── admin.html
│   └── error.html
└── static/
    └── style.css           # 網頁美化
```

首次執行會建立 `campus.db`。要將測試資料重設為初始狀態，關閉程式後刪除 `campus.db` 再執行。

## 資料表設計

- `users`：使用者帳號、密碼雜湊、系所、角色。
- `courses`：課程代碼、名稱、教師、學分、容量、上課時段。
- `enrollments`：學生與課程的多對多關聯，以及成績。

## 注意事項與擴充

此專案是**本機示範版**，方便完成課堂作業；有登入及基本權限控制，但**不適合直接公開部署或儲存真實個資**。公開前須設定安全的 `SECRET_KEY`、關閉 debug、增加 CSRF 保護、登入速率限制、測試、稽核紀錄等。

可再擴充課表衝堂檢查、加退選期間限制、GPA 換算、老師專用權限、學費繳納模擬、公告與個人資料頁面。

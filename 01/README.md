# mycurl

一個以 C 語言撰寫、`curl` 風格命令列的 HTTP 用戶端工具。

## 專案簡介

此程式模仿 curl 的命令列介面與輸出樣式，透過原始 Winsock socket 實作 HTTP/1.1 請求，
不需依賴任何第三方函式庫即可完成 GET / POST / PUT / DELETE 等常見 HTTP 操作。

## 功能特性

- 支援 HTTP 方法：GET、POST、PUT、DELETE、HEAD、PATCH、OPTIONS
- 自訂 HTTP 標頭（`-H`）、User-Agent（`-A`）、Basic 認證（`-u`）
- 請求資料（`-d`）自動設定 `Content-Length` 與 `Content-Type`
- 重定向跟隨（`-L`），支援絕對與相對路徑 Location
- 回應標頭顯示（`-i` / `-I`）、verbose 除錯輸出（`-v`）
- 輸出至檔案（`-o`）、靜默模式（`-s`）
- chunked 傳輸編碼解碼
- 跨平台程式碼（Windows 使用 Winsock，POSIX 使用 BSD socket）

## 建置方式

### 方式一：build.bat（Windows）

```bat
build.bat
```

腳本會自動搜尋 MinGW-w64 gcc（含 MSYS2 預設路徑），靜態連結輸出 `bin\mycurl.exe`。

### 方式二：make

```bash
make          # 建置
make clean    # 清除建置產物
make debug    # 除錯模式（-g -O0）
```

### 編譯器需求

- GCC 12+（或任何支援 C99 的編譯器）
- Windows 需 MinGW-w64 或 MSVC（MSVC 不需要 `-lws2_32`，由 `#pragma comment` 自動連結）

## 使用方式

```
mycurl [options...] <URL>
```

### 基本範例

```bat
bin\mycurl.exe http://example.com
bin\mycurl.exe -v http://httpbin.org/get
bin\mycurl.exe -X POST -d "username=admin&password=123" http://httpbin.org/post
bin\mycurl.exe -H "Authorization: Bearer token" http://api.example.com
bin\mycurl.exe -o output.json http://httpbin.org/get
bin\mycurl.exe -L http://example.com/redirect
bin\mycurl.exe -I http://example.com        <-- 僅顯示回應標頭（HEAD）
bin\mycurl.exe -s --version
```

### 選項一覽

| 選項 | 說明 |
| --- | --- |
| `-X, --request <method>` | 指定 HTTP 方法 |
| `-d, --data <data>` | 請求資料（自動轉為 POST） |
| `-H, --header <header>` | 新增自訂標頭，可多次使用 |
| `-o, --output <file>` | 將回應寫入檔案 |
| `-v, --verbose` | verbose 模式（顯示連線與請求細節） |
| `-i, --include` | 輸出回應標頭與內文 |
| `-I, --head` | HEAD 請求（僅標頭） |
| `-L, --location` | 跟隨重定向 |
| `-A, --user-agent <ua>` | 設定 User-Agent |
| `-u, --user <user:pass>` | Basic 認證 |
| `-s, --silent` | 靜默模式 |
| `--connect-timeout <sec>` | 連線逾時 |
| `--max-time <sec>` | 總請求時間上限 |
| `-k, --insecure` | 允許不安全的 SSL（保留，尚未實作 TLS） |
| `--help` / `--version` | 說明／版本 |

## 專案結構

```
benny921/_se/01/
├── include/
│   └── mycurl/
│       └── mycurl.h      # 共用型別與函式宣告
├── src/
│   ├── main.c            # 主程式：參數解析與流程控制
│   ├── http_client.c     # socket 連線、請求/回應、chunked 解碼、重定向
│   ├── url_parser.c      # URL 解析與重組
│   └── utils.c           # header 容器、base64、工具函式
├── examples/
│   ├── run_examples.bat  # 使用範例集合
│   └── response.json     # 範例輸出檔
├── build.bat             # Windows 一鍵建置腳本
└── Makefile              # make 建置腳本
```

## 內部設計重點

- **URL 解析**：`url_parse()` 將 scheme / host / port / path / query / fragment / 認證資訊拆分至 `parsed_url_t`。
- **Header 容器**：`header_list_t` 使用鍵值陣列，`header_list_add()` 會覆蓋同名標頭。
- **請求流程**：`http_execute()` 依序完成 DNS 解析 → TCP 連線 → 送出請求行與標頭 →
  接收回應 → 解析狀態行與標頭 → 處理 chunked 內文 →（視需要）跟隨 Location 重定向。
- **記憶體管理**：`url_free` / `header_list_free` / `request_free` / `response_free`
  專責釋放各自結構持有的內容，呼叫端負責釋放結構本體，避免嵌入式結構的雙重釋放。

## 目前限制

- HTTPS / TLS 尚未實作（`https://` 會以明文連線至 443，需自行擴充 OpenSSL 或 SChannel）
- 不支援 cookies、multipart/form-data、代理伺服器
- 回應內文上限為動態成長緩衝區，未限制大小

## 版權

此專案為教學用途之練習作品，與 curl 官方專案無關。
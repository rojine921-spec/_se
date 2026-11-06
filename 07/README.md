# 理財日常｜智慧記帳與消費分析網站

第 9 週 Cloudflare 網站應用作業。純 HTML、CSS、JavaScript，無需安裝套件或後端。

## 特色

- 精緻紫色系、桌面側邊選單、手機自適應排版
- 新增收入與支出、分類、日期
- 按月份檢視收入、支出與結餘
- 支出甜甜圈圖表與各類別占比
- 搜尋項目、篩選收支、刪除紀錄
- 匯出指定月份 CSV（可用 Excel 開啟）
- localStorage 儲存資料：同一瀏覽器保留，不會雲端同步；清除網站資料會遺失
- 與前一版相同儲存 key，更新同網域網站不會清掉原有記錄

## 本機開啟

解壓縮 ZIP，雙擊 `index.html` 即可預覽。建議使用 Chrome 或 Edge；瀏覽器在 `file://` 模式可能限制本機儲存，部署後使用 HTTPS 更穩定。

## Cloudflare Pages 部署（Direct Upload）

1. 前往 https://dash.cloudflare.com/ 登入。
2. 選擇 **Workers & Pages** → 建立應用程式 → **Pages** → **Direct Upload / Drag and drop**。
3. 新增專案名稱，例如 `my-expense-tracker`。
4. 上傳解壓縮後的網站資料夾，或這份根目錄直接包含 `index.html` 的 ZIP。
5. 點擊 **Deploy**，取得 `*.pages.dev` 公開網址。

## 檔案結構

- `index.html`：網站結構
- `style.css`：網站視覺與響應式設計
- `app.js`：記帳操作、篩選、統計、圖表、匯出
- `README.md`：使用說明

## 注意

此網站是教學展示用；所有收支資料保存在本機瀏覽器，無後端帳號或跨裝置同步功能，建議定期匯出 CSV 備份。

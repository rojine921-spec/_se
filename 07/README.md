# 🤖 AI 工具使用說明

**本專案有使用 AI 工具 ChatGPT 協助開發。**

- **使用的 AI 工具：** ChatGPT（OpenAI）
- **AI 協助內容：** 網站主題發想、功能規劃、HTML／CSS／JavaScript 程式撰寫、版面與視覺美化、問題排查，以及 Cloudflare Pages 部署流程說明。
- **實作方式：** 將網站程式碼上傳 GitHub，連接 Cloudflare Pages 部署，並整理網站成果截圖與 README 說明。

# 第 9 週｜理財日常：智慧記帳與消費分析網站

這是一個使用 **HTML、CSS、JavaScript** 製作的記帳網站，可新增收入與支出、查看每月結餘與消費分類分析，並透過 **Cloudflare Pages** 公開部署。無需安裝套件或建立後端伺服器。

**網站連結：** https://expense-tracker-07.pages.dev

## 網站成果截圖

![智慧記帳與消費分析網站首頁](images/1.png)

## 網站特色

- 紫色系視覺設計、側邊選單與手機自適應排版
- 新增收入與支出，記錄金額、分類和日期
- 按月份檢視收入、支出與結餘
- 消費分類分析圖表與各類別占比
- 搜尋項目、篩選收支及刪除紀錄
- 匯出指定月份 CSV（可用 Excel 開啟）
- 使用瀏覽器 `localStorage` 儲存資料，無須註冊帳號

## 使用技術

- **HTML：** 網頁內容與結構
- **CSS：** 網頁美化、卡片排版與響應式設計
- **JavaScript：** 記帳操作、統計、圖表與資料匯出
- **GitHub：** 儲存與管理程式碼
- **Cloudflare Pages：** 網站部署與公開瀏覽
- **ChatGPT：** 協助網站開發與技術問題處理

## 使用方式

1. 開啟網站：https://expense-tracker-07.pages.dev
2. 點選「新增記帳」，輸入收入或支出資料。
3. 在總覽儀表板查看本月收支與結餘。
4. 使用交易紀錄與分類分析功能檢視消費情況。
5. 需要備份時，可將資料匯出為 CSV。

## GitHub 與 Cloudflare Pages 部署

1. 將 `07` 資料夾的網站檔案上傳至 GitHub。
2. 在 Cloudflare Workers 和 Pages 中建立 **Pages** 專案，連接 GitHub 儲存庫。
3. 選擇 `main` 分支，Framework 預設為「無」，組建命令留空，組建輸出目錄設為 `07`（根目錄維持 `/`）。
4. 部署成功後取得 `.pages.dev` 公開網址。

## 專案檔案結構

```text
07/
├── index.html     # 網站頁面
├── style.css      # 版面與視覺設計
├── app.js         # 記帳與分析功能
├── README.md      # 專案說明
└── images/
    └── 1.png      # 網站成果截圖
```

## 注意事項

本網站為課程練習作品。記帳資料儲存在目前瀏覽器的 `localStorage` 中，**不會跨裝置同步**；清除瀏覽器網站資料可能使記錄遺失，建議定期使用 CSV 匯出備份。網站的記帳功能是 JavaScript 實作，**不是內建 AI 自動分析服務**；AI 的角色是協助開發本專案。

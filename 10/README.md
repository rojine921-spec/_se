# 第十週：Expense Tracker Utils — npm 套件發布

## AI 工具使用說明

本專案使用 OpenAI ChatGPT（GPT-6）輔助理解 npm 套件架構、整理 JavaScript 工具函式、撰寫測試程式、README 文件及協助完成 npm 發布流程。

## 專案介紹

本專案使用 Node.js 與 JavaScript 製作 npm 套件，將第九週「智慧記帳與消費分析網站」的記帳統計功能整理成可重複使用的工具函式，並發布至 npm 公開套件庫。

- **套件名稱：** `expense-tracker-utils-rojine921`
- **版本：** `1.0.0`
- **程式語言：** JavaScript / Node.js
- **發布平台：** npm
- **授權方式：** MIT

## 專案連結

**第九週 Cloudflare Pages 網站：**

https://expense-tracker-07.pages.dev

**第十週 npm 公開套件：**

https://www.npmjs.com/package/expense-tracker-utils-rojine921

**GitHub 程式碼：**

https://github.com/rojine921-spec/_se/tree/main/10

## 套件功能

1. `totals()`：計算收入總額、支出總額及結餘。
2. `categorySummary()`：統計各支出分類的金額。
3. `filterByMonth()`：依照指定月份篩選交易紀錄。
4. `toCSV()`：將交易資料轉換成 CSV 格式，方便匯出到 Excel。

## 安裝方式（Install）

使用 npm 安裝：

```bash
npm install expense-tracker-utils-rojine921
```

## 使用範例（Usage）

```javascript
const {
  totals,
  categorySummary,
  filterByMonth,
  toCSV
} = require('expense-tracker-utils-rojine921');

const transactions = [
  {
    date: '2026-11-01',
    type: 'income',
    category: '薪資',
    amount: 30000
  },
  {
    date: '2026-11-02',
    type: 'expense',
    category: '餐飲',
    amount: 200
  }
];

// 計算收入、支出與結餘
console.log(totals(transactions));

// 結果：
// { income: 30000, expense: 200, balance: 29800 }

// 支出分類統計
console.log(categorySummary(transactions));

// 結果：
// { '餐飲': 200 }

// 篩選 2026 年 11 月的交易
console.log(filterByMonth(transactions, '2026-11'));

// 匯出 CSV
console.log(toCSV(transactions));
```

## 資料格式

每筆交易資料包含：

- `date`：交易日期，格式為 YYYY-MM-DD。
- `type`：收入 income 或支出 expense。
- `amount`：交易金額，必須大於或等於 0。
- `category`：交易分類，可省略。
- `note`：備註，可省略。

## 測試方式（Test）

執行測試：

```bash
npm test
```

檢查套件打包內容：

```bash
npm pack --dry-run
```

## 發布方式（Publish）

先註冊 npm 帳號並完成身分驗證。

```bash
npm login
npm whoami
npm publish
```

本專案已成功發布至 npm 公開套件庫，版本為 `1.0.0`。

可以使用以下指令查詢：

```bash
npm view expense-tracker-utils-rojine921 version
```

查詢結果：

```text
1.0.0
```

## 開發說明

本套件是從第九週智慧記帳網站的功能概念獨立整理而成，沒有直接複製原網站的 `app.js`。

目前已完成 npm 套件發布，但尚未將套件整合回 Cloudflare Pages 網站。

## 作業成果

- 完成 Node.js / JavaScript 工具套件。
- 完成 npm 套件打包及測試。
- 成功發布 npm 公開套件 `1.0.0`。
- 將原始程式碼提交至 GitHub。
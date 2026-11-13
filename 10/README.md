# Expense Tracker Utils

第十週 Node.js / npm 套件發布作業。從第九週「智慧記帳與消費分析網站」抽出可重複使用的記帳統計工具。

原 Cloudflare Pages 網站：https://expense-tracker-07.pages.dev

## Install

```bash
npm install expense-tracker-utils-rojine921
```

## Usage

```js
const { totals, categorySummary, filterByMonth, toCSV } = require('expense-tracker-utils-rojine921');
const transactions = [
  { date: '2026-11-01', type: 'income', category: '薪資', amount: 30000 },
  { date: '2026-11-02', type: 'expense', category: '餐飲', amount: 200 }
];
console.log(totals(transactions)); // { income: 30000, expense: 200, balance: 29800 }
console.log(categorySummary(transactions)); // { '餐飲': 200 }
console.log(filterByMonth(transactions, '2026-11'));
console.log(toCSV(transactions));
```

每筆資料的 `type` 為 `income` 或 `expense`，`amount` 為大於或等於零的數字；`date` 使用 `YYYY-MM-DD` 字串。 `category` 和 `note` 可省略。

## Test

```bash
npm test
npm pack --dry-run
```

## Publish

先到 https://www.npmjs.com/signup 註冊、驗證 email 並設定雙因素驗證，然後：

```bash
npm login
npm whoami
npm publish
```

若名稱已被使用，請先修改 `package.json` 的 `name` 並同步更新 README 中的套件安裝名稱。發布前請檢查 `npm pack --dry-run` 顯示的打包檔案。不要提交密碼、API Token 或 `.npmrc`。

發布後公開網址格式：`https://www.npmjs.com/package/expense-tracker-utils-rojine921`（只有實際發布成功後才會存在）。

## 注意

這是從原網站功能「另行整理」的獨立工具函式，未直接取用原 `app.js`，也尚未接入原 Cloudflare Pages 網站。未來可把套件整合到前端建置流程中。

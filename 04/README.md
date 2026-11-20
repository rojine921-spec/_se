111410505 翁沂慶

母專案 -- https://github.com/111410505-se/github_se/tree/main

分支 -- https://github.com/111410505-se/github_se/tree/developGitBranch

子專案 --https://github.com/rojine921-spec/github_se

# Git 分支、合併、Fork、Pull Request 操作說明

這份 README.md 記錄我在 Git 與 GitHub 上完成以下四個操作時，使用的指令與步驟：

1. 建立分支
2. 合併分支
3. Fork 專案
4. 建立 Pull Request

## 這是哪一種 Git 流程？

這次練習其實結合了兩種常見的 Git 協作流程：

### 1. Feature Branch Workflow（功能分支流程）

我先從 `main` 建立一個新的 `developGitBranch` 分支，在分支上修改檔案、Commit，再把分支推到 GitHub。

流程大致是：

```text
main
  ↓
建立 developGitBranch
  ↓
修改檔案
  ↓
git add
  ↓
git commit
  ↓
git push
  ↓
合併回 main
```

這種做法的好處是：不直接在 `main` 上修改，可以把每個功能或作業放在獨立分支中，完成後再合併。

這個流程和 **GitHub Flow** 的概念很接近。GitHub Flow 通常也是從 `main` 建立分支、修改、Commit、Push，接著透過 Pull Request 審查後再合併回 `main`。

### 2. Forking Workflow（Fork 工作流程）

Fork 則是另一種常見的 GitHub 協作方式。

它通常用在「不是直接在原始 Repository 裡開分支」的情況，而是：

```text
原始 Repository
        ↓
       Fork
        ↓
自己的 Repository
        ↓
建立 / 修改分支
        ↓
Push
        ↓
Pull Request
        ↓
請求合併回原始 Repository
```

所以這次作業不是單一一種 Git 流程，而是同時練習：

- **Feature Branch Workflow / GitHub Flow**：分支、Commit、Push、Merge
- **Forking Workflow**：Fork、Pull Request

---

---

## 1. 建立分支 Branch

先確認目前所在的分支：

```bash
git branch
```

建立一個新的分支 `developGitBranch`，並直接切換到該分支：

```bash
git checkout -b developGitBranch
```

也可以使用較新的寫法：

```bash
git switch -c developGitBranch
```

其中：

- `-b`：建立新分支並切換
- `-c`：代表 `--create`，效果和 `checkout -b` 相同

再用下面的指令確認目前所在分支：

```bash
git branch
```

如果看到：

```text
* developGitBranch
  main
```

前面的 `*` 代表目前正在 `developGitBranch` 分支。

接著把修改過的 Markdown 檔案加入暫存區：

```bash
git add *.md
```

建立 Commit：

```bash
git commit -m "add gitBranch.md"
```

最後把新的分支推送到 GitHub：

```bash
git push origin developGitBranch
```

---

## 2. 合併分支 Merge

先切回 `main` 分支：

```bash
git checkout main
```

或：

```bash
git switch main
```

接著把 `developGitBranch` 合併到 `main`：

```bash
git merge developGitBranch
```

如果沒有衝突，會看到類似：

```text
Fast-forward
```

代表合併成功。

最後把合併後的 `main` 推送到 GitHub：

```bash
git push origin main
```

---

## 3. Fork 專案

Fork 是在 GitHub 網頁上操作。

因為我的 Repository 是 Private，而且屬於 Organization，所以一開始無法 Fork。

我先到 Organization 的設定：

```text
Organization
→ Settings
→ Member privileges
→ Repository forking
```

勾選：

```text
Allow forking of private repositories
```

並按下 `Save`。

接著回到 Repository：

```text
Repository
→ Settings
→ General
```

找到：

```text
Allow forking
```

將它勾選開啟。

最後回到 Repository 首頁，按右上角：

```text
Fork
```

即可建立一份 Fork 的 Repository。

---

## 4. 建立 Pull Request

先把自己的分支推送到 GitHub：

```bash
git push origin developGitBranch
```

推送完成後，GitHub 會提示可以建立 Pull Request。

也可以直接到 GitHub Repository：

```text
Pull requests
→ New pull request
```

選擇：

```text
base: main
compare: developGitBranch
```

確認修改內容後，按：

```text
Create pull request
```

填寫標題與說明，再按一次：

```text
Create pull request
```

這樣就完成 Pull Request。

如果確認內容沒有問題，之後也可以在 GitHub 上按：

```text
Merge pull request
```

把 Pull Request 合併到 `main`。

---

## 操作流程整理

```text
建立分支
git checkout -b developGitBranch
        ↓
修改檔案
        ↓
git add *.md
        ↓
git commit -m "add gitBranch.md"
        ↓
git push origin developGitBranch
        ↓
建立 Pull Request
        ↓
合併到 main
```

GitHub 的 Fork 則是先開啟 Repository 的 Fork 權限，再按 GitHub 頁面上的 `Fork` 按鈕完成。

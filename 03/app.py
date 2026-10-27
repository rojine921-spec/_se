"""AI 智慧記帳與消費分析｜僅使用 Python 標準函式庫。"""
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from pathlib import Path
from datetime import date, datetime
from collections import defaultdict
import sqlite3, json, csv, io, re, os

ROOT = Path(__file__).resolve().parent
DB = Path(os.getenv('EXPENSE_DB', str(ROOT / 'expenses.db')))
CATEGORIES = ['餐飲','交通','購物','居家','娛樂','醫療','教育','其他']
KEYWORDS = {
 '餐飲':['早餐','午餐','晚餐','便當','飲料','咖啡','牛肉麵','全聯','超商','麵包','餐廳','火鍋','手搖','早餐店'],
 '交通':['捷運','公車','高鐵','台鐵','火車','計程車','uber','加油','停車','機票'],
 '購物':['蝦皮','momo','pchome','衣服','鞋子','網購','耳機','包包'],
 '居家':['水費','電費','瓦斯','房租','網路費','家具','日用品'],
 '娛樂':['電影','遊戲','netflix','演唱會','ktv','訂閱'],
 '醫療':['看診','掛號','藥局','牙醫','醫院','保健'],
 '教育':['學費','書籍','課本','課程','補習','文具'],
}

def predict_category(note):
    """Explainable keyword-based smart classification, not a trained AI model."""
    note = note.lower()
    scores = {cat: sum(len(k) for k in words if k.lower() in note) for cat, words in KEYWORDS.items()}
    best = max(scores, key=scores.get)
    return best if scores[best] else '其他'

def connect():
    DB.parent.mkdir(parents=True, exist_ok=True)
    c = sqlite3.connect(DB)
    c.row_factory = sqlite3.Row
    c.execute('CREATE TABLE IF NOT EXISTS expenses (id INTEGER PRIMARY KEY AUTOINCREMENT, spend_date TEXT NOT NULL, amount INTEGER NOT NULL CHECK(amount>0), note TEXT NOT NULL, category TEXT NOT NULL, created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)')
    c.execute('CREATE TABLE IF NOT EXISTS budgets (month TEXT PRIMARY KEY, amount INTEGER NOT NULL CHECK(amount>0))')
    c.commit()
    return c

def validate_date(s):
    try: return date.fromisoformat(s).isoformat() == s
    except (ValueError,TypeError): return False

def month_ok(s):
    return bool(re.fullmatch(r'\d{4}-(0[1-9]|1[0-2])',str(s)))

def summary(month):
    with connect() as c:
        items = [dict(r) for r in c.execute('SELECT id,spend_date,amount,note,category FROM expenses WHERE substr(spend_date,1,7)=? ORDER BY spend_date DESC,id DESC',(month,))]
        budget_row = c.execute('SELECT amount FROM budgets WHERE month=?',(month,)).fetchone()
        previous = str((date(int(month[:4]),int(month[5:]),1).replace(day=1)))[:7]
        year, mon = map(int,month.split('-')); previous = f'{year-1}-12' if mon==1 else f'{year}-{mon-1:02d}'
        prev_total = c.execute('SELECT COALESCE(SUM(amount),0) FROM expenses WHERE substr(spend_date,1,7)=?',(previous,)).fetchone()[0]
    categories = {cat:sum(x['amount'] for x in items if x['category']==cat) for cat in CATEGORIES}
    total = sum(x['amount'] for x in items)
    days=defaultdict(int)
    for x in items: days[x['spend_date']]+=x['amount']
    budget=budget_row[0] if budget_row else None
    alerts=[]
    if budget:
        ratio=total/budget
        if ratio>=1: alerts.append('本月支出已超過或達到預算，建議檢視高支出類別。')
        elif ratio>=.8: alerts.append('本月已使用超過 80% 預算，建議留意剩餘可用金額。')
    highest = max(categories,key=categories.get)
    if total: alerts.append(f'本月支出最高類別為「{highest}」，占 {categories[highest]/total:.0%}。')
    if prev_total and total>prev_total: alerts.append(f'本月支出目前比上個月整月增加 {total-prev_total:,} 元；請留意月份尚未結束時比較可能失真。')
    return dict(items=items,total=total,count=len(items),categories=categories,daily=[{'date':k,'amount':v} for k,v in sorted(days.items())],budget=budget,previous_total=prev_total,insights=alerts,remaining=None if budget is None else budget-total)

class Handler(BaseHTTPRequestHandler):
    def log_message(self,format,*args): pass
    def reply(self,code,data,ctype='application/json; charset=utf-8',extra=None):
        buf = json.dumps(data,ensure_ascii=False).encode() if not isinstance(data,bytes) else data
        self.send_response(code);self.send_header('Content-Type',ctype);self.send_header('Content-Length',str(len(buf)))
        self.send_header('Cache-Control','no-store')
        if extra:
            for k,v in extra.items():self.send_header(k,v)
        self.end_headers();self.wfile.write(buf)
    def body(self):
        length=int(self.headers.get('Content-Length','0'))
        if length>1_000_000: raise ValueError('資料大小超過限制')
        try: return json.loads(self.rfile.read(length))
        except (json.JSONDecodeError,UnicodeError):raise ValueError('JSON 格式不正確')
    def do_GET(self):
        u=urlparse(self.path);q=parse_qs(u.query)
        if u.path=='/': return self.reply(200,(ROOT/'index.html').read_bytes(),'text/html; charset=utf-8')
        month=q.get('month',[date.today().strftime('%Y-%m')])[0]
        if not month_ok(month): return self.reply(400,{'error':'月份格式應為 YYYY-MM'})
        if u.path=='/api/summary': return self.reply(200,summary(month))
        if u.path=='/api/export':
            output=io.StringIO(); w=csv.writer(output);w.writerow(['日期','金額','說明','分類'])
            for x in summary(month)['items']: w.writerow([x['spend_date'],x['amount'],x['note'],x['category']])
            return self.reply(200,output.getvalue().encode('utf-8-sig'),'text/csv; charset=utf-8',{'Content-Disposition':f'attachment; filename="expenses-{month}.csv"'})
        return self.reply(404,{'error':'找不到頁面'})
    def do_POST(self):
        try:
            data=self.body()
            if not isinstance(data,dict): raise ValueError('資料格式錯誤')
            if self.path=='/api/predict': return self.reply(200,{'category':predict_category(str(data.get('note','')))})
            if self.path=='/api/expense':
                day=data.get('date');note=str(data.get('note','')).strip();cat=data.get('category') or predict_category(note)
                if not validate_date(day):raise ValueError('日期格式應為 YYYY-MM-DD')
                if not note or len(note)>100:raise ValueError('說明應有 1 至 100 字')
                if cat not in CATEGORIES:raise ValueError('分類不正確')
                try: amount=int(data.get('amount'))
                except (ValueError,TypeError):raise ValueError('金額必須是整數')
                if str(data.get('amount'))!=str(amount) or not 1<=amount<=100_000_000:raise ValueError('金額需為 1 至 100,000,000 的整數')
                with connect() as c: c.execute('INSERT INTO expenses(spend_date,amount,note,category) VALUES (?,?,?,?)',(day,amount,note,cat))
                return self.reply(201,{'ok':True})
            if self.path=='/api/budget':
                month=data.get('month')
                if not month_ok(month):raise ValueError('月份格式錯誤')
                try: amount=int(data.get('amount'))
                except (ValueError,TypeError):raise ValueError('預算必須是整數')
                if str(data.get('amount'))!=str(amount) or not 1<=amount<=100_000_000:raise ValueError('預算必須是正整數')
                with connect() as c: c.execute('INSERT INTO budgets(month,amount) VALUES (?,?) ON CONFLICT(month) DO UPDATE SET amount=excluded.amount',(month,amount))
                return self.reply(200,{'ok':True})
            if self.path=='/api/delete':
                try: ident=int(data.get('id'))
                except (ValueError,TypeError):raise ValueError('ID 錯誤')
                with connect() as c: c.execute('DELETE FROM expenses WHERE id=?',(ident,))
                return self.reply(200,{'ok':True})
            return self.reply(404,{'error':'找不到 API'})
        except ValueError as e: return self.reply(400,{'error':str(e)})

if __name__=='__main__':
    connect().close()
    print('智慧記帳網站已啟動：http://localhost:8000')
    ThreadingHTTPServer(('127.0.0.1',8000),Handler).serve_forever()

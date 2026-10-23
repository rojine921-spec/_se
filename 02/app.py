"""模擬大學校務系統：Flask + SQLite，僅使用虛構示範資料。"""
import os
import sqlite3
from functools import wraps
from pathlib import Path
from flask import Flask, abort, flash, g, redirect, render_template, request, session, url_for
from werkzeug.security import check_password_hash, generate_password_hash

BASE = Path(__file__).resolve().parent
DB = BASE / 'campus.db'
app = Flask(__name__)
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'demo-only-change-me-before-deployment')
app.config.update(SESSION_COOKIE_HTTPONLY=True, SESSION_COOKIE_SAMESITE='Lax')


def database():
    if 'db' not in g:
        g.db = sqlite3.connect(DB)
        g.db.row_factory = sqlite3.Row
        g.db.execute('PRAGMA foreign_keys = ON')
    return g.db


@app.teardown_appcontext
def close_db(_error):
    db = g.pop('db', None)
    if db is not None:
        db.close()


def init_db():
    with app.app_context():
        db = database()
        db.executescript('''
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                name TEXT NOT NULL,
                department TEXT NOT NULL,
                role TEXT NOT NULL CHECK(role IN ('student','admin'))
            );
            CREATE TABLE IF NOT EXISTS courses (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                code TEXT NOT NULL UNIQUE,
                name TEXT NOT NULL,
                teacher TEXT NOT NULL,
                credits INTEGER NOT NULL CHECK(credits > 0),
                capacity INTEGER NOT NULL CHECK(capacity >= 1),
                weekday TEXT NOT NULL,
                period TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS enrollments (
                user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
                course_id INTEGER NOT NULL REFERENCES courses(id) ON DELETE CASCADE,
                grade INTEGER CHECK(grade BETWEEN 0 AND 100),
                PRIMARY KEY(user_id, course_id)
            );
        ''')
        if db.execute('SELECT COUNT(*) FROM users').fetchone()[0] == 0:
            db.executemany('INSERT INTO users(username,password_hash,name,department,role) VALUES(?,?,?,?,?)', [
                ('s001', generate_password_hash('student123'), '王小明', '資訊工程學系', 'student'),
                ('s002', generate_password_hash('student123'), '李小華', '企業管理學系', 'student'),
                ('admin', generate_password_hash('admin123'), '系統管理員', '教務處', 'admin'),
            ])
        if db.execute('SELECT COUNT(*) FROM courses').fetchone()[0] == 0:
            db.executemany('INSERT INTO courses(code,name,teacher,credits,capacity,weekday,period) VALUES(?,?,?,?,?,?,?)', [
                ('CS101','程式設計','陳老師',3,35,'週一','第 1–3 節'),
                ('CS202','資料結構','林老師',3,30,'週二','第 2–4 節'),
                ('CS203','資料庫系統','張老師',3,30,'週三','第 5–7 節'),
                ('CS301','計算機網路','吳老師',3,25,'週四','第 2–4 節'),
                ('GE110','通識：永續發展','許老師',2,40,'週五','第 3–4 節'),
            ])
        db.commit()


@app.before_request
def load_user():
    user_id = session.get('user_id')
    g.user = database().execute('SELECT * FROM users WHERE id=?', (user_id,)).fetchone() if user_id else None


def login_required(view):
    @wraps(view)
    def protected(*args, **kwargs):
        if g.user is None:
            return redirect(url_for('login'))
        return view(*args, **kwargs)
    return protected


def admin_required(view):
    @wraps(view)
    @login_required
    def protected(*args, **kwargs):
        if g.user['role'] != 'admin':
            abort(403)
        return view(*args, **kwargs)
    return protected


@app.get('/')
def index():
    return redirect(url_for('dashboard') if g.user else url_for('login'))


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username', '').strip()
        password = request.form.get('password', '')
        user = database().execute('SELECT * FROM users WHERE username=?', (username,)).fetchone()
        if user and check_password_hash(user['password_hash'], password):
            session.clear()
            session['user_id'] = user['id']
            return redirect(url_for('dashboard'))
        flash('帳號或密碼錯誤', 'error')
    return render_template('login.html')


@app.post('/logout')
@login_required
def logout():
    session.clear()
    flash('已登出', 'success')
    return redirect(url_for('login'))


@app.get('/dashboard')
@login_required
def dashboard():
    db = database()
    if g.user['role'] == 'admin':
        metrics = {
            '學生人數': db.execute("SELECT COUNT(*) FROM users WHERE role='student'").fetchone()[0],
            '開設課程': db.execute('SELECT COUNT(*) FROM courses').fetchone()[0],
            '選課人次': db.execute('SELECT COUNT(*) FROM enrollments').fetchone()[0],
        }
    else:
        row = db.execute('''SELECT COUNT(*) AS total, COALESCE(SUM(c.credits),0) AS credits
                            FROM enrollments e JOIN courses c ON e.course_id=c.id
                            WHERE e.user_id=?''', (g.user['id'],)).fetchone()
        metrics = {'已選課程': row['total'], '總學分數': row['credits']}
    return render_template('dashboard.html', metrics=metrics)


@app.get('/courses')
@login_required
def courses():
    db = database()
    rows = db.execute('''SELECT c.*, COUNT(e.user_id) AS enrolled,
        EXISTS(SELECT 1 FROM enrollments mine WHERE mine.course_id=c.id AND mine.user_id=?) AS selected
        FROM courses c LEFT JOIN enrollments e ON e.course_id=c.id
        GROUP BY c.id ORDER BY c.code''', (g.user['id'],)).fetchall()
    return render_template('courses.html', courses=rows)


@app.post('/enroll/<int:course_id>')
@login_required
def enroll(course_id):
    if g.user['role'] != 'student':
        abort(403)
    db = database()
    # 寫入交易避免同時間選課超收；SQLite 的 BEGIN IMMEDIATE 取得寫入鎖。
    db.execute('BEGIN IMMEDIATE')
    course = db.execute('SELECT * FROM courses WHERE id=?', (course_id,)).fetchone()
    if not course:
        db.rollback()
        abort(404)
    exists = db.execute('SELECT 1 FROM enrollments WHERE user_id=? AND course_id=?', (g.user['id'], course_id)).fetchone()
    count = db.execute('SELECT COUNT(*) FROM enrollments WHERE course_id=?', (course_id,)).fetchone()[0]
    if exists:
        flash('已選過這門課', 'error')
    elif count >= course['capacity']:
        flash('課程已額滿', 'error')
    else:
        db.execute('INSERT INTO enrollments(user_id,course_id) VALUES(?,?)', (g.user['id'], course_id))
        flash('選課成功', 'success')
    db.commit()
    return redirect(url_for('courses'))


@app.post('/drop/<int:course_id>')
@login_required
def drop(course_id):
    if g.user['role'] != 'student':
        abort(403)
    db = database()
    row = db.execute('SELECT grade FROM enrollments WHERE user_id=? AND course_id=?', (g.user['id'], course_id)).fetchone()
    if row is None:
        flash('未選修此課程', 'error')
    elif row['grade'] is not None:
        flash('已評分課程不能退選', 'error')
    else:
        db.execute('DELETE FROM enrollments WHERE user_id=? AND course_id=?', (g.user['id'], course_id))
        db.commit()
        flash('退選成功', 'success')
    return redirect(url_for('my_courses'))


@app.get('/my-courses')
@login_required
def my_courses():
    if g.user['role'] != 'student':
        abort(403)
    rows = database().execute('''SELECT c.*, e.grade FROM enrollments e JOIN courses c ON c.id=e.course_id
        WHERE e.user_id=? ORDER BY c.code''', (g.user['id'],)).fetchall()
    return render_template('my_courses.html', courses=rows)


@app.get('/grades')
@login_required
def grades():
    if g.user['role'] != 'student':
        abort(403)
    rows = database().execute('''SELECT c.code,c.name,c.credits,e.grade FROM enrollments e
        JOIN courses c ON c.id=e.course_id WHERE e.user_id=? ORDER BY c.code''', (g.user['id'],)).fetchall()
    scored = [r for r in rows if r['grade'] is not None]
    average = round(sum(r['grade'] * r['credits'] for r in scored) / sum(r['credits'] for r in scored), 1) if scored else None
    return render_template('grades.html', grades=rows, average=average)


@app.route('/admin', methods=['GET','POST'])
@admin_required
def admin():
    db = database()
    if request.method == 'POST':
        try:
            course_id = int(request.form.get('course_id',''))
            student_id = int(request.form.get('student_id',''))
            grade = int(request.form.get('grade',''))
        except ValueError:
            flash('請輸入有效的學生、課程及 0～100 分成績', 'error')
        else:
            if not 0 <= grade <= 100:
                flash('成績必須介於 0～100 分', 'error')
            else:
                result = db.execute('UPDATE enrollments SET grade=? WHERE user_id=? AND course_id=?', (grade,student_id,course_id))
                db.commit()
                flash('成績已更新' if result.rowcount else '查無該選課紀錄', 'success' if result.rowcount else 'error')
        return redirect(url_for('admin'))
    students = db.execute("SELECT id,username,name FROM users WHERE role='student' ORDER BY username").fetchall()
    records = db.execute('''SELECT u.id AS student_id,u.username,u.name AS student_name,
        c.id AS course_id,c.code,c.name AS course_name,e.grade
        FROM enrollments e JOIN users u ON e.user_id=u.id JOIN courses c ON e.course_id=c.id
        ORDER BY u.username,c.code''').fetchall()
    return render_template('admin.html', students=students, records=records)


@app.errorhandler(403)
def forbidden(_error):
    return render_template('error.html', message='沒有權限瀏覽此頁面'), 403


@app.errorhandler(404)
def not_found(_error):
    return render_template('error.html', message='找不到這個頁面或課程'), 404


if __name__ == '__main__':
    init_db()
    app.run(debug=True, host='127.0.0.1', port=5000)

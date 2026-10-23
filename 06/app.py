from pathlib import Path
import os
import sqlite3
from flask import Flask, jsonify, render_template, request


def validate_task(title):
    """Pure server-side validation used by the API."""
    if not isinstance(title, str):
        return False, '標題必須是文字'
    title = title.strip()
    if not title:
        return False, '請輸入待辦事項'
    if len(title) > 100:
        return False, '標題不可超過100字'
    return True, title


def create_app(test_config=None):
    app = Flask(__name__)
    app.config['DATABASE'] = str(Path(__file__).with_name('tasks.db'))
    app.config['TESTING'] = False
    if test_config:
        app.config.update(test_config)

    def db_connection():
        conn = sqlite3.connect(app.config['DATABASE'])
        conn.row_factory = sqlite3.Row
        return conn

    def init_db():
        with db_connection() as conn:
            conn.execute('''CREATE TABLE IF NOT EXISTS tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                completed INTEGER NOT NULL DEFAULT 0
            )''')

    init_db()

    @app.get('/')
    def index():
        return render_template('index.html')

    @app.get('/api/tasks')
    def list_tasks():
        with db_connection() as conn:
            rows = conn.execute('SELECT id, title, completed FROM tasks ORDER BY id DESC').fetchall()
        return jsonify([{'id': r['id'], 'title': r['title'], 'completed': bool(r['completed'])} for r in rows])

    @app.post('/api/tasks')
    def add_task():
        data = request.get_json(silent=True)
        if not isinstance(data, dict):
            return jsonify(error='請提供 JSON 物件'), 400
        valid, value = validate_task(data.get('title'))
        if not valid:
            return jsonify(error=value), 400
        with db_connection() as conn:
            cur = conn.execute('INSERT INTO tasks (title) VALUES (?)', (value,))
            task_id = cur.lastrowid
        return jsonify(id=task_id, title=value, completed=False), 201

    @app.patch('/api/tasks/<int:task_id>')
    def update_task(task_id):
        data = request.get_json(silent=True)
        if not isinstance(data, dict) or not isinstance(data.get('completed'), bool):
            return jsonify(error='completed 必須是布林值'), 400
        with db_connection() as conn:
            exists = conn.execute('SELECT id FROM tasks WHERE id=?', (task_id,)).fetchone()
            if not exists:
                return jsonify(error='找不到待辦事項'), 404
            conn.execute('UPDATE tasks SET completed=? WHERE id=?', (int(data['completed']), task_id))
        return jsonify(id=task_id, completed=data['completed'])

    @app.delete('/api/tasks/<int:task_id>')
    def delete_task(task_id):
        with db_connection() as conn:
            cur = conn.execute('DELETE FROM tasks WHERE id=?', (task_id,))
            if cur.rowcount == 0:
                return jsonify(error='找不到待辦事項'), 404
        return '', 204

    return app


if __name__ == '__main__':
    create_app().run(debug=True)

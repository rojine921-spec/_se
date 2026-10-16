'use strict';
const http = require('node:http');
const { Pool } = require('pg');

const pool = new Pool({
  host: process.env.DB_HOST || 'db',
  port: Number(process.env.DB_PORT || 5432),
  user: process.env.DB_USER || 'demo_user',
  password: process.env.DB_PASSWORD,
  database: process.env.DB_NAME || 'docker_demo',
  max: 10,
  connectionTimeoutMillis: 3000,
  idleTimeoutMillis: 10000,
});
const port = Number(process.env.PORT || 3000);

function json(res, statusCode, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(statusCode, {
    'Content-Type': 'application/json; charset=utf-8',
    'Cache-Control': 'no-store',
    'X-Content-Type-Options': 'nosniff',
  });
  res.end(body);
}

async function readJson(req) {
  if (!(req.headers['content-type'] || '').toLowerCase().startsWith('application/json')) {
    const error = new Error('Content-Type 必須是 application/json');
    error.statusCode = 415;
    throw error;
  }
  const chunks = [];
  let length = 0;
  for await (const chunk of req) {
    length += chunk.length;
    if (length > 8192) {
      const error = new Error('請求內容太大');
      error.statusCode = 413;
      throw error;
    }
    chunks.push(chunk);
  }
  try {
    return JSON.parse(Buffer.concat(chunks).toString('utf8'));
  } catch {
    const error = new Error('JSON 格式錯誤');
    error.statusCode = 400;
    throw error;
  }
}

const server = http.createServer(async (req, res) => {
  try {
    const pathname = new URL(req.url, 'http://localhost').pathname;

    if (req.method === 'GET' && pathname === '/health') {
      await pool.query('SELECT 1');
      return json(res, 200, { status: 'ok', service: 'node-api', database: 'connected' });
    }

    if (req.method === 'GET' && pathname === '/tasks') {
      const result = await pool.query('SELECT id, title, completed, created_at FROM tasks ORDER BY id DESC LIMIT 100');
      return json(res, 200, { tasks: result.rows });
    }

    if (req.method === 'GET' && pathname === '/summary') {
      const result = await pool.query('SELECT COUNT(*)::int AS total, COUNT(*) FILTER (WHERE completed)::int AS done FROM tasks');
      const { total, done } = result.rows[0];
      return json(res, 200, { total, done, pending: total - done });
    }

    if (req.method === 'POST' && pathname === '/tasks') {
      const body = await readJson(req);
      const title = typeof body?.title === 'string' ? body.title.trim() : '';
      if (!title || [...title].length > 120) {
        return json(res, 400, { error: '任務名稱須為 1～120 個字元' });
      }
      const result = await pool.query(
        'INSERT INTO tasks (title) VALUES ($1) RETURNING id, title, completed, created_at',
        [title]
      );
      return json(res, 201, { task: result.rows[0] });
    }

    const match = pathname.match(/^\/tasks\/(\d+)$/);
    if (match && req.method === 'PATCH') {
      const body = await readJson(req);
      if (typeof body?.completed !== 'boolean') {
        return json(res, 400, { error: 'completed 必須是布林值' });
      }
      const result = await pool.query(
        'UPDATE tasks SET completed = $1 WHERE id = $2 RETURNING id, title, completed, created_at',
        [body.completed, match[1]]
      );
      if (!result.rowCount) return json(res, 404, { error: '找不到此任務' });
      return json(res, 200, { task: result.rows[0] });
    }

    if (match && req.method === 'DELETE') {
      const result = await pool.query('DELETE FROM tasks WHERE id = $1 RETURNING id', [match[1]]);
      if (!result.rowCount) return json(res, 404, { error: '找不到此任務' });
      return json(res, 200, { deleted: true, id: result.rows[0].id });
    }

    return json(res, 404, { error: 'API 路徑不存在' });
  } catch (error) {
    const status = error.statusCode || (error.code && /^(?:ECONN|ENOTFOUND|EAI_AGAIN)/.test(error.code) ? 503 : 500);
    if (status >= 500) console.error('API error:', error);
    return json(res, status, { error: status >= 500 ? '服務暫時無法使用' : error.message });
  }
});

server.listen(port, '0.0.0.0', () => console.log(`Node API listening on ${port}`));
process.on('SIGTERM', () => server.close(() => pool.end()));

'use strict';

const $ = (id) => document.getElementById(id);

async function request(path, options = {}) {
  const response = await fetch(path, {
    ...options,
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
    cache: 'no-store',
  });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || `HTTP ${response.status}`);
  return data;
}

function status(id, online) {
  const element = $(id);
  element.textContent = online ? '運作中' : '無法連線';
  element.className = `status-badge ${online ? 'online' : 'offline'}`;
}

function feedback(message, success = false) {
  const element = $('feedback');
  element.textContent = message;
  element.className = `feedback${success ? ' success' : ''}`;
}

function renderTasks(tasks) {
  const list = $('task-list');
  list.replaceChildren();
  $('task-list-count').textContent = `${tasks.length} 筆（最多顯示 100 筆）`;
  if (!tasks.length) {
    const empty = document.createElement('li');
    empty.className = 'task-placeholder';
    empty.textContent = '目前沒有任務，從上方新增第一筆吧！';
    list.append(empty);
    return;
  }
  for (const task of tasks) {
    const row = document.createElement('li');
    const checkbox = document.createElement('input');
    checkbox.className = 'task-check';
    checkbox.type = 'checkbox';
    checkbox.checked = task.completed;
    checkbox.setAttribute('aria-label', `完成任務：${task.title}`);
    checkbox.addEventListener('change', async () => {
      checkbox.disabled = true;
      try {
        await request(`/api/tasks/${encodeURIComponent(task.id)}`, {
          method: 'PATCH', body: JSON.stringify({ completed: checkbox.checked }),
        });
        await refresh();
        feedback('任務狀態已更新', true);
      } catch (error) {
        checkbox.checked = !checkbox.checked;
        feedback(error.message);
      } finally { checkbox.disabled = false; }
    });

    const name = document.createElement('span');
    name.className = `task-text${task.completed ? ' completed' : ''}`;
    name.textContent = task.title;

    const remove = document.createElement('button');
    remove.type = 'button';
    remove.className = 'delete-button';
    remove.textContent = '×';
    remove.title = `刪除 ${task.title}`;
    remove.setAttribute('aria-label', `刪除任務：${task.title}`);
    remove.addEventListener('click', async () => {
      remove.disabled = true;
      try {
        await request(`/api/tasks/${encodeURIComponent(task.id)}`, { method: 'DELETE' });
        await refresh();
        feedback('任務已刪除', true);
      } catch (error) { feedback(error.message); remove.disabled = false; }
    });
    row.append(checkbox, name, remove);
    list.append(row);
  }
}

async function refresh() {
  const [health, tasks, report] = await Promise.allSettled([
    request('/api/health'), request('/api/tasks'), request('/api/report'),
  ]);
  const apiOnline = health.status === 'fulfilled' && health.value.status === 'ok';
  status('api-status', apiOnline);
  status('db-status', apiOnline && health.value.database === 'connected');
  status('report-status', report.status === 'fulfilled');

  if (tasks.status === 'fulfilled') {
    renderTasks(tasks.value.tasks);
  } else {
    $('task-list').replaceChildren();
    const errorRow = document.createElement('li');
    errorRow.className = 'task-placeholder';
    errorRow.textContent = `任務讀取失敗：${tasks.reason.message}`;
    $('task-list').append(errorRow);
  }

  if (report.status === 'fulfilled') {
    const data = report.value;
    $('total-count').textContent = data.total;
    $('done-count').textContent = data.done;
    $('pending-count').textContent = data.pending;
    $('rate-count').textContent = data.completion_rate;
    $('progress-fill').style.width = `${Math.min(100, Math.max(0, data.completion_rate))}%`;
    $('report-message').textContent = `${data.message}　目前完成率 ${data.completion_rate}%（由 Python 微服務計算）。`;
  } else {
    $('report-message').textContent = 'Python 報表暫時無法使用，請查看 docker compose logs report。';
  }
  $('updated-at').textContent = `最後更新 ${new Date().toLocaleTimeString('zh-TW', { hour12: false })}`;
}

$('task-form').addEventListener('submit', async (event) => {
  event.preventDefault();
  const input = $('task-input');
  const title = input.value.trim();
  if (!title) return;
  $('add-button').disabled = true;
  feedback('');
  try {
    await request('/api/tasks', { method: 'POST', body: JSON.stringify({ title }) });
    input.value = '';
    await refresh();
    feedback('任務新增成功，資料已存入 PostgreSQL', true);
  } catch (error) { feedback(error.message); }
  finally { $('add-button').disabled = false; }
});
$('refresh-button').addEventListener('click', () => refresh().catch((error) => feedback(error.message)));
refresh().catch((error) => feedback(error.message));
setInterval(() => refresh().catch(() => {}), 30000);

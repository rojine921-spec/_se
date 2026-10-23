const form = document.querySelector('#task-form');
const title = document.querySelector('#task-title');
const list = document.querySelector('#task-list');
const empty = document.querySelector('#empty');
const error = document.querySelector('#error');
async function api(url, options = {}) {
  const res = await fetch(url, options);
  if (!res.ok) {
    const result = await res.json().catch(() => ({}));
    throw new Error(result.error || '操作失敗');
  }
  return res.status === 204 ? null : res.json();
}
async function loadTasks() {
  const tasks = await api('/api/tasks');
  list.replaceChildren();
  empty.hidden = tasks.length !== 0;
  for (const task of tasks) {
    const li = document.createElement('li');
    const label = document.createElement('label');
    const check = document.createElement('input');
    check.type = 'checkbox'; check.checked = task.completed;
    check.setAttribute('aria-label', `完成 ${task.title}`);
    const span = document.createElement('span'); span.textContent = task.title;
    if (task.completed) span.classList.add('done');
    check.addEventListener('change', async () => {
      try { await api(`/api/tasks/${task.id}`, {method:'PATCH', headers:{'Content-Type':'application/json'}, body:JSON.stringify({completed:check.checked})}); await loadTasks(); }
      catch (e) { error.textContent=e.message; await loadTasks(); }
    });
    const remove = document.createElement('button'); remove.textContent='刪除'; remove.type='button';
    remove.setAttribute('aria-label', `刪除 ${task.title}`);
    remove.addEventListener('click', async () => {
      try { await api(`/api/tasks/${task.id}`, {method:'DELETE'}); await loadTasks(); }
      catch (e) { error.textContent=e.message; }
    });
    label.append(check,span); li.append(label,remove); list.append(li);
  }
}
form.addEventListener('submit', async e => {
  e.preventDefault(); error.textContent='';
  try { await api('/api/tasks', {method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({title:title.value})}); title.value=''; await loadTasks(); }
  catch (err) {error.textContent=err.message;}
});
loadTasks().catch(e => error.textContent=e.message);

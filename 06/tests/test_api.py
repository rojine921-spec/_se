def test_home(client):
    assert client.get('/').status_code == 200


def test_task_crud(client):
    created = client.post('/api/tasks', json={'title': '讀書'})
    assert created.status_code == 201
    task_id = created.json['id']
    assert created.json['title'] == '讀書'
    listed = client.get('/api/tasks')
    assert listed.status_code == 200
    assert len(listed.json) == 1
    updated = client.patch(f'/api/tasks/{task_id}', json={'completed': True})
    assert updated.status_code == 200
    assert client.get('/api/tasks').json[0]['completed'] is True
    assert client.delete(f'/api/tasks/{task_id}').status_code == 204
    assert client.get('/api/tasks').json == []


def test_bad_input(client):
    assert client.post('/api/tasks', json={'title': ''}).status_code == 400
    assert client.post('/api/tasks', data='no-json').status_code == 400
    assert client.patch('/api/tasks/99', json={'completed': True}).status_code == 404
    assert client.delete('/api/tasks/99').status_code == 404

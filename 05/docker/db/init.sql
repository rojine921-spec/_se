CREATE TABLE IF NOT EXISTS tasks (
  id BIGSERIAL PRIMARY KEY,
  title VARCHAR(120) NOT NULL CHECK (char_length(btrim(title)) > 0),
  completed BOOLEAN NOT NULL DEFAULT FALSE,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

INSERT INTO tasks (title, completed) VALUES
  ('啟動 Docker Compose 四個服務', TRUE),
  ('測試 Nginx → Node.js → PostgreSQL', FALSE),
  ('查看 Python 微服務統計報表', FALSE);

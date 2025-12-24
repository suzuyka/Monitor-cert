-- name: CREATE_USERS
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE,
    password TEXT,
    display_name TEXT
);

-- name: INSERT_USER
INSERT INTO users (username, password, display_name) VALUES (?, ?, ?);

-- name: CREATE_CERTS
CREATE TABLE IF NOT EXISTS certificates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    type TEXT,
    date TEXT,
    description TEXT,
    user_id INTEGER
);

-- name: CHECK_USER
SELECT id FROM users WHERE username = ? AND password = ?;

-- name: INSERT_CERT
INSERT INTO certificates (name, type, date, description, user_id)
VALUES (?, ?, ?, ?, ?);

-- name: UPDATE_CERT
UPDATE certificates
SET name = ?, type = ?, date = ?, description = ?
WHERE id = ?;

-- name: DELETE_CERT
DELETE FROM certificates WHERE id = ?;

-- name: SELECT_CERTS_BY_USER
SELECT id, name, type, date, description, user_id
FROM certificates WHERE user_id = ?;

-- name: GET_USER_ID
SELECT id FROM users WHERE username = ? AND password = ?;

-- name: CHECK_OLD_PASSWORD
SELECT id FROM users WHERE id = ? AND password = ?;

-- name: UPDATE_PASSWORD
UPDATE users SET password = ? WHERE id = ?;

-- name: CLEAR_CERTS_BY_USER
DELETE FROM certificates WHERE user_id = ?;

-- name: reset_certificates
DELETE FROM sqlite_sequence WHERE name = 'certificates';

-- name: ALTER_USERS_ADD_DISPLAY_NAME
ALTER TABLE IF NOT EXISTS users ADD COLUMN display_name TEXT;

-- name: UPDATE_DISPLAY_NAME
UPDATE users SET display_name = ? WHERE id = ?;

-- name: GET_DISPLAY_NAME
SELECT display_name FROM users WHERE id = ?;

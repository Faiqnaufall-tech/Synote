CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS projects (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id),
    name TEXT NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL,
    updated_at TIMESTAMP WITH TIME ZONE NOT NULL,
    deleted_at TIMESTAMP WITH TIME ZONE,
    version INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS notes (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id),
    title TEXT NOT NULL,
    body TEXT NOT NULL,
    project_id UUID REFERENCES projects(id),
    created_at TIMESTAMP WITH TIME ZONE NOT NULL,
    updated_at TIMESTAMP WITH TIME ZONE NOT NULL,
    deleted_at TIMESTAMP WITH TIME ZONE,
    version INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS tags (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id),
    name TEXT NOT NULL,
    UNIQUE (user_id, name)
);

CREATE TABLE IF NOT EXISTS note_tags (
    note_id UUID NOT NULL REFERENCES notes(id),
    tag_id UUID NOT NULL REFERENCES tags(id),
    PRIMARY KEY (note_id, tag_id)
);

CREATE TABLE IF NOT EXISTS summaries (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users(id),
    note_id UUID REFERENCES notes(id),
    project_id UUID REFERENCES projects(id),
    content TEXT NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL
);

CREATE TABLE IF NOT EXISTS sync_state (
    user_id UUID PRIMARY KEY REFERENCES users(id),
    last_sync_at TIMESTAMP WITH TIME ZONE,
    device_id TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_notes_updated_at ON notes(updated_at);

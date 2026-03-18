# Synote Aplikasi Desktop Catatan + AI

Struktur repo:
- `desktop/` : Aplikasi Qt C++ (SQLite + FTS5, offline-first)
- `backend/` : Backend Go + Postgres (Auth + Sync API)

## Desktop (Qt C++)
Prasyarat:
- Qt 6 (Widgets + Sql)
- CMake >= 3.16
- QtKeychain (opsional, untuk simpan API key di OS keychain)

Build cepat:
```bash
cd desktop
cmake -S . -B build
cmake --build build
```

Login:
- Saat app dibuka, login/register wajib terlebih dahulu.

## Backend (Go)
Prasyarat:
- Go 1.22+
- Postgres

Environment:
- `DATABASE_URL` (connection string Postgres)
- `JWT_SECRET` (random secret)
- `OPENAI_API_KEY` (API key OpenAI)
- `OPENAI_MODEL` (model id, contoh: `gpt-5.4`)

Jalankan server:
```bash
cd backend
go run ./cmd/server
```

Migrasi:
- Jalankan isi `backend/migrations/001_init.sql` di database kamu.

## Endpoint utama
- `POST /auth/register`
- `POST /auth/login`
- `POST /sync/push` (auth)
- `GET /sync?since=RFC3339` (auth)

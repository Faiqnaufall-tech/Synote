package handlers

import (
    "context"
    "encoding/json"
    "net/http"
    "time"

    "github.com/google/uuid"

    "project-aplikasi-desktop/backend/internal/db"
)

type SyncHandler struct {
    DB *db.DB
}

type NotePayload struct {
    ID        string  `json:"id"`
    Title     string  `json:"title"`
    Body      string  `json:"body"`
    ProjectID *string `json:"project_id"`
    CreatedAt string  `json:"created_at"`
    UpdatedAt string  `json:"updated_at"`
    DeletedAt *string `json:"deleted_at"`
    Version   int     `json:"version"`
}

type ProjectPayload struct {
    ID        string  `json:"id"`
    Name      string  `json:"name"`
    CreatedAt string  `json:"created_at"`
    UpdatedAt string  `json:"updated_at"`
    DeletedAt *string `json:"deleted_at"`
    Version   int     `json:"version"`
}

type TagPayload struct {
    ID   string `json:"id"`
    Name string `json:"name"`
}

type NoteTagPayload struct {
    NoteID string `json:"note_id"`
    TagID  string `json:"tag_id"`
}

type SummaryPayload struct {
    ID        string  `json:"id"`
    NoteID    *string `json:"note_id"`
    ProjectID *string `json:"project_id"`
    Content   string  `json:"content"`
    CreatedAt string  `json:"created_at"`
}

type SyncPushRequest struct {
    Notes     []NotePayload     `json:"notes"`
    Projects  []ProjectPayload  `json:"projects"`
    Tags      []TagPayload      `json:"tags"`
    NoteTags  []NoteTagPayload  `json:"note_tags"`
    Summaries []SummaryPayload  `json:"summaries"`
    DeviceID  string            `json:"device_id"`
}

type SyncPullResponse struct {
    Notes     []NotePayload     `json:"notes"`
    Projects  []ProjectPayload  `json:"projects"`
    Tags      []TagPayload      `json:"tags"`
    NoteTags  []NoteTagPayload  `json:"note_tags"`
    Summaries []SummaryPayload  `json:"summaries"`
    ServerNow string            `json:"server_now"`
}

func (h *SyncHandler) Push(w http.ResponseWriter, r *http.Request) {
    userID := userIDFromContext(r)
    if userID == "" {
        writeError(w, http.StatusUnauthorized, "user tidak valid")
        return
    }

    var req SyncPushRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    ctx := context.Background()

    tx, err := h.DB.Pool.Begin(ctx)
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal tx")
        return
    }
    defer tx.Rollback(ctx)

    for _, p := range req.Projects {
        if p.ID == "" {
            p.ID = uuid.NewString()
        }
        _, err = tx.Exec(ctx,
            `INSERT INTO projects (id, user_id, name, created_at, updated_at, deleted_at, version)
             VALUES ($1, $2, $3, $4, $5, $6, $7)
             ON CONFLICT (id) DO UPDATE SET
               name = EXCLUDED.name,
               created_at = EXCLUDED.created_at,
               updated_at = EXCLUDED.updated_at,
               deleted_at = EXCLUDED.deleted_at,
               version = EXCLUDED.version
             WHERE EXCLUDED.updated_at > projects.updated_at`,
            p.ID, userID, p.Name, p.CreatedAt, p.UpdatedAt, p.DeletedAt, p.Version,
        )
        if err != nil {
            writeError(w, http.StatusInternalServerError, "gagal sync project")
            return
        }
    }

    for _, n := range req.Notes {
        if n.ID == "" {
            n.ID = uuid.NewString()
        }
        _, err = tx.Exec(ctx,
            `INSERT INTO notes (id, user_id, title, body, project_id, created_at, updated_at, deleted_at, version)
             VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
             ON CONFLICT (id) DO UPDATE SET
               title = EXCLUDED.title,
               body = EXCLUDED.body,
               project_id = EXCLUDED.project_id,
               created_at = EXCLUDED.created_at,
               updated_at = EXCLUDED.updated_at,
               deleted_at = EXCLUDED.deleted_at,
               version = EXCLUDED.version
             WHERE EXCLUDED.updated_at > notes.updated_at`,
            n.ID, userID, n.Title, n.Body, n.ProjectID, n.CreatedAt, n.UpdatedAt, n.DeletedAt, n.Version,
        )
        if err != nil {
            writeError(w, http.StatusInternalServerError, "gagal sync catatan")
            return
        }
    }

    for _, t := range req.Tags {
        if t.ID == "" {
            t.ID = uuid.NewString()
        }
        _, err = tx.Exec(ctx,
            `INSERT INTO tags (id, user_id, name)
             VALUES ($1, $2, $3)
             ON CONFLICT (id) DO UPDATE SET name = EXCLUDED.name`,
            t.ID, userID, t.Name,
        )
        if err != nil {
            writeError(w, http.StatusInternalServerError, "gagal sync tag")
            return
        }
    }

    for _, nt := range req.NoteTags {
        _, err = tx.Exec(ctx,
            `INSERT INTO note_tags (note_id, tag_id)
             VALUES ($1, $2)
             ON CONFLICT DO NOTHING`,
            nt.NoteID, nt.TagID,
        )
        if err != nil {
            writeError(w, http.StatusInternalServerError, "gagal sync note_tags")
            return
        }
    }

    for _, s := range req.Summaries {
        if s.ID == "" {
            s.ID = uuid.NewString()
        }
        _, err = tx.Exec(ctx,
            `INSERT INTO summaries (id, user_id, note_id, project_id, content, created_at)
             VALUES ($1, $2, $3, $4, $5, $6)
             ON CONFLICT (id) DO UPDATE SET
               content = EXCLUDED.content`,
            s.ID, userID, s.NoteID, s.ProjectID, s.Content, s.CreatedAt,
        )
        if err != nil {
            writeError(w, http.StatusInternalServerError, "gagal sync ringkasan")
            return
        }
    }

    _, err = tx.Exec(ctx,
        `INSERT INTO sync_state (user_id, last_sync_at, device_id)
         VALUES ($1, NOW(), $2)
         ON CONFLICT (user_id) DO UPDATE SET last_sync_at = NOW(), device_id = EXCLUDED.device_id`,
        userID, req.DeviceID,
    )
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal sync state")
        return
    }

    if err := tx.Commit(ctx); err != nil {
        writeError(w, http.StatusInternalServerError, "gagal commit")
        return
    }

    writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

func (h *SyncHandler) Pull(w http.ResponseWriter, r *http.Request) {
    userID := userIDFromContext(r)
    if userID == "" {
        writeError(w, http.StatusUnauthorized, "user tidak valid")
        return
    }

    sinceParam := r.URL.Query().Get("since")
    since := time.Time{}
    if sinceParam != "" {
        t, err := time.Parse(time.RFC3339, sinceParam)
        if err == nil {
            since = t
        }
    }

    ctx := context.Background()

    resp := SyncPullResponse{ServerNow: time.Now().UTC().Format(time.RFC3339)}

    rows, err := h.DB.Pool.Query(ctx,
        `SELECT id, name, created_at, updated_at, deleted_at, version
         FROM projects
         WHERE user_id = $1 AND updated_at > $2`,
        userID, since,
    )
    if err == nil {
        defer rows.Close()
        for rows.Next() {
            var p ProjectPayload
            if err := rows.Scan(&p.ID, &p.Name, &p.CreatedAt, &p.UpdatedAt, &p.DeletedAt, &p.Version); err == nil {
                resp.Projects = append(resp.Projects, p)
            }
        }
    }

    rows, err = h.DB.Pool.Query(ctx,
        `SELECT id, title, body, project_id, created_at, updated_at, deleted_at, version
         FROM notes
         WHERE user_id = $1 AND updated_at > $2`,
        userID, since,
    )
    if err == nil {
        defer rows.Close()
        for rows.Next() {
            var n NotePayload
            if err := rows.Scan(&n.ID, &n.Title, &n.Body, &n.ProjectID, &n.CreatedAt, &n.UpdatedAt, &n.DeletedAt, &n.Version); err == nil {
                resp.Notes = append(resp.Notes, n)
            }
        }
    }

    rows, err = h.DB.Pool.Query(ctx,
        `SELECT id, name FROM tags WHERE user_id = $1`,
        userID,
    )
    if err == nil {
        defer rows.Close()
        for rows.Next() {
            var t TagPayload
            if err := rows.Scan(&t.ID, &t.Name); err == nil {
                resp.Tags = append(resp.Tags, t)
            }
        }
    }

    rows, err = h.DB.Pool.Query(ctx,
        `SELECT nt.note_id, nt.tag_id
         FROM note_tags nt
         JOIN notes n ON n.id = nt.note_id
         WHERE n.user_id = $1`,
        userID,
    )
    if err == nil {
        defer rows.Close()
        for rows.Next() {
            var nt NoteTagPayload
            if err := rows.Scan(&nt.NoteID, &nt.TagID); err == nil {
                resp.NoteTags = append(resp.NoteTags, nt)
            }
        }
    }

    rows, err = h.DB.Pool.Query(ctx,
        `SELECT id, note_id, project_id, content, created_at
         FROM summaries
         WHERE user_id = $1 AND created_at > $2`,
        userID, since,
    )
    if err == nil {
        defer rows.Close()
        for rows.Next() {
            var s SummaryPayload
            if err := rows.Scan(&s.ID, &s.NoteID, &s.ProjectID, &s.Content, &s.CreatedAt); err == nil {
                resp.Summaries = append(resp.Summaries, s)
            }
        }
    }

    writeJSON(w, http.StatusOK, resp)
}

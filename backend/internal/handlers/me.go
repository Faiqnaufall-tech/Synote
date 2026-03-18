package handlers

import (
    "context"
    "net/http"

    "project-aplikasi-desktop/backend/internal/db"
)

type MeHandler struct {
    DB *db.DB
}

type meResponse struct {
    UserID string `json:"user_id"`
    Email  string `json:"email"`
}

func (h *MeHandler) Me(w http.ResponseWriter, r *http.Request) {
    userID := userIDFromContext(r)
    if userID == "" {
        writeError(w, http.StatusUnauthorized, "user tidak valid")
        return
    }

    var email string
    err := h.DB.Pool.QueryRow(context.Background(),
        "SELECT email FROM users WHERE id = $1", userID,
    ).Scan(&email)
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal mengambil user")
        return
    }

    writeJSON(w, http.StatusOK, meResponse{UserID: userID, Email: email})
}

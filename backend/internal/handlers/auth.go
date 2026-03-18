package handlers

import (
    "context"
    "encoding/json"
    "net/http"

    "github.com/google/uuid"
    "github.com/jackc/pgx/v5"

    "project-aplikasi-desktop/backend/internal/auth"
    "project-aplikasi-desktop/backend/internal/db"
)

type AuthHandler struct {
    DB *db.DB
}

type authRequest struct {
    Email    string `json:"email"`
    Password string `json:"password"`
}

type authResponse struct {
    Token string `json:"token"`
}

func (h *AuthHandler) Register(w http.ResponseWriter, r *http.Request) {
    var req authRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    hash, err := auth.HashPassword(req.Password)
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal hash")
        return
    }

    userID := uuid.NewString()
    _, err = h.DB.Pool.Exec(context.Background(),
        "INSERT INTO users (id, email, password_hash) VALUES ($1, $2, $3)",
        userID, req.Email, hash,
    )
    if err != nil {
        writeError(w, http.StatusBadRequest, "email sudah terdaftar")
        return
    }

    token, err := auth.SignToken(userID)
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal token")
        return
    }

    writeJSON(w, http.StatusOK, authResponse{Token: token})
}

func (h *AuthHandler) Login(w http.ResponseWriter, r *http.Request) {
    var req authRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    var userID string
    var passwordHash string
    err := h.DB.Pool.QueryRow(context.Background(),
        "SELECT id, password_hash FROM users WHERE email = $1",
        req.Email,
    ).Scan(&userID, &passwordHash)
    if err == pgx.ErrNoRows {
        writeError(w, http.StatusUnauthorized, "email atau password salah")
        return
    }
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal login")
        return
    }

    if err := auth.CheckPassword(passwordHash, req.Password); err != nil {
        writeError(w, http.StatusUnauthorized, "email atau password salah")
        return
    }

    token, err := auth.SignToken(userID)
    if err != nil {
        writeError(w, http.StatusInternalServerError, "gagal token")
        return
    }

    writeJSON(w, http.StatusOK, authResponse{Token: token})
}

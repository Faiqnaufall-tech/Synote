package handlers

import (
    "context"
    "net/http"
    "strings"

    "project-aplikasi-desktop/backend/internal/auth"
)

type contextKey string

const userIDKey contextKey = "user_id"

func WithAuth(next http.HandlerFunc) http.HandlerFunc {
    return func(w http.ResponseWriter, r *http.Request) {
        header := r.Header.Get("Authorization")
        if header == "" || !strings.HasPrefix(header, "Bearer ") {
            writeError(w, http.StatusUnauthorized, "token tidak ada")
            return
        }
        token := strings.TrimPrefix(header, "Bearer ")
        claims, err := auth.ParseToken(token)
        if err != nil {
            writeError(w, http.StatusUnauthorized, "token tidak valid")
            return
        }

        ctx := context.WithValue(r.Context(), userIDKey, claims.UserID)
        next(w, r.WithContext(ctx))
    }
}

func userIDFromContext(r *http.Request) string {
    v := r.Context().Value(userIDKey)
    if v == nil {
        return ""
    }
    if s, ok := v.(string); ok {
        return s
    }
    return ""
}

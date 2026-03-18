package main

import (
    "context"
    "log"
    "net/http"
    "os"

    "project-aplikasi-desktop/backend/internal/db"
    "project-aplikasi-desktop/backend/internal/handlers"
)

func main() {
    if os.Getenv("DATABASE_URL") == "" {
        log.Fatal("DATABASE_URL belum di-set")
    }
    if os.Getenv("JWT_SECRET") == "" {
        log.Fatal("JWT_SECRET belum di-set")
    }

    database, err := db.Connect(context.Background())
    if err != nil {
        log.Fatalf("gagal konek DB: %v", err)
    }
    defer database.Pool.Close()

    authHandler := &handlers.AuthHandler{DB: database}
    meHandler := &handlers.MeHandler{DB: database}
    syncHandler := &handlers.SyncHandler{DB: database}
    aiHandler := &handlers.AIHandler{}

    mux := http.NewServeMux()
    mux.HandleFunc("/auth/register", authHandler.Register)
    mux.HandleFunc("/auth/login", authHandler.Login)
    mux.HandleFunc("/auth/me", handlers.WithAuth(meHandler.Me))
    mux.HandleFunc("/sync/push", handlers.WithAuth(syncHandler.Push))
    mux.HandleFunc("/sync", handlers.WithAuth(syncHandler.Pull))
    mux.HandleFunc("/ai/summarize", handlers.WithAuth(aiHandler.Summarize))
    mux.HandleFunc("/ai/draft", handlers.WithAuth(aiHandler.Draft))
    mux.HandleFunc("/ai/todo", handlers.WithAuth(aiHandler.Todo))

    port := os.Getenv("PORT")
    if port == "" {
        port = "8080"
    }
    addr := ":" + port
    log.Printf("Server running on %s", addr)
    if err := http.ListenAndServe(addr, mux); err != nil {
        log.Fatal(err)
    }
}

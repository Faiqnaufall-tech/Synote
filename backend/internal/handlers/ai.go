package handlers

import (
    "bytes"
    "encoding/json"
    "net/http"
    "os"
)

type AIHandler struct{}

type aiRequest struct {
    Title   string `json:"title"`
    Body    string `json:"body"`
    Hints   string `json:"hints"`
    Context string `json:"context"`
}

type aiResponse struct {
    Text string `json:"text"`
}

type openAIRequest struct {
    Model        string `json:"model"`
    Instructions string `json:"instructions,omitempty"`
    Input        string `json:"input"`
}

type openAIResponse struct {
    Output []struct {
        Type    string `json:"type"`
        Content []struct {
            Type string `json:"type"`
            Text string `json:"text"`
        } `json:"content"`
    } `json:"output"`
}

func (h *AIHandler) Summarize(w http.ResponseWriter, r *http.Request) {
    var req aiRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    system := "Kamu asisten belajar. Jawab singkat, jelas, dan terstruktur."
    user := "Ringkas catatan berikut dalam 5-10 poin penting.\n\n"
    user += "Judul: " + req.Title + "\n"
    user += "Isi:\n" + req.Body + "\n\n"
    if req.Context != "" {
        user += "Konteks catatan lain (ringkas):\n" + req.Context + "\n"
    }

    text, err := callOpenAI(system, user)
    if err != nil {
        writeError(w, http.StatusBadGateway, err.Error())
        return
    }

    writeJSON(w, http.StatusOK, aiResponse{Text: text})
}

func (h *AIHandler) Draft(w http.ResponseWriter, r *http.Request) {
    var req aiRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    system := "Kamu asisten belajar. Tulis draf catatan yang rapi dan mudah dipelajari."
    user := "Buat draf catatan berdasarkan judul dan petunjuk berikut.\n\n"
    user += "Judul: " + req.Title + "\n"
    if req.Hints != "" {
        user += "Petunjuk: " + req.Hints + "\n"
    }
    if req.Context != "" {
        user += "Konteks catatan lain (ringkas):\n" + req.Context + "\n"
    }

    text, err := callOpenAI(system, user)
    if err != nil {
        writeError(w, http.StatusBadGateway, err.Error())
        return
    }

    writeJSON(w, http.StatusOK, aiResponse{Text: text})
}

func (h *AIHandler) Todo(w http.ResponseWriter, r *http.Request) {
    var req aiRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        writeError(w, http.StatusBadRequest, "payload tidak valid")
        return
    }

    system := "Kamu asisten belajar. Ubah catatan menjadi daftar to-do yang bisa dieksekusi."
    user := "Ubah catatan berikut menjadi daftar to-do dengan bullet.\n\n"
    user += "Judul: " + req.Title + "\n"
    user += "Isi:\n" + req.Body + "\n"

    text, err := callOpenAI(system, user)
    if err != nil {
        writeError(w, http.StatusBadGateway, err.Error())
        return
    }

    writeJSON(w, http.StatusOK, aiResponse{Text: text})
}

func callOpenAI(systemPrompt, userPrompt string) (string, error) {
    apiKey := os.Getenv("OPENAI_API_KEY")
    if apiKey == "" {
        return "", errString("OPENAI_API_KEY belum di-set")
    }
    model := os.Getenv("OPENAI_MODEL")
    if model == "" {
        return "", errString("OPENAI_MODEL belum di-set")
    }

    payload := openAIRequest{
        Model:        model,
        Instructions: systemPrompt,
        Input:        userPrompt,
    }
    body, _ := json.Marshal(payload)

    req, _ := http.NewRequest("POST", "https://api.openai.com/v1/responses", bytes.NewReader(body))
    req.Header.Set("Content-Type", "application/json")
    req.Header.Set("Authorization", "Bearer "+apiKey)

    client := &http.Client{}
    resp, err := client.Do(req)
    if err != nil {
        return "", err
    }
    defer resp.Body.Close()

    if resp.StatusCode < 200 || resp.StatusCode >= 300 {
        return "", errString("gagal memanggil OpenAI")
    }

    var out openAIResponse
    if err := json.NewDecoder(resp.Body).Decode(&out); err != nil {
        return "", err
    }

    var text bytes.Buffer
    for _, item := range out.Output {
        if item.Type != "message" {
            continue
        }
        for _, c := range item.Content {
            if c.Type == "output_text" {
                text.WriteString(c.Text)
            }
        }
    }

    if text.Len() == 0 {
        return "", errString("response OpenAI kosong")
    }

    return text.String(), nil
}

type errString string

func (e errString) Error() string { return string(e) }

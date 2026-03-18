# Build stage
FROM golang:1.22-alpine AS build
WORKDIR /app

# Copy module files from backend folder
COPY backend/go.mod backend/go.sum ./
RUN if [ -s go.sum ]; then go mod download; else go mod tidy; fi

# Copy the backend source
COPY backend/ ./
RUN go build -o app ./cmd/server

# Run stage
FROM alpine:3.20
WORKDIR /app
RUN adduser -D appuser

COPY --from=build /app/app ./app

USER appuser
ENV PORT=8080
EXPOSE 8080

CMD ["./app"]

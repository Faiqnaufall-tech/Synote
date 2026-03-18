# Build stage
FROM golang:1.22-alpine AS build
WORKDIR /app

# Copy module files from backend folder
COPY backend/go.mod backend/go.sum ./

# Copy the backend source
COPY backend/ ./

# Ensure go.sum is generated based on source imports
RUN go mod tidy
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

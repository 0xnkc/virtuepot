FROM clux/muslrust:latest AS builder

WORKDIR /rust/src/app
COPY . .

RUN rustup target add x86_64-unknown-linux-musl
RUN cargo build --release --target x86_64-unknown-linux-musl
RUN strip ./target/x86_64-unknown-linux-musl/release/watertank_simulation_server

FROM alpine
WORKDIR /app

COPY --from=builder /rust/src/app/target/x86_64-unknown-linux-musl/release/watertank_simulation_server /app/

CMD ["./watertank_simulation_server"]
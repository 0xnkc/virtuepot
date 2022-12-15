# Stage 1 - Install build dependencies
FROM python:3.8-alpine AS builder

WORKDIR /app

ARG BUILD_DEPS="build-base gcc libffi-dev openssl-dev"

RUN apk add --no-cache ${BUILD_DEPS} \
 && python -m venv .venv \
 && .venv/bin/pip install --no-cache-dir -U pip setuptools

COPY requirements.txt .

RUN .venv/bin/pip install --no-cache-dir -r requirements.txt \
 && find /app/.venv \
        \( -type d -a -name test -o -name tests \) \
        -o \( -type f -a -name '*.pyc' -o -name '*.pyo' \) \
        -exec rm -rf '{}' \+

# Stage 2 - Copy only necessary files to the runner stage
FROM python:3.8-alpine

WORKDIR /app

ARG RUNTIME_DEPS="libssl1.1"
RUN apk add --no-cache ${RUNTIME_DEPS}

COPY --from=builder /app /app
#COPY my_project my_project
COPY src/asyncio-server.py .

ENV PATH="/app/.venv/bin:$PATH"

CMD ["python", "asyncio-server.py"]

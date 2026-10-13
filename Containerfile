FROM python:3.12-slim

ARG PLATFORMIO_VERSION=6.1.19

RUN apt-get update \
    && apt-get install --yes --no-install-recommends git \
    && pip install --no-cache-dir "platformio==${PLATFORMIO_VERSION}" \
    && rm -rf /var/lib/apt/lists/* \
    && git config --system --add safe.directory /workspace

WORKDIR /workspace
ENTRYPOINT ["pio"]
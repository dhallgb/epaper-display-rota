FROM debian:bookworm
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN mkdir /app
COPY scrape-site.py pyproject.toml /app
WORKDIR /app

RUN uv sync
RUN uv run playwright install --with-deps chromium

EXPOSE 5050

CMD ["uv", "run", "/app/scrape-site.py"]
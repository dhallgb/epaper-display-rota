FROM debian:bookworm
ENV PATH=/venv/bin:$PATH
ENV PATH=/root/.cargo/bin:$PATH

RUN apt update -y
RUN apt install curl clang -y
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

RUN mkdir /app
COPY scrape-site.py pyproject.toml /app
WORKDIR /app

RUN uv sync
RUN uv run playwright install --with-deps chromium

EXPOSE 5050

CMD ["uv", "run", "/app/scrape-site.py"]
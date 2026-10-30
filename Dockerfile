FROM gcc:14

WORKDIR /app

COPY main.c .

COPY capture.bin .

RUN gcc -o main main.c

CMD ["./main"]
FROM gcc:14

WORKDIR /app

COPY main.c .

RUN gcc -Wall -Wextra -O2 -o main main.c

CMD ["./main"]
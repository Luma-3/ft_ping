
FROM debian:bullseye
RUN apt-get update && apt-get install -y iproute2 iputils-ping tcpdump&& rm -rf /var/lib/apt/lists/*
COPY ft_ping /usr/local/bin/ft_ping
RUN chmod +x /usr/local/bin/ft_ping
CMD ["sleep", "infinity"]


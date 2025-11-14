
FROM debian:bullseye
RUN apt-get update && apt-get install -y iproute2 inetutils-ping tcpdump && rm -rf /var/lib/apt/lists/*
COPY ft_ping /usr/local/bin/ft_ping
COPY apply_netem.sh /usr/local/bin/apply_netem.sh
RUN chmod +x /usr/local/bin/ft_ping
RUN chmod +x /usr/local/bin/apply_netem.sh
CMD ["sleep", "infinity"]


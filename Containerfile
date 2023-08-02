FROM debian:bullseye
RUN apt-get update && \
	apt-get install --no-install-recommends \
		build-essential \
		cmake \
		corosync \
		git \
		pkg-config \
		debhelper \
		libcorosync-common-dev \
		libcpg-dev \
		libquorum-dev \
		libsystemd-dev \
		-y && \
	apt-get clean
COPY . /app
WORKDIR /app/build
RUN cmake .. && \
	make package

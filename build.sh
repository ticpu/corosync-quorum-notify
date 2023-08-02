#!/bin/sh
BUILD=bullseye-build
CONTAINER=bullseye-container
podman build -t $BUILD .
podman rm -f $CONTAINER 2>/dev/null
DEB=$(podman run --name $CONTAINER $BUILD sh -c "ls /app/build/*.deb")
podman cp $CONTAINER:"${DEB}" .
podman rm $CONTAINER
podman image untag $BUILD

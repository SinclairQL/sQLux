#Building for Release Using Docker

## Prepare buildx

Buildx is needed so that the container can be built in an insecure env otherwise the chroots will not get
created correctly. So first create a buildx builder with the correct flags.

```
docker buildx create --use --name insecure-builder --buildkitd-flags '--allow-insecure-entitlement security.insecure'
```

# Build the sqlux-build container

This will build the container and load it into the local docker container stores so it can be used
later. If you want to push to a registry you can also use --push

```
docker buildx build --allow security.insecure --load -t sqlux-build .
```

# Build the sqlux release

Replace sqlux-dir and sqlux-release in the command with the the checked out sqlux directoy and a
directory where the build artifacts will be place.
 
```
docker run -v <sqlux-dir>:/build/sqlux -v <sqlux-release>:/build/release -it sqlux-build
```

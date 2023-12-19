# Github workflows - older actions

This folder exists to document/backup older versions of actions.

Those were upgraded or reimplemented, but we didn't want to get rid of them.

Actual run is disabled on all branches, unless someone creates and pushes to a branch named `other`.

## Rework log

### `build-arm.old.yml`

- abandonware
- uses a simple action `actions/upload-artifact@v2` that only stores data for a short period of time, or makes it available to other jobs.

### `build-arm.other.yml`

- Tried to make it work w/ `runs-on: ubuntu-22.04`, which runs into a container on the runner.
- We faced problems when cross-compiling for multiple architectures.
- We abandoned all efforts after many failures, last problem was actually installing cross-libraries.

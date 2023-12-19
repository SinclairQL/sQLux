# Github workflows

## Workflow job descriptions

### List of jobs:
- Prepare build definitions
- build_x86_64
- build_arm
- build_macos
- build_windows

### Description of jobs:

1. `build_x86_64`

    - Builds on Ubuntu 22.04 "Jammy" (x86_64)
    - Tested on an Intel desktop w/ Ubuntu 22.04.03 LTS

2. `build_arm`

    Builds against `arm` architectures, by way of cross-compiling on:

    - aarch64
      - Ubuntu 22.04 "Jammy"
    - armv7
      - Ubuntu 22.04 "Jammy"
      - Debian 11 "Bullseye"
      - Debian 10 "Buster"


3. `build_macos`

    It's not fully documented which are the available runners archs...

    https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners/about-github-hosted-runners#supported-runners-and-hardware-resources

    - runs on `macos-12`, which is, in today's commit date, an intel machine, as it can be seen in the runner logs:

    ```
    machdep.cpu.brand_string: Intel(R) Core(TM) i7-8700B CPU @ 3.20GHz
    ```

4. `build_windows`

    Builds on Windows x86_64 image, with MSYS2 dev tools

    Crossbuilds:
    - mingw64 : x86_64 libraries
    - mingw32 : i686 libraries



## Workflow local tests with `act`

https://github.com/nektos/act

Local testing saves runner times on Github.com, though builds are working only for some architecture...

Anyhow `act` can be useful to test parts of the workflow, saving commit/push/wait-for-runners time.

The workflow is configurable with variables, so the workflow developer can concentrate on particular features.

### 1 - configure `act` using local github env

```sh
cat > ~/sQLux.events.json <<EOF
{
  "inputs": {
    "job_enabled_x86_64":    "false",
    "job_enabled_arm":       "false",
    "job_enabled_macos":     "false",
    "job_enabled_windows":   "false",
    "build_enabled_x86_64":  "false",
    "build_enabled_arm":     "false",
    "build_enabled_macos":   "false",
    "build_enabled_windows": "false"
  }
}
EOF
```

Description of configurations:
- `job_*  ` : disables whole jobs
- `build_*` : disables only the `cmake` steps.

### 2 - run `act`

  ```sh
  mkdir /tmp/artifacts
  act -v -W .github/workflows/build.yml --artifact-server-path /tmp/artifacts -e ~/sQLux.events.json
  ```

Note: `--artifact-server-path` mocks a github server w/ a local filesystem

### Supported builds w/ `act`:
Known to work builds are:
- x86_64

### Unsupported builds w/ `act`:
- Linux on ARM
    We use `uraimo/run-on-arch-action@v2.6.0` action,
    which spawns containers by itself under the hood, thus resulting in DinD containers, which is unsupported.  
    This action step works only on Github.com runners.  
    Therefore, to use `act`, disable the 'build' with the related `inputs.build_enabled_arm` in `~/sQLux.events.json`  
    https://github.com/uraimo/run-on-arch-action/issues/42
- MacOS
  - `act` will skip this job
- Windows
  - `act` will skip this job


## Build tips

### Debian 10 "Buster"

Tested on RaspberryPI 400

```txt
pi@raspberry400:~ $ lsb_release -a
No LSB modules are available.
Distributor ID:  Raspbian
Description   :  Raspbian GNU/Linux 10 (buster)
Release       :  10
Codename      :  buster

pi@raspberry400:~ $ grep -Ei "model|hardware" /proc/cpuinfo |uniq
model name    : ARMv7 Processor rev 3 (v7l)
Hardware      : BCM2711
Model         : Raspberry Pi 400 Rev 1.0

pi@raspberry400:~ $ apt list libc6
Listing... Done
libc6/oldoldstable 2.28-10+rpt2+rpi1+deb10u2 armhf [upgradable from: 2.28-10+rpi1]
```

## CHANGELOG.md tips

Some tool exist to manage `keepachangelog` markdown formatted notes.

The workflow uses a python module to extract the content to be attached to the released artifacts,
but other modules exists to lint, create, update the content itself (npm `keep-a-changelog`, other python modules)

Install `kacl-cli`

```sh
pip install kacl-cli
```

Extract only a level of log:
```sh
kacl-cli get unreleased
kacl-cli get v1.0.6
```

Check for eventual errors or linting needs:
```sh
kacl-cli verify
```

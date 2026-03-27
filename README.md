# bonicpp

## Build

Make sure vcpkg environment is set up,
and then configure CMake with it.

```sh
vcpkg install
cmake -B build
```

After that, compile with

```sh
cmake --build build
```

To build tests, use the `dev` preset when configuring.

```sh
cmake -B build --preset dev
cmake --build build
ctest --test-dir build
```

### Setting up vcpkg

vcpkg can be obtained with Git.

```bash
export VCPKG_ROOT="${HOME}/.local/opt/vcpkg"
mkdir -p "${VCPKG_ROOT}"

git clone 'https://github.com/Microsoft/vcpkg.git' "${VCPKG_ROOT}"
"${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics
```

Back in the project directory,

```bash
export CMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
export VCPKG_DISABLE_METRICS=1
vcpkg install
cmake -B build
```

@echo off
bazel build windows_binary
start bazel-bin/windows_binary.exe
"bazel-bin/windows_binary.exe"
@echo off
bazel build windows_binary
"bazel-bin/windows_binary.exe"
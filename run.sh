bazel build linux_binary
alacritty -e ./bazel-bin/linux_binary server &
./bazel-bin/linux_binary

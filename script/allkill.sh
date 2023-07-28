#!/bin/bash

# topsolver プロセスを取得して、PIDを取得する
topsolver_pids=$(pgrep -f "topsolver")

# topsolver プロセスが存在するかチェック
if [ -z "$topsolver_pids" ]; then
  echo "topsolver プロセスは実行中ではありません。"
else
  # topsolver プロセスを kill する
  echo "以下の PID の topsolver プロセスを kill します："
  echo "$topsolver_pids"
  kill $topsolver_pids
  echo "topsolver プロセスを kill しました。"
fi


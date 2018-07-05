#!/bin/bash

# Copyright (C) 2018 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Kills a list of processes by name when SIGINT is received.

declare -a procs=(
  "gzserver"
  # Add other processes here separated with spaces
)

on_sigterm() {
  for p in "${procs[@]}"
  do
    echo "[Terminator] Terminating [$p], no questions"
    kill -9 `pidof $p`
  done
  exit 0
}

trap on_sigterm TERM
sleep infinity

#!/bin/bash
set -e

relevant_instances() {

  DIRECTORY="$1"
  if [ -z "$DIRECTORY" ]; then
    DIRECTORY=test/benchmark/
  fi

  MAX_SIZE="$2"
  if [ -z "$MAX_SIZE" ]; then
    MAX_SIZE=100
  fi

  MAX_DEPTH="$3"
  if [ -z "$MAX_DEPTH" ]; then
    MAX_DEPTH=100
  fi

  find "$DIRECTORY" -maxdepth "$MAX_DEPTH" -size "-${MAX_SIZE}k" -name '*.cnf' | sed 's|\.cnf$||'
}

relevant_directories() {
  relevant_instances "$@" | xargs dirname | sort -u
}

max_ten_per_dir() {
  DIRECTORY="$1"
  if [ -z "$DIRECTORY" ]; then
    DIRECTORY=test/benchmark/
  fi

  MAX_SIZE="$2"
  if [ -z "$MAX_SIZE" ]; then
    MAX_SIZE=100
  fi

  INSTANCES="$3"
  if [ -z "$INSTANCES" ]; then
    INSTANCES=100
  fi

  relevant_directories "$DIRECTORY" "$MAX_SIZE" | while read SUB_DIR; do
    relevant_instances "$SUB_DIR" "$MAX_SIZE" 1 | head "-$INSTANCES"
  done
}

schedule_execution() {
  SOURCES="X:\\sharpSAT\\coverage"
  SHARPSAT="$SOURCES\\windows\\Debug\\sharpSAT.exe"

  FILE="$1"
  TIMEOUT="$2"
  VERBOSITY="$3"
  PREPROCESS="$4"
  COMP_CACHING="$5"

  WINFILE=$(echo "$FILE" | sed 's|/|\\|g')

  ARGS="-t $TIMEOUT"

  if [ "$VERBOSITY" -eq 0 ]; then
    ARGS="$ARGS -q"
  fi
  if [ "$VERBOSITY" -eq 2 ]; then
    ARGS="$ARGS -v"
  fi

  if [ "$PREPROCESS" -eq 0 ]; then
    ARGS="$ARGS -noPP"
  fi

  if [ "$COMP_CACHING" -eq 0 ]; then
    ARGS="$ARGS -coCC"
  fi

  DIR="${VERBOSITY}_${PREPROCESS}_${COMP_CACHING}_${TIMEOUT}"
  mkdir -p "cov/$DIR"

  COV=""

  # Schedule
  if [ ! -e "cov/$DIR/$FILE.cov" ]; then
    echo opencppcoverage --quiet --export_type "binary:cov\\$DIR\\$WINFILE.cov" --sources "$SOURCES" --module "$SHARPSAT" -- "$SHARPSAT" "$ARGS" "$WINFILE.cnf"
  fi

  # Prepare for merge
  echo input_coverage="cov\\$DIR\\$WINFILE.cov" >> cov/merge_config.txt
}

mkdir -p cov
rm -f cov/merge_config.txt

{

  # no component caching
  for FILE in $(max_ten_per_dir '' 2 2); do
    schedule_execution $FILE 6 1 1 0
  done

  # no preprocessing
  for FILE in $(max_ten_per_dir '' 6 1); do
    schedule_execution $FILE 6 1 0 1
  done

  # no preprocessing, no component caching
  for FILE in $(max_ten_per_dir '' 3 2); do
    schedule_execution $FILE 6 1 0 0
  done

  # legacy, not needed any more, but already generated
  for FILE in $(max_ten_per_dir '' 64 10 | head -26); do
    schedule_execution $FILE 60 1 0 0
  done

  # small runtime, many instances
  for FILE in $(max_ten_per_dir '' 64 10 | head -60); do
    schedule_execution $FILE 60 1 1 1
  done

  # medium runtime, less instances
  for FILE in $(max_ten_per_dir '' 128 2 | head -5); do
    schedule_execution $FILE 300 1 1 1
  done

  # big runtime, 3 huge instances
  for FILE in test/benchmark/ijcai07/langford/lang20.cnf test/benchmark/pmc/Planning/4blocks.cnf test/benchmark/pmc/bmc/bmc-ibm-6.cnf; do
    schedule_execution $FILE 3600 1 1 1
  done

echo export_type=html:cov\\report >> cov/merge_config.txt
echo "opencppcoverage --config cov\\merge_config.txt"
} | unix2dos > cov/execute.bat

echo -n "Cases generated: "
cat cov/execute.bat | wc -l

echo Now please run \"cov\\execute.bat\" from Windows Command Prompt \(cmd.exe\)

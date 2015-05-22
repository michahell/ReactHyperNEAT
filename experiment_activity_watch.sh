#!/usr/bin/env bash

# colored red text (error)
function errorMsg () {
  printf '\033[0;31m%s\033[0m\n' "$*" >&2
}

# check for folder to monitor argument
if [[ $# -ne 1 ]]
then
  errorMsg "add experiment (folder) to monitor for activity!"
  exit 2
fi

# time-out: 3 minutes
TIMEOUT_TIME=180

# Read fswatch output as long as it is
# within the timeout value; every change reported
# resets the timer.
while IFS= read -t $TIMEOUT_TIME -d '' -r file; do 
  echo "changed: [$file]"
done < <(fswatch -r -0 "${1}")

# Getting here means that read timed out.
echo "file write activity halted!" | mail -s "trouble!" "maggelo@gmail.com"
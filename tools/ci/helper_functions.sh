#!/usr/bin/env bash

# Collection of helper functions that can be used in the different continuous
# integration scripts.

# A small utility to run a command and only print logs if the command fails.
# On success, all logs are hidden.
function log_errors {
    if log=$("$@" 2>&1); then
        echo "'$@' completed successfully!"
    else
        echo "$log"
        return 1
    fi    
}

# Get the system timeout command
function get_timeout {
    uname=`uname`
    if [[ "$uname" == 'Linux' ]]; then
        echo "timeout"
    elif [[ "$uname" == 'Darwin' ]]; then
        echo "gtimeout"
    fi
}

function check_tools_version {
    # Get required version fields
    IFS='.' read -ra FIELDS <<< "$@"
    MIN_VERSION_MAJOR=${FIELDS[0]}
    MIN_VERSION_MINOR=${FIELDS[1]}
    MIN_VERSION_PATCH=${FIELDS[2]}
    # Run xcc --version 
    xcc_version_output_string=`xcc --version`
    if [[ "$xcc_version_output_string" == *"XTC version:"* ]]; then
        # Find the semantic version substring
        prefix=${xcc_version_output_string%%"XTC version:"*}
        zero_index=${#prefix}
        start_index=`expr $zero_index + 14`
        prefix=${xcc_version_output_string%%"Copyright"*}
        end_index=${#prefix}
        xcc_semver_substring=`echo $xcc_version_output_string | cut -c$start_index-$end_index`
        # Split semver substring into fields
        IFS='.' read -ra FIELDS <<< "$xcc_semver_substring"
        XTC_VERSION_MAJOR=${FIELDS[0]}
        XTC_VERSION_MINOR=${FIELDS[1]}
        XTC_VERSION_PATCH=${FIELDS[2]}
    else
        # Unable to determine the version. Return 15.1.0 and hope for the best
        # Note, 15.1.0 had a bug where the version was missing
        XTC_VERSION_MAJOR="15"
        XTC_VERSION_MINOR="1"
        XTC_VERSION_PATCH="0"
    fi
    # Check version
    if [ "$XTC_VERSION_MAJOR" -lt "$MIN_VERSION_MAJOR" ]
    then
        return 1
    else
        if [ "$XTC_VERSION_MINOR" -lt "$MIN_VERSION_MINOR" ]
        then
            return 1
        else
            if [ "$XTC_VERSION_PATCH" -lt "$MIN_VERSION_PATCH" ]
            then
                return 1
            fi
        fi
    fi
}

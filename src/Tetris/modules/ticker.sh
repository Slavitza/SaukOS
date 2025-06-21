ticker() {
    trap exit SIGUSR2
    trap 'DELAY=$(awk "BEGIN {print $DELAY * $DELAY_FACTOR}")' SIGUSR1
    while true ; do echo -n $DOWN; sleep $DELAY; done
}


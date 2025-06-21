reader() {
    trap exit SIGUSR2
    trap '' SIGUSR1
    local -u key a='' b='' cmd esc_ch=$'\x1b'
    declare -A commands=([A]=$ROTATE [C]=$RIGHT [D]=$LEFT
        [_S]=$ROTATE [_A]=$LEFT [_D]=$RIGHT
        [_]=$DROP [_Q]=$QUIT [_H]=$TOGGLE_HELP [_N]=$TOGGLE_NEXT [_C]=$TOGGLE_COLOR)
    while read -s -n 1 key ; do
        case "$a$b$key" in
            "${esc_ch}["[ACD]) cmd=${commands[$key]} ;;
            *${esc_ch}${esc_ch}) cmd=$QUIT ;;
            *) cmd=${commands[_$key]:-} ;;
        esac
        a=$b
        b=$key
        [ -n "$cmd" ] && echo -n "$cmd"
    done
}


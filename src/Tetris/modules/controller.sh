cmd_right() {
    move_piece $((current_piece_x + 1)) $current_piece_y
}

cmd_left() {
    move_piece $((current_piece_x - 1)) $current_piece_y
}

cmd_rotate() {
    local available_rotations old_rotation new_rotation
    available_rotations=$((${#piece[$current_piece]} / 8))
    old_rotation=$current_piece_rotation
    new_rotation=$(((old_rotation + 1) % available_rotations))
    current_piece_rotation=$new_rotation
    if new_piece_location_ok $current_piece_x $current_piece_y ; then
        current_piece_rotation=$old_rotation
        clear_current
        current_piece_rotation=$new_rotation
        show_current
    else
        current_piece_rotation=$old_rotation
    fi
}

cmd_down() {
    move_piece $current_piece_x $((current_piece_y + 1))
}

cmd_drop() {
    while move_piece $current_piece_x $((current_piece_y + 1)) ; do : ; done
}

cmd_quit() {
    showtime=false
    pkill -SIGUSR2 -f "/bin/bash $0"
    xyprint $GAMEOVER_X $GAMEOVER_Y "Game over!"
    echo -e "$screen_buffer"
}

init() {
    local i
    for ((i = 0; i < PLAYFIELD_H * PLAYFIELD_W; i++)) {
        play_field[$i]=-1
    }
    clear
    hide_cursor
    get_random_next
    get_random_next
    toggle_color
}

controller() {
    trap '' SIGUSR1 SIGUSR2
    local cmd commands
    commands[$QUIT]=cmd_quit
    commands[$RIGHT]=cmd_right
    commands[$LEFT]=cmd_left
    commands[$ROTATE]=cmd_rotate
    commands[$DOWN]=cmd_down
    commands[$DROP]=cmd_drop
    commands[$TOGGLE_HELP]=toggle_help
    commands[$TOGGLE_NEXT]=toggle_next
    commands[$TOGGLE_COLOR]=toggle_color

    init

    while $showtime; do
        echo -ne "$screen_buffer"
        screen_buffer=""
        read -s -n 1 cmd
        ${commands[$cmd]}
    done
}


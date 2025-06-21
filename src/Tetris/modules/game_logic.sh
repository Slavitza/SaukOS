update_score() {
    ((lines_completed += $1))
    ((score += ($1 * $1)))
    if (( score > LEVEL_UP * level)) ; then
        ((level++))
        pkill -SIGUSR1 -f "/bin/bash $0"
    fi
    set_bold
    set_fg $SCORE_COLOR
    xyprint $SCORE_X $SCORE_Y         "Lines completed: $lines_completed"
    xyprint $SCORE_X $((SCORE_Y + 1)) "Level:           $level"
    xyprint $SCORE_X $((SCORE_Y + 2)) "Score:           $score"
    reset_colors
}

toggle_help() {
    local i s
    set_bold
    set_fg $HELP_COLOR
    for ((i = 0; i < ${#help[@]}; i++ )) {
        ((help_on == 1)) && s="${help[i]}" || s="${help[i]//?/ }"
        xyprint $HELP_X $((HELP_Y + i)) "$s"
    }
    ((help_on = -help_on))
    reset_colors
}

toggle_next() {
    case $next_on in
        1) clear_next; next_on=-1 ;;
        -1) next_on=1; show_next ;;
    esac
}

toggle_color() {
    $no_color && no_color=false || no_color=true
    show_next
    update_score 0
    toggle_help
    toggle_help
    draw_border
    redraw_playfield
    show_current
}

new_piece_location_ok() {
    local j i x y x_test=$1 y_test=$2
    for ((j = 0, i = 1; j < 8; j += 2, i = j + 1)) {
        ((y = ${piece[$current_piece]:$((j + current_piece_rotation * 8)):1} + y_test))
        ((x = ${piece[$current_piece]:$((i + current_piece_rotation * 8)):1} + x_test))
        ((y < 0 || y >= PLAYFIELD_H || x < 0 || x >= PLAYFIELD_W )) && return 1
        ((${play_field[y * PLAYFIELD_W + x]} != -1 )) && return 1
    }
    return 0
}

get_random_next() {
    current_piece=$next_piece
    current_piece_rotation=$next_piece_rotation
    current_piece_color=$next_piece_color
    ((current_piece_x = (PLAYFIELD_W - 4) / 2))
    ((current_piece_y = 0))
    new_piece_location_ok $current_piece_x $current_piece_y || cmd_quit
    show_current
    clear_next
    ((next_piece = RANDOM % ${#piece[@]}))
    ((next_piece_rotation = RANDOM % (${#piece[$next_piece]} / 8)))
    ((next_piece_color = RANDOM % ${#colors[@]}))
    show_next
}

flatten_playfield() {
    local i j k x y
    for ((i = 0, j = 1; i < 8; i += 2, j += 2)) {
        ((y = ${piece[$current_piece]:$((i + current_piece_rotation * 8)):1} + current_piece_y))
        ((x = ${piece[$current_piece]:$((j + current_piece_rotation * 8)):1} + current_piece_x))
        ((k = y * PLAYFIELD_W + x))
        play_field[$k]=$current_piece_color
    }
}

process_complete_lines() {
    local j i complete_lines
    ((complete_lines = 0))
    for ((j = 0; j < PLAYFIELD_W * PLAYFIELD_H; j += PLAYFIELD_W)) {
        for ((i = j + PLAYFIELD_W - 1; i >= j; i--)) {
            ((${play_field[$i]} == -1)) && break
        }
        ((i >= j)) && continue
        ((complete_lines++))
        for ((i = j - 1; i >= 0; i--)) {
            play_field[$((i + PLAYFIELD_W))]=${play_field[$i]}
        }
        for ((i = 0; i < PLAYFIELD_W; i++)) {
            play_field[$i]=-1
        }
    }
    return $complete_lines
}

process_fallen_piece() {
    flatten_playfield
    process_complete_lines && return
    update_score $?
    redraw_playfield
}

move_piece() {
    if new_piece_location_ok $1 $2 ; then
        clear_current
        current_piece_x=$1
        current_piece_y=$2
        show_current
        return 0
    fi
    (($2 == current_piece_y)) && return 0
    process_fallen_piece
    get_random_next
    return 1
}


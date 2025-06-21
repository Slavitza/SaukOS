# playfield es 1D, cada celda contiene color o -1 si está vacía
redraw_playfield() {
    local j i x y xp yp
    ((xp = PLAYFIELD_X))
    for ((y = 0; y < PLAYFIELD_H; y++)) {
        ((yp = y + PLAYFIELD_Y))
        ((i = y * PLAYFIELD_W))
        xyprint $xp $yp ""
        for ((x = 0; x < PLAYFIELD_W; x++)) {
            ((j = i + x))
            if ((${play_field[$j]} == -1)) ; then
                puts "$empty_cell"
            else
                set_fg ${play_field[$j]}
                set_bg ${play_field[$j]}
                puts "$filled_cell"
                reset_colors
            fi
        }
    }
}

draw_piece() {
    local i x y
    for ((i = 0; i < 8; i += 2)) {
        ((x = $1 + ${piece[$3]:$((i + $4 * 8 + 1)):1} * 2))
        ((y = $2 + ${piece[$3]:$((i + $4 * 8)):1}))
        xyprint $x $y "$5"
    }
}

draw_next() {
    ((next_on == -1)) && return
    draw_piece $NEXT_X $NEXT_Y $next_piece $next_piece_rotation "$1"
}

clear_next() {
    draw_next "${filled_cell//?/ }"
}

show_next() {
    set_fg $next_piece_color
    set_bg $next_piece_color
    draw_next "${filled_cell}"
    reset_colors
}

draw_current() {
    draw_piece $((current_piece_x * 2 + PLAYFIELD_X)) $((current_piece_y + PLAYFIELD_Y)) $current_piece $current_piece_rotation "$1"
}

show_current() {
    set_fg $current_piece_color
    set_bg $current_piece_color
    draw_current "${filled_cell}"
    reset_colors
}

clear_current() {
    draw_current "${empty_cell}"
}

draw_border() {
    local i x1 x2 y
    set_bold
    set_fg $BORDER_COLOR
    ((x1 = PLAYFIELD_X - 2))
    ((x2 = PLAYFIELD_X + PLAYFIELD_W * 2))
    for ((i = 0; i < PLAYFIELD_H + 1; i++)) {
        ((y = i + PLAYFIELD_Y))
        xyprint $x1 $y "<|"
        xyprint $x2 $y "|>"
    }
    ((y = PLAYFIELD_Y + PLAYFIELD_H))
    for ((i = 0; i < PLAYFIELD_W; i++)) {
        ((x1 = i * 2 + PLAYFIELD_X))
        xyprint $x1 $y '=='
        xyprint $x1 $((y + 1)) "\/"
    }
    reset_colors
}


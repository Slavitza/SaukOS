puts() {
    screen_buffer+=${1}
}

xyprint() {
    puts "\033[${2};${1}H${3}"
}

show_cursor() {
    echo -ne "\033[?25h"
}

hide_cursor() {
    echo -ne "\033[?25l"
}

set_fg() {
    $no_color && return
    puts "\033[3${1}m"
}

set_bg() {
    $no_color && return
    puts "\033[4${1}m"
}

reset_colors() {
    puts "\033[0m"
}

set_bold() {
    puts "\033[1m"
}


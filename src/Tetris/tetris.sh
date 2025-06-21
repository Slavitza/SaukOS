#!/bin/bash
set -u

source "$(dirname "$0")/modules/constants.sh"
source "$(dirname "$0")/modules/ui.sh"
source "$(dirname "$0")/modules/playfield.sh"
source "$(dirname "$0")/modules/game_logic.sh"
source "$(dirname "$0")/modules/input.sh"
source "$(dirname "$0")/modules/ticker.sh"
source "$(dirname "$0")/modules/controller.sh"

stty_g=$(stty -g)

(
    ticker &
    reader
)|(
    controller
)

show_cursor
stty $stty_g


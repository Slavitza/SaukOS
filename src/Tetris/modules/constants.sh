# Señales y comandos
QUIT=0
RIGHT=1
LEFT=2
ROTATE=3
DOWN=4
DROP=5
TOGGLE_HELP=6
TOGGLE_NEXT=7
TOGGLE_COLOR=8

DELAY=1
DELAY_FACTOR=0.8

RED=1
GREEN=2
YELLOW=3
BLUE=4
FUCHSIA=5
CYAN=6
WHITE=7

PLAYFIELD_W=10
PLAYFIELD_H=20
PLAYFIELD_X=30
PLAYFIELD_Y=1
BORDER_COLOR=$YELLOW

SCORE_X=1
SCORE_Y=2
SCORE_COLOR=$GREEN

HELP_X=58
HELP_Y=1
HELP_COLOR=$CYAN

NEXT_X=14
NEXT_Y=11

GAMEOVER_X=1
GAMEOVER_Y=$((PLAYFIELD_H + 3))

LEVEL_UP=20

colors=($RED $GREEN $YELLOW $BLUE $FUCHSIA $CYAN $WHITE)

no_color=true
showtime=true
empty_cell=" ."
filled_cell="[]"

score=0
level=1
lines_completed=0

help=(
"  Use cursor keys"
"       or"
"      s: up"
"a: left,  d: right"
"    space: drop"
"      q: quit"
"  c: toggle color"
"n: toggle show next"
"h: toggle this help"
)

help_on=-1

piece=(
"00011011"
"0212223210111213"
"0001111201101120"
"0102101100101121"
"01021121101112220111202100101112"
"01112122101112200001112102101112"
"01111221101112210110112101101112"
)

next_piece=0
next_piece_rotation=0
next_piece_color=0

next_on=1


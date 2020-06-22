COLOR_BOLD_YELLOW="\033[1;33m"
COLOR_BOLD_BLUE="\033[1;34m"
COLOR_BOLD_MAGENTA="\033[1;35m"
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

function section {
    echo
    echo -e $COLOR_BOLD_CYAN$1 $COLOR_RESET
    echo "----------------------------------------------------------"
}

function warn {
    echo -e $COLOR_BOLD_YELLOW"WARNING: "$COLOR_RESET$1
}

function clean_if_not_empty {
    if [ "$(ls -A $1)" ]; then
         echo "cleaning $1"
         rm -rf $1/*
    else
        echo "."
    fi
}


function safelink {
    if [ ! -e  $2 ]; then
        sudo ln -s $1 $2
    else
        warn "$2 already exists"
    fi
}

function overlink {
    if [ ! -e  $2 ]; then
        sudo ln -s $1 $2
    else
        warn "$2 already exists, overwriting it"
        sudo rm $2
        sudo ln -s $1 $2
    fi
}

function ptestkill {
    pgrep -f $1 > /dev/null
    if [ $? -eq 0 ]; then
      warn "$1 process is running. killing it"
      pkill -9 -f $1
    else
      echo "."
    fi
}

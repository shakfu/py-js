COLOR_BOLD_YELLOW="\033[1;33m"
COLOR_BOLD_BLUE="\033[1;34m"
COLOR_BOLD_MAGENTA="\033[1;35m"
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_BOLD_RED="\033[1;31m"
COLOR_BOLD_WHITE="\e[1m"
COLOR_RESET="\033[m"


function section {
    echo
    echo -e $COLOR_BOLD_MAGENTA$1 $COLOR_RESET
    echo "--------------------------------------------------------------------"
}

function info {
    echo
    echo -e "$COLOR_BOLD_CYAN$1 $COLOR_RESET"
    echo
}

function runtime {
    echo
    echo "----------------------------------------------------------"
    echo -e "=> $COLOR_BOLD_YELLOW BUILDTIME:$COLOR_RESET $1 seconds"
    echo "----------------------------------------------------------"
    echo
}

## ---------------------------------------------------------------------

START=`date +%s`


section "CONFIGURATION"

info "creating build folder"
mkdir -p ./build

info "cd to build folder"
cd ./build

info "configuring cmake"
cmake -G Xcode ..


section "COMPILATION"

info "build externals"
cmake --build .

info "return to project root"
cd ..

END=`date +%s`

runtime "$((END-START))"


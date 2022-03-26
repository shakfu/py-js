function runlog() {
    mkdir -p logs
    time make $1 &> logs/$1.log
    rpl -q "[1;36m" "" logs/$1.log &> /dev/null
    rpl -q "[m" "" logs/$1.log &> /dev/null
    rpl -q "$HOME" "~" logs/$1.log  &> /dev/null
}

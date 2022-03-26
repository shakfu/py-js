ESCAPED_HOME=$(echo $HOME | sed 's_/_\\/_g')
CHECK="python3 source/py/scripts/check_success.py"

TARGETS="           \
    default         \
    homebrew-pkg    \
    homebrew-ext    \
    shared-pkg      \
    shared-ext      \
    static-ext      \
    framework-pkg   \
    framework-ext   \
    relocatable-pkg \
"

TARGETS_NO_BREW="   \
    default         \
    shared-pkg      \
    shared-ext      \
    static-ext      \
    framework-pkg   \
    framework-ext   \
    relocatable-pkg \
"


function runlog() {
    mkdir -p logs
    echo
    echo "running 'make $1'"
    time make $1 &> logs/$1.log
    sed -i '' "s/\[1;36m//g" logs/$1.log
    sed -i '' "s/\[m//g" logs/$1.log
    sed -i '' "s/$ESCAPED_HOME/~/g" logs/$1.log
    echo
    $CHECK logs/$1.log
}

function runlog_all() {
    for t in $TARGETS
    do
        runlog logs/$t.log
    done
}

function check_all() {
    for t in $TARGETS
    do
        $CHECK logs/$t.log
    done
}

function runlog_all_no_brew() {
    for t in $TARGETS_NO_BREW
    do
        runlog logs/$t.log
    done
}

function check_all_no_brew() {
    for t in $TARGETS_NO_BREW
    do
        $CHECK logs/$t.log
    done
}


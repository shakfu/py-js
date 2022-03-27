
BASEDIR="$HOME/.build_pyjs"

BASELOGSDIR="$BASEDIR/logs"

PYTHON_VERSION=`python3 -c 'import platform; print(platform.python_version())'`

LOGDIR="$BASELOGSDIR/$PYTHON_VERSION"

CHECK="python3 source/py/scripts/check_success.py"

ESCAPED_HOME=$(echo $HOME | sed 's_/_\\/_g')

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
    mkdir -p $LOGDIR
    LOG=$LOGDIR/$1.log
    echo
    echo "running 'make $1'"
    time make $1 &> $LOG
    sed -i '' "s/\[1;36m//g" $LOG
    sed -i '' "s/\[m//g" $LOG
    sed -i '' "s/$ESCAPED_HOME/~/g" $LOG
    echo
    $CHECK $LOG
}

function runlog_all() {
    for t in $TARGETS
    do
        runlog $t
    done
}

function check_all() {
    for t in $TARGETS
    do
        $CHECK $LOGDIR/$t.log
    done
}

function runlog_all_no_brew() {
    for t in $TARGETS_NO_BREW
    do
        runlog $t
    done
}

function check_all_no_brew() {
    for t in $TARGETS_NO_BREW
    do
        $CHECK $LOGDIR/$t.log
    done
}


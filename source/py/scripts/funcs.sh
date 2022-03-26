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
    mkdir -p logs
    time make $1 &> logs/$1.log
    sed -i '' "s/\[1;36m//g" logs/$1.log
    sed -i '' "s/\[m//g" logs/$1.log
    sed -i '' "s/$ESCAPED_HOME/~/g" logs/$1.log
}

function runlog_all() {
    for t in $TARGETS
    do
        runlog $t
    done
}

function runlog_all_no_brew() {
    for t in $TARGETS_NO_BREW
    do
        runlog $t
    done
}


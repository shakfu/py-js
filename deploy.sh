NAME="py"
PACKAGE=$HOME"/Documents/Max 8/Packages/"$NAME

mkdir -p "$PACKAGE"
for target in docs examples externals help init \
              javascript jsextensions media patchers
do
    echo "syncing $target..."
    touch "$PACKAGE"/tst
    rsync -a --delete ./$target "$PACKAGE"
done


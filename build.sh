# xattr -cr .
#make -C source/py default

#cd source/py
#python3 -m builder brew_sys
#cd ../../

cd source/py/targets/python-sys
python3 build.py
cd ../../../../


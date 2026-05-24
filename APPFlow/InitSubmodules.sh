
git submodule update --init --recursive
 
git submodule foreach --recursive "git pull origin master"  

git submodule foreach --recursive "git checkout master"  

pause

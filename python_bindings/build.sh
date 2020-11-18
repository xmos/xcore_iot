set -e

clobber='false'

while getopts ":c" opt; do
  case $opt in
    c)
      clobber='true'
      ;;
  esac
done

echo "****************************"
echo "* Building python_bindings"
echo "*"
echo "* clobber=$clobber"
echo "****************************"

if [ $clobber = 'true' ]
then
    rm -rf build
    mkdir build
fi

cd build
cmake ../
make install
cd ..

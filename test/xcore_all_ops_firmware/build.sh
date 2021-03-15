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
echo "* Building xcore_firmware"
echo "*"
echo "* clobber=$clobber"
echo "****************************"

if [ $clobber = 'true' ]
then
    rm -rf build
fi

mkdir -p build
cd build
cmake ../
make install
cd ..
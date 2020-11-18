for file in `ls images/*`
do
  ../test_image.py $file
done

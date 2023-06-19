for seed in 1 2 3 4 5
 do
 for n in 9 11 13
  do
  for e in 0.1 0.3
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   python3 random_graph_generator.py -n $n -e $e -s $seed > data/${FILE_NAME}.in
  done
 done
done


for seed in 1 2 3 4 5
 do
 for n in 9 11 13
  do
  for e in 0.1 0.3
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   ./klta -k 2 -h blind -j random -c < data/${FILE_NAME}.in > output/blind-random-${FILE_NAME}.out &
  done
 done
done
echo Random-Random Calculating ... & wait
echo Random-Random Completed!!

for seed in 1 2 3 4 5
 do
 for n in 9 11 13
  do
  for e in 0.1 0.3
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   ./klta -k 2 -h singleton -j random -c < data/${FILE_NAME}.in > output/singleton-random-${FILE_NAME}.out &
  done
 done
done
echo Singleton-Random Calculating ... & wait
echo Singleton-Random Completed!!

for seed in 1 2 3 4 5
 do
 for n in 9 11 13
  do
  for e in 0.1 0.3
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   ./klta -k 2 -h singleton -j nearest -c -u < data/${FILE_NAME}.in > output/singleton-nearest-${FILE_NAME}.out &
  done
 done
done
echo Singleton-Nearest Calculating ... & wait
echo Singleton-Nearest Completed!!

python3 report.py -d "output/*" -o report.csv

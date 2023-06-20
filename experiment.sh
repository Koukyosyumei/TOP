for seed in {1..10}
 do
 for n in 11 12 13
  do
  for e in 0.2 0.3 0.4
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   python3 random_graph_generator.py -n $n -e $e -s $seed > data/${FILE_NAME}.in
  done
 done
done


for seed in {1..10}
 do
 for n in 11 12 13
  do
  for e in 0.2 0.3 0.4
   do
   for k in 2
    do
     FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
     ./klta -k $k -h blind -j random -c < data/${FILE_NAME}.in > output/blind-random-${FILE_NAME}k$k.out &
   done
  done
 done
 echo Random-Random Calculating seed=$seed ... & wait
done
echo Random-Random Completed!!

for j in random nearest nearestdist
do
for seed in {1..10}
 do
 for n in 11 12 13
  do
  for e in 0.2 0.3 0.4
   do
   for k in 2
    do
     FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
     if [ "${j}" == "random" ]; then
     ./klta -k $k -h singleton -j ${j} -c < data/${FILE_NAME}.in > output/singleton-${j}$-${FILE_NAME}k$k.out &
     else
     ./klta -k $k -h singleton -j ${j} -c -u < data/${FILE_NAME}.in > output/singleton-${j}$-${FILE_NAME}k$k.out &
     fi
   done
  done
 done
 echo Singleton-$j Calculating seed=$seed ... & wait
done
echo Singleton-$j Completed!!
done

python3 report.py -d "output/*" -o report.csv

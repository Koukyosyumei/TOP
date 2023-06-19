python3 reuse.py -f assets/map.in -d data -s 42 -n 5

for k in 2 3 5 7
 do
 for el in 0 100 300 500 700
  do
  for h in singleton
   do
   for j in random nearest
    do
    for seed in 0 1 2 3 4
     do
     FILE_NAME="${k}-${el}-${h}-${j}_n=${368}_e=${851}_s=${seed}_"
     ./klta -k $k -l ${el} -h ${h$ -j ${j} -u < data/map.in-${seed}.in > output/${FILE_NAME}k$k.out &
    done
    echo $k ${el} $h $j & wait
   done
  done
 done
done



python3 reuse.py -f assets/map.in -d data -s 42 -n 10

for k in 2 3 5
 do
 for el in 0 50 100 300
  do
  for h in singleton
   do
   for j in random nearest nearestdist
    do
    for seed in ${0..9}
     do
     FILE_NAME="${k}-${el}-${h}-${j}_n=${368}_e=${851}_s=${seed}_"
     ./klta -k $k -l ${el} -h ${h} -j ${j} < data/map.in-${seed}.in > output/${FILE_NAME}k$k.out &
    done
    echo $k ${el} $h $j & wait
   done
  done
 done
done

python3 report.py -d "output/*" -o georeport.csv


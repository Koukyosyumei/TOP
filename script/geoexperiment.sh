python3 script/reuse.py -f assets/map.in -d data -s 42 -n 10

for k in 2 3 5
 do
 for el in 0 100 300
  do
  for h in blind tunnel tunnel+
   do
   for j in random nearest
    do
    for seed in {0..9}
     do
     FILE_NAME="${k}-${el}-${h}-${j}_n=${368}_e=${851}_s=${seed}_"
     ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 10000 -c -u < data/map.in-${seed}.in > output/${FILE_NAME}k${k}.out
    done
    echo ${k} ${el} ${h} ${j} & wait
   done
  done
 done
done

python3 script/report.py -d "output/*" -o georeport_summary.csv -t georeport_time.csv


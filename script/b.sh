python3 script/reuse.py -f assets/map.in -d data -s 42 -n 10

for k in 5
 do
 for el in 100
  do
  for h in tunnel mst
   do
   for j in random
    do
    for rr in 500 1000 1500
    do
     for seed in 1
     do
     FILE_NAME="${k}-${el}-${h}-${j}-${v}-${rr}_n=368_e=851_s=${seed}_"
     ./topsolver -v radius -r ${rr} -k ${k} -l ${el} -h ${h} -j ${j} -f output/${FILE_NAME}k${k}.out -b 100 -t 10000 -c -u < data/map.in-${seed}.in
     done
    done
    echo ${k} ${el} ${h} ${j} & wait
   done
  done
 done
done

python3 script/report.py -d "output/*" -o georeport_summary.csv -t georeport_time.csv


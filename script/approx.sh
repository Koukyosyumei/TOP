python3 script/reuse.py -f assets/map.in -d data -s 42 -n 10

for seed in 1 2 3 4 5
 do
   FILE_NAME="t=internet_n=400_e=1.0_s=${seed}_"
   python3 script/random_graph_generator.py -t internet -n 400 -s $seed > data/${FILE_NAME}.in

   FILE_NAME="t=gnp_n=400_e=0.3_s=${seed}_"
   python3 script/random_graph_generator.py -t gnp -n 400 -e 0.3 -s $seed > data/${FILE_NAME}.in
done

for k in 2 3 5
 do
 for el in 50 100 300
  do
  for h in blind tunnel
   do
   for j in random nearest
    do
    for seed in 1 2 3 4 5
     do
     FILE_NAME="${k}-${el}-${h}-${j}_t=map_n=368_e=851_s=${seed}_"
     ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 20000 -f output/${FILE_NAME}.out -c -u < data/map.in-${seed}.in
    done
    echo ${k} ${el} ${h} ${j} & wait
   done
  done
 done
done

for k in 2 3 5
 do
 for el in 5 10 30
  do
  for h in blind tunnel
   do
   for j in random nearest
    do
    for seed in 1 2 3 4 5
     do
     FILE_NAMEI="t=internet_n=400_e=1.0_s=${seed}_"
     ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 20000 -f output/${k}-${el}-${h}-${j}${FILE_NAMEI}.out -c -u < data/${FILE_NAMEI}.in
     FILE_NAMER="t=gnp_n=400_e=0.3_s=${seed}_"
     ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 20000 -f output/${k}-${el}-${h}-${j}${FILE_NAMER}.out -c -u < data/${FILE_NAMER}.in
    done
    echo ${k} ${el} ${h} ${j} & wait
   done
  done
 done
done

python3 script/report.py -d "output/*" -o approx_summary.csv -t approx_time.csv


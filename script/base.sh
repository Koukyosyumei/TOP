python3 script/reuse.py -f assets/map.in -d data -s 42 -n 10

for seed in 1 2 3 4 5
 do
 for f in "lak102d.map" "lak108d.map" "lak109d.map" "den101d.map" "den201d.map" 
   do
   python3 script/random_graph_generator.py -t fgrid -g assets/${f} -s $seed > "data/${f}_n=1_e=1_s=${seed}.in"
 done
done

for k in 1
 do
 for el in 1
  do
  for h in tunnel
   do
   for j in random
    do
    for f in "lak102d.map" "lak108d.map" "lak109d.map" "den101d.map" "den201d.map"
    do
     for seed in 1 2 3 4 5
      do
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -f output/${h}${j}${k}${el}${f}_${seed}.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -v radius -r 1 -f output/r1${h}${j}${k}${el}${f}_${seed}.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -v radius -r 2 -f output/r2${h}${j}${k}${el}${f}_${seed}.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
     echo ${f} ${k} ${el} ${h} ${j} & wait
    done
   done
  done
 done
done

python3 script/report.py -d "output/*" -o base_summary.csv -t base_time.csv


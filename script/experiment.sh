for seed in {1..5}
 do
 for n in 15
  do
  for e in 0.2 0.3
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   python3 script/random_graph_generator.py -n $n -e $e -s $seed > data/${FILE_NAME}.in
  done
 done
done


for seed in {1..5}
do
 for h in blind tunnel tunnel+
 do
  for j in random nearest
  do
   for n in 15
   do
    for e in 0.2 0.3
    do
     for k in 2 3
     do
      for l in 1 2 3
      do
      FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
      ./topsolver -k $k -l $l -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAME}n${n}e${e}k${k}l${l}.out -c -u < data/${FILE_NAME}.in &
      done
     done
    echo "$h - $j k=$k el=$l seed=${seed}" & wait
    done
   done
  done
 done
done

python3 script/report.py -d "output/*" -o report.csv

for seed in {1..5}
 do
 for n in 9 11
  do
  for e in 0.3 0.4
   do
   FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
   python3 script/random_graph_generator.py -n $n -e $e -s $seed > data/${FILE_NAME}.in
  done
 done
done

for seed in {1..5}
do
 for h in blind tunnel
  do
   for n in 9 11
   do
    for e in 0.3 0.4
    do
     for k in 2 3
     do
      for l in 1
      do
      FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
      ./topsolver -k $k -l $l -p greedy -h $h -f output/${h}-${FILE_NAME}n${n}e${e}k${k}l${l}greedy.out -c -u < data/${FILE_NAME}.in &
      done
     done
     echo "h=$h j=$j n=$n e=$e seed=${seed}" & wait
    done
  done
 done
done


for seed in {1..5}
do
 for h in blind tunnel
 do
  for j in random nearest
  do
   for n in 9 11
   do
    for e in 0.3 0.4
    do
     for k in 2 3
     do
      for l in 1
      do
      FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
      ./topsolver -k $k -l $l -p merge -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAME}n${n}e${e}k${k}l${l}merge.out -c -u < data/${FILE_NAME}.in &
      done
     done
     echo "h=$h j=$j n=$n e=$e seed=${seed}" & wait
    done
   done
  done
 done
done

echo "Finishing..." & wait
python3 script/report.py -d "output/*" -o report.csv

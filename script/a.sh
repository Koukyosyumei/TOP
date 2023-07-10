for seed in {1..5}
 do
 for n in 13
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
 for h in tunnel mst
 do
  for j in random
  do
   for n in 13
   do
    for e in 0.2 0.3
    do
     for k in 2 3
     do
      for l in 2
      do
      FILE_NAME="random_n=${n}_e=${e}_s=${seed}_"
      ./topsolver -k $k -l $l -v onestep -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAME}n${n}e${e}k${k}l${l}.out -c -u < data/${FILE_NAME}.in &
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

for seed in 1 2 3 4 5
 do
   FILE_NAME="t=grid_n=5_e=0.3_s=${seed}_"
   python3 script/random_graph_generator.py -t grid -n 5 -o 0.3 -s $seed > data/${FILE_NAME}.in

   # FILE_NAME="t=internet_n=13_e=1.0_s=${seed}_"
   # python3 script/random_graph_generator.py -t internet -n 13 -s $seed > data/${FILE_NAME}.in

   # FILE_NAME="t=gnp_n=13_e=0.3_s=${seed}_"
   # python3 script/random_graph_generator.py -t gnp -n 13 -e 0.3 -s $seed > data/${FILE_NAME}.in
done

for seed in 1 2 3 4 5
do
FILE_NAMEG="t=grid_n=5_e=0.3_s=${seed}_"
# FILE_NAMEI="t=internet_n=13_e=1.0_s=${seed}_"
# FILE_NAMER="t=gnp_n=13_e=0.3_s=${seed}_"
 for h in blind tunnel
 do
  for j in random nearest
  do
   for k in 2 3
   do
    for l in 1 2 3
    do
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -v radius -r 1 -f output/radius1-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -v radius -r 2 -f output/radius2-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    done
   done
   echo "h=$h j=$j seed=${seed}" & wait
  done
 done
done

for seed in 1 2 3 4 5
do
FILE_NAMEG="t=grid_n=5_e=0.3_s=${seed}_"
# FILE_NAMEI="t=internet_n=13_e=1.0_s=${seed}_"
# FILE_NAMER="t=gnp_n=13_e=0.3_s=${seed}_"
 for h in blind tunnel
 do
   for k in 2 3
   do
    for l in 1 2 3
    do
    ./topsolver -k $k -l $l -p greedy -h $h -f output/${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p greedy -h $h -v radius -r 1 -f output/radius1-${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p greedy -h $h -v radius -r 2 -f output/radius2-${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    done
   echo "h=$h seed=${seed}" & wait
  done
 done
done

echo "Finishing..." & wait
python3 script/report.py -d "output/*" -o report.csv

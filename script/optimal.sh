TIMEOUT=1000000

for seed in 1 2 3 4 5
 do
   FILE_NAME="t=grid_n=6_e=0.5_s=${seed}_"
   python3 script/random_graph_generator.py -t grid -n 6 -o 0.5 -s $seed > data/${FILE_NAME}.in
done

for seed in 1 2 3 4 5
do
FILE_NAMEG="t=grid_n=6_e=0.5_s=${seed}_"
 for h in blind tunnel
 do
  for j in random nearest
  do
   for k in 2 3
   do
    for l in 1 2 3
    do
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -f output/${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -v radius -r 1 -f output/radius1-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -v radius -r 2 -f output/radius2-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    done
   done
  done
 done
done
echo "Merge Done" & wait

for seed in 1 2 3 4 5
do
FILE_NAMEG="t=grid_n=6_e=0.5_s=${seed}_"
 for h in blind tunnel
 do
   for k in 2 3
   do
    for l in 1 2 3
    do
    ./topsolver -k $k -l $l -p greedy -h $h -t ${TIMEOUT} -f output/${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p greedy -h $h -t ${TIMEOUT} -v radius -r 1 -f output/radius1-${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p greedy -h $h -t ${TIMEOUT} -v radius -r 2 -f output/radius2-${h}-${FILE_NAMEG}k${k}l${l}greedy.out -c -u < data/${FILE_NAMEG}.in &
    done
  done
 done
done
echo "Greedy Done" & wait

echo "Finishing..." & wait
python3 script/report.py -d "output/*" -o report.csv

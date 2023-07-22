for seed in 1 2 3 4 5
 do
   FILE_NAME="t=grid_n=4_e=0.3_s=${seed}_"
   python3 script/random_graph_generator.py -t grid -n 4 -o 0.3 -s $seed > data/${FILE_NAME}.in

   FILE_NAME="t=internet_n=13_e=1.0_s=${seed}_"
   python3 script/random_graph_generator.py -t internet -n 13 -s $seed > data/${FILE_NAME}.in

   FILE_NAME="t=gnp_n=13_e=0.3_s=${seed}_"
   python3 script/random_graph_generator.py -t gnp -n 13 -e 0.3 -s $seed > data/${FILE_NAME}.in
done

for seed in 1
do
FILE_NAMEG="t=grid_n=4_e=0.3_s=${seed}_"
FILE_NAMEI="t=internet_n=13_e=1.0_s=${seed}_"
FILE_NAMER="t=gnp_n=13_e=0.3_s=${seed}_"
 for h in blind tunnel
 do
  for j in random nearest
  do
   for k in 2 3
   do
    for l in 1 2 3
    do
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAMEI}k${k}l${l}merge.out -c -u < data/${FILE_NAMEI}.in &
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -f output/${h}-${j}$-${FILE_NAMER}k${k}l${l}merge.out -c -u < data/${FILE_NAMER}.in &
    done
   done
   echo "h=$h j=$j seed=${seed}" & wait
  done
 done
done

echo "Finishing..." & wait
python3 script/report.py -d "output/*" -o report.csv

TIMEOUT=100000

for seed in 0 1 2 3 4 5 6 7 8 9
 do
   FILE_NAME="t=grid_n=5_e=0.4_s=${seed}_"
   python3 script/random_graph_generator.py -t grid -n 5 -o 0.4 -s $seed > data/${FILE_NAME}.in
done

for seed in 0 1 2 3 4 5 6 7 8 9
do
FILE_NAMEG="t=grid_n=5_e=0.4_s=${seed}_"
 for h in tunnel
 do
  for j in "random" "adacost" "asccost" "ascnear" "deccost" "decnear" "adacost+" "adacost-" "adacostnear+" "adacostnear-" "adanearcost+" "adanearcost-"
  do
   for k in 2 3
   do
    for l in 1 2
    do
    ./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -f output/${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    #./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -v radius -r 1 -f output/radius1-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    #./topsolver -k $k -l $l -p merge -h $h -j ${j} -t ${TIMEOUT} -v radius -r 2 -f output/radius2-${h}-${j}$-${FILE_NAMEG}k${k}l${l}merge.out -c -u < data/${FILE_NAMEG}.in &
    done
   done
  done
 done
done
echo "Merge Done" & wait

echo "Finishing..." & wait
python3 script/report.py -d "output/*" -o report.csv
